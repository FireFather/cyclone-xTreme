// pawn.cpp

// includes

#include <cstring>

#include "board.h"
#include "colour.h"
#include "hash.h"
#include "init.h"
#include "option.h"
#include "pawn.h"
#include "piece.h"
#include "protocol.h"
#include "search.h"
#include "square.h"
#include "util.h"

// constants

static const bool UseTable = true;
static const uint32 TableSize = 16384;

// types

typedef pawn_info_t entry_t;

struct pawn_t
    {
    entry_t *table;
    uint32 size;
    uint32 mask;
    uint32 used;
    sint64 read_nb;
    sint64 read_hit;
    sint64 write_nb;
    sint64 write_collision;
    };

// constants and variables

const int OutpostMatrix[2][64] =
    {
	0,	 0,  0,  0,  0,  0,  0,  0,
	0,	 0,  0,  0,  0,  0,  0,  0,
	1,	 2,  3,  3,  3,  3,  2,  1,
	2,	 4,  5,  7,  7,  5,  4,  2,
	3,	 5,  6, 10, 10,  6,  5,  3,
	5,	10, 15, 20, 20, 15, 10,  5,
	5,	10, 15, 25, 25, 15, 10,  5,
	5,	 5, 10, 15, 15, 10,  5,  5,

	5,	 5, 10, 15, 15, 10,  5,  5,
	5,	10, 15, 25, 25, 15, 10,  5,
	5,	10, 15, 20, 20, 15, 10,  5,
	3,	 5,  6, 10, 10,  6,  5,  3,
	2,	 4,  5,  7,  7,  5,  4,  2,
	1,	 2,  3,  3,  3,  3,  2,  1,
	0,	 0,  0,  0,  0,  0,  0,  0,
	0,	 0,  0,  0,  0,  0,  0,  0
    };

static int Bonus[RankNb];

// variables

int BitEQ[16];
int BitLT[16];
int BitLE[16];
int BitGT[16];
int BitGE[16];

int BitFirst[0x100];
int BitLast[0x100];
int BitCount[0x100];
int BitRev[0x100];

static pawn_t Pawn[MaxThreads][1];

static int BitRank1[RankNb];
static int BitRank2[RankNb];
static int BitRank3[RankNb];

// prototypes

static void pawn_comp_info( pawn_info_t *info, const board_t *board );

// functions

// pawn_init_bit()

void pawn_init_bit()
    {

    int rank;
    int first, last, count;
    int b, rev;

    // rank-indexed Bit*[]

    for ( rank = 0; rank < RankNb; rank++ )
        {

        BitEQ[rank] = 0;
        BitLT[rank] = 0;
        BitLE[rank] = 0;
        BitGT[rank] = 0;
        BitGE[rank] = 0;

        BitRank1[rank] = 0;
        BitRank2[rank] = 0;
        BitRank3[rank] = 0;
        }

    for ( rank = Rank1; rank <= Rank8; rank++ )
        {
        BitEQ[rank] = 1 << (rank - Rank1);
        BitLT[rank] = BitEQ[rank] - 1;
        BitLE[rank] = BitLT[rank] | BitEQ[rank];
        BitGT[rank] = BitLE[rank] ^ 0xFF;
        BitGE[rank] = BitGT[rank] | BitEQ[rank];
        }

    for ( rank = Rank1; rank <= Rank8; rank++ )
        {
        BitRank1[rank] = BitEQ[rank + 1];
        BitRank2[rank] = BitEQ[rank + 1] | BitEQ[rank + 2];
        BitRank3[rank] = BitEQ[rank + 1] | BitEQ[rank + 2] | BitEQ[rank + 3];
        }

    // bit-indexed Bit*[]

    for ( b = 0; b < 0x100; b++ )
        {

        first = Rank8;
        last = Rank1;
        count = 0;
        rev = 0;

        for ( rank = Rank1; rank <= Rank8; rank++ )
            {
            if( (b &BitEQ[rank]) != 0 )
                {
                if( rank < first )
                    first = rank;

                if( rank > last )
                    last = rank;
                count++;
                rev |= BitEQ[RANK_OPP(rank)];
                }
            }

        BitFirst[b] = first;
        BitLast[b] = last;
        BitCount[b] = count;
        BitRev[b] = rev;
        }
    }

// pawn_init()

void pawn_init()
    {

    int rank, ThreadId;

    // bonus

    for ( rank = 0; rank < RankNb; rank++ )
        Bonus[rank] = 0;

    Bonus[Rank4] = 26;
    Bonus[Rank5] = 77;
    Bonus[Rank6] = 154;
    Bonus[Rank7] = 256;

    // pawn hash-table

    for ( ThreadId = 0; ThreadId < NumberThreads; ThreadId++ )
        {
        Pawn[ThreadId]->size = 0;
        Pawn[ThreadId]->mask = 0;
        Pawn[ThreadId]->table = NULL;
        }
    }

// pawn_alloc()

void pawn_alloc()
    {

    int ThreadId;

    ASSERT(sizeof(entry_t) == 16);

    if( UseTable )
        {
        for ( ThreadId = 0; ThreadId < NumberThreads; ThreadId++ )
            {
            Pawn[ThreadId]->size = TableSize;
            Pawn[ThreadId]->mask = TableSize - 1;
            Pawn[ThreadId]->table = (entry_t *)my_malloc(Pawn[ThreadId]->size * sizeof(entry_t));

            pawn_clear(ThreadId);
            }
        }
    }

// pawn_clear()

void pawn_clear( int ThreadId )
    {
    if( Pawn[ThreadId]->table != NULL )
        {
        memset(Pawn[ThreadId]->table, 0, Pawn[ThreadId]->size * sizeof(entry_t));
        }

    Pawn[ThreadId]->used = 0;
    Pawn[ThreadId]->read_nb = 0;
    Pawn[ThreadId]->read_hit = 0;
    Pawn[ThreadId]->write_nb = 0;
    Pawn[ThreadId]->write_collision = 0;
    }

// pawn_get_info()

void pawn_get_info( pawn_info_t *info, const board_t *board, int ThreadId )
    {

    uint64 key;
    entry_t *entry;

    ASSERT(info != NULL);
    ASSERT(board != NULL);

    // probe

    if( UseTable )
        {

        Pawn[ThreadId]->read_nb++;

        key = board->pawn_key;
        entry = &Pawn[ThreadId]->table[KEY_INDEX(key) & Pawn[ThreadId]->mask];

        if( entry->lock == KEY_LOCK(key) )
            {

            // found

            Pawn[ThreadId]->read_hit++;

            *info = *entry;

            return;
            }
        }

    // calculation

    pawn_comp_info(info, board);

    // store

    if( UseTable )
        {

        Pawn[ThreadId]->write_nb++;

        if( entry->lock == 0 )
            {
            Pawn[ThreadId]->used++;
            }
        else
            {
            Pawn[ThreadId]->write_collision++;
            }

        *entry = *info;
        entry->lock = KEY_LOCK(key);
        }
    }

// pawn_comp_info()

static void pawn_comp_info( pawn_info_t *info, const board_t *board )
    {

    int colour;
    int file, rank;
    int me, opp;
    const sq_t *ptr;
    int sq;
    bool backward, candidate, doubled, isolated, open, passed;
    int t1, t2;
    int n;
    int bits;
    int opening[ColourNb], endgame[ColourNb];
    int flags[ColourNb];
    int file_bits[ColourNb];
    int passed_bits[ColourNb];
    int single_file[ColourNb];

    ASSERT(info != NULL);
    ASSERT(board != NULL);

    // init

    for ( colour = 0; colour < ColourNb; colour++ )
        {

        opening[colour] = 0;
        endgame[colour] = 0;

        flags[colour] = 0;
        file_bits[colour] = 0;
        passed_bits[colour] = 0;
        single_file[colour] = SquareNone;
        }

    // features and scoring

    for ( colour = 0; colour < ColourNb; colour++ )
        {

        me = colour;
        opp = COLOUR_OPP(me);

        for ( ptr = &board->pawn[me][0]; (sq = *ptr) != SquareNone; ptr++ )
            {

            // init

            file = SQUARE_FILE(sq);
            rank = PAWN_RANK(sq, me);

            ASSERT(file >= FileA && file <= FileH);
            ASSERT(rank >= Rank2 && rank <= Rank7);

            // flags

            file_bits[me] |= BIT(file);

            if( rank == Rank2 )
                flags[me] |= BackRankFlag;

            // features

            backward = false;
            candidate = false;
            doubled = false;
            isolated = false;
            open = false;
            passed = false;

            t1 = board->pawn_file[me][file - 1] | board->pawn_file[me][file + 1];
            t2 = board->pawn_file[me][file] | BitRev[board->pawn_file[opp][file]];

            // doubled

            if( (board->pawn_file[me][file]&BitLT[rank]) != 0 )
                {
                doubled = true;
                }

            // isolated and backward

            if( t1 == 0 )
                {

                isolated = true;
                }
            else if( (t1 &BitLE[rank]) == 0 )
                {

                backward = true;

                // really backward?

                if( (t1 &BitRank1[rank]) != 0 )
                    {
                    ASSERT(rank + 2 <= Rank8);

                    if( ((t2 &BitRank1[rank])
                        | ((BitRev[board->pawn_file[opp][file - 1]]
                            | BitRev[board->pawn_file[opp][file + 1]]) & BitRank2[rank]))
                        == 0 )
                        {
                        backward = false;
                        }
                    }
                else if( rank == Rank2 && ((t1 &BitEQ[rank + 2]) != 0) )
                    {

                    ASSERT(rank + 3 <= Rank8);

                    if( ((t2 &BitRank2[rank])
                        | ((BitRev[board->pawn_file[opp][file - 1]]
                            | BitRev[board->pawn_file[opp][file + 1]]) & BitRank3[rank]))
                        == 0 )
                        {
                        backward = false;
                        }
                    }
                }

            // open, candidate and passed

            if( (t2 &BitGT[rank]) == 0 )
                {

                open = true;

                if( ((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]]) & BitGT[rank])
                    == 0 )
                    {
                    passed = true;
                    passed_bits[me] |= BIT(file);
                    }
                else
                    {

                    // candidate?

                    n = 0;

                    n += BIT_COUNT(board->pawn_file[me][file - 1] & BitLE[rank]);
                    n += BIT_COUNT(board->pawn_file[me][file + 1] & BitLE[rank]);

                    n -= BIT_COUNT(BitRev[board->pawn_file[opp][file - 1]] & BitGT[rank]);
                    n -= BIT_COUNT(BitRev[board->pawn_file[opp][file + 1]] & BitGT[rank]);

                    if( n >= 0 )
                        {

                        // safe?

                        n = 0;

                        n += BIT_COUNT(board->pawn_file[me][file - 1] & BitEQ[rank - 1]);
                        n += BIT_COUNT(board->pawn_file[me][file + 1] & BitEQ[rank - 1]);

                        n -= BIT_COUNT(BitRev[board->pawn_file[opp][file - 1]] & BitEQ[rank + 1]);
                        n -= BIT_COUNT(BitRev[board->pawn_file[opp][file + 1]] & BitEQ[rank + 1]);

                        if( n >= 0 )
                            candidate = true;
                        }
                    }
                }

            // outposts

            if( me == White )
                {
                if( SQUARE_IS_OK(sq + 15) )
                    {
                    if( WEAK_SQUARE_BLACK(sq + 15) )
                        opening[White] += OutpostMatrix[White][SQUARE_TO_64(sq + 15)];
                    else
                        opening[White] += (OutpostMatrix[White][SQUARE_TO_64(sq + 15)] + 1) / 2;
                    }

                if( SQUARE_IS_OK(sq + 17) )
                    {
                    if( WEAK_SQUARE_BLACK(sq + 17) )
                        opening[White] += OutpostMatrix[White][SQUARE_TO_64(sq + 17)];
                    else
                        opening[White] += (OutpostMatrix[White][SQUARE_TO_64(sq + 17)] + 1) / 2;
                    }
                }
            else
                {
                if( SQUARE_IS_OK(sq - 15) )
                    {
                    if( WEAK_SQUARE_WHITE(sq - 15) )
                        opening[Black] += OutpostMatrix[Black][SQUARE_TO_64(sq - 15)];
                    else
                        opening[Black] += (OutpostMatrix[Black][SQUARE_TO_64(sq - 15)] + 1) / 2;
                    }

                if( SQUARE_IS_OK(sq - 17) )
                    {
                    if( WEAK_SQUARE_WHITE(sq - 17) )
                        opening[Black] += OutpostMatrix[Black][SQUARE_TO_64(sq - 17)];
                    else
                        opening[Black] += (OutpostMatrix[Black][SQUARE_TO_64(sq - 17)] + 1) / 2;
                    }
                }

            // score

            if( doubled )
                {
                opening[me] -= DoubledOpening;
                endgame[me] -= DoubledEndgame;
                }

            if( isolated )
                {
                if( open )
                    {
                    opening[me] -= IsolatedOpeningOpen;
                    endgame[me] -= IsolatedEndgame;
                    }
                else
                    {
                    opening[me] -= IsolatedOpening;
                    endgame[me] -= IsolatedEndgame;
                    }
                }

            if( backward )
                {
                if( open )
                    {
                    opening[me] -= BackwardOpeningOpen;
                    endgame[me] -= BackwardEndgame;
                    }
                else
                    {
                    opening[me] -= BackwardOpening;
                    endgame[me] -= BackwardEndgame;
                    }
                }

            if( candidate )
                {
                opening[me] += quad(CandidateOpeningMin, CandidateOpeningMax, rank);
                endgame[me] += quad(CandidateEndgameMin, CandidateEndgameMax, rank);
                }
            }
        }

    // store info

    info->opening = ((opening[White] - opening[Black]) * PawnStructureWeightOpening) / 256;
    info->endgame = ((endgame[White] - endgame[Black]) * PawnStructureWeightEndgame) / 256;

    for ( colour = 0; colour < ColourNb; colour++ )
        {

        me = colour;
        opp = COLOUR_OPP(me);

        // draw flags

        bits = file_bits[me];

        if( bits != 0 && (bits &(bits - 1)) == 0 )
            { // one set bit

            file = BIT_FIRST(bits);
            rank = BIT_FIRST(board->pawn_file[me][file]);
            ASSERT(rank >= Rank2);

            if( ((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]]) & BitGT[rank])
                == 0 )
                {
                rank = BIT_LAST(board->pawn_file[me][file]);
                single_file[me] = SQUARE_MAKE(file, rank);
                }
            }

        info->flags[colour] = flags[colour];
        info->passed_bits[colour] = passed_bits[colour];
        info->single_file[colour] = single_file[colour];
        }
    }

// quad()

int quad( int y_min, int y_max, int x )
    {

    int y;

    ASSERT(y_min >= 0 && y_min <= y_max && y_max <= +32767);
    ASSERT(x >= Rank2 && x <= Rank7);

    y = y_min + ((y_max - y_min) * Bonus[x] + 128) / 256;
    ASSERT(y >= y_min && y <= y_max);

    return y;
    }

// is_passed()

bool is_passed( const board_t *board, int to, int me )
    {

    int t2;
    int opp;
    int file, rank;

    opp = COLOUR_OPP(me);
    file = SQUARE_FILE(to);
    rank = PAWN_RANK(to, me);

    t2 = board->pawn_file[me][file] | BitRev[board->pawn_file[opp][file]];

    // passed pawns
    if( (t2 &BitGT[rank]) == 0 )
        {
        if( ((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]]) & BitGT[rank]) == 0 )
            {
            return true;
            }
        }

    return false;
    }

// pawn_fork()

bool pawn_fork( const board_t *board, int to )
    {
    int inc = PAWN_MOVE_INC(board->turn);
    int opp_flag = COLOUR_FLAG(COLOUR_OPP(board->turn));
    int piece_flag = Knight64 | Bishop64 | Rook64 | Queen64 | King64;
    int right = board->square[to + inc + 1], left = board->square[to + inc - 1];

    return FLAG_IS(left, piece_flag) && FLAG_IS(left, opp_flag) && FLAG_IS(right, piece_flag)
        && FLAG_IS(right, opp_flag);
    }

// end of pawn.cpp
