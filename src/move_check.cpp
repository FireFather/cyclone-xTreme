// move_check.cpp

// includes

#include "attack.h"
#include "colour.h"
#include "fen.h"
#include "list.h"
#include "move.h"
#include "move_check.h"
#include "move_do.h"
#include "move_gen.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// prototypes

static void add_quiet_checks( list_t *list, const board_t *board );
static void add_castle_checks( list_t *list, board_t *board );
static void add_check( list_t *list, int move, board_t *board );
static void find_pins( int list [], const board_t *board );

// functions

// gen_quiet_checks()

void gen_quiet_checks( list_t *list, board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    ASSERT(!board_is_check(board));

    LIST_CLEAR(list);

    add_quiet_checks(list, board);
    add_castle_checks(list, board);

    // debug

    ASSERT(list_is_ok(list));
    }

// add_quiet_checks()

static void add_quiet_checks( list_t *list, const board_t *board )
    {

    int me, opp;
    int king;
    const sq_t *ptr, *ptr_2;
    int from, to, sq;
    int piece;
    const inc_t *inc_ptr;
    int inc;
    int pawn;
    int rank;
    int pin[8 + 1];

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    // init

    me = board->turn;
    opp = COLOUR_OPP(me);

    king = KING_POS(board, opp);

    find_pins(pin, board);

    // indirect checks

    for ( ptr = pin; (from = *ptr) != SquareNone; ptr++ )
        {

        piece = board->square[from];

        ASSERT(is_pinned(board, from, opp));

        if( PIECE_IS_PAWN(piece) )
            {

            inc = PAWN_MOVE_INC(me);
            rank = PAWN_RANK(from, me);

            if( rank != Rank7 )
                { // promotes are generated with captures
                to = from + inc;

                if( board->square[to] == Empty )
                    {
                    if( DELTA_INC_LINE(to - king) != DELTA_INC_LINE(from - king) )
                        {
                        ASSERT(!SQUARE_IS_PROMOTE(to));
                        LIST_ADD(list, MOVE_MAKE(from, to));

                        if( rank == Rank2 )
                            {
                            to = from + (2 * inc);

                            if( board->square[to] == Empty )
                                {
                                ASSERT(DELTA_INC_LINE(to - king) != DELTA_INC_LINE(from - king));
                                ASSERT(!SQUARE_IS_PROMOTE(to));
                                LIST_ADD(list, MOVE_MAKE(from, to));
                                }
                            }
                        }
                    }
                }
            }
        else if( PIECE_IS_SLIDER(piece) )
            {
            for ( inc_ptr = PIECE_INC(piece); (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                for ( to = from + inc; board->square[to] == Empty; to += inc )
                    {
                    ASSERT(DELTA_INC_LINE(to - king) != DELTA_INC_LINE(from - king));
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                }
            }
        else
            {
            for ( inc_ptr = PIECE_INC(piece); (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                to = from + inc;

                if( board->square[to] == Empty )
                    {
                    if( DELTA_INC_LINE(to - king) != DELTA_INC_LINE(from - king) )
                        {
                        LIST_ADD(list, MOVE_MAKE(from, to));
                        }
                    }
                }
            }
        }

    // piece direct checks

    for ( ptr = &board->piece[me][1]; (from = *ptr) != SquareNone; ptr++ )
        {
        for ( ptr_2 = pin; (sq = *ptr_2) != SquareNone; ptr_2++ )
            {
            if( sq == from )
                goto next_piece;
            }

        ASSERT(!is_pinned(board, from, opp));

        piece = board->square[from];
        inc_ptr = PIECE_INC(piece);

        if( PIECE_IS_SLIDER(piece) )
            {
            for (; (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                for ( to = from + inc; board->square[to] == Empty; to += inc )
                    {
                    if( PIECE_ATTACK(board, piece, to, king) )
                        {
                        LIST_ADD(list, MOVE_MAKE(from, to));
                        }
                    }
                }
            }
        else
            {
            for (; (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                to = from + inc;

                if( board->square[to] == Empty )
                    {
                    if( PSEUDO_ATTACK(piece, king - to) )
                        {
                        LIST_ADD(list, MOVE_MAKE(from, to));
                        }
                    }
                }
            }

        next_piece:
        ;
        }

    // pawn direct checks

    inc = PAWN_MOVE_INC(me);
    pawn = PAWN_MAKE(me);

    to = king - (inc - 1);
    ASSERT(PSEUDO_ATTACK(pawn, king - to));

    from = to - inc;

    if( board->square[from] == pawn )
        {
        if( board->square[to] == Empty )
            {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list, MOVE_MAKE(from, to));
            }
        }
    else
        {
        from = to - (2 * inc);

        if( board->square[from] == pawn )
            {
            if( PAWN_RANK(from, me) == Rank2 && board->square[to] == Empty && board->square[from + inc] == Empty )
                {
                ASSERT(!SQUARE_IS_PROMOTE(to));
                LIST_ADD(list, MOVE_MAKE(from, to));
                }
            }
        }

    to = king - (inc + 1);
    ASSERT(PSEUDO_ATTACK(pawn, king - to));

    from = to - inc;

    if( board->square[from] == pawn )
        {
        if( board->square[to] == Empty )
            {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list, MOVE_MAKE(from, to));
            }
        }
    else
        {
        from = to - (2 * inc);

        if( board->square[from] == pawn )
            {
            if( PAWN_RANK(from, me) == Rank2 && board->square[to] == Empty && board->square[from + inc] == Empty )
                {
                ASSERT(!SQUARE_IS_PROMOTE(to));
                LIST_ADD(list, MOVE_MAKE(from, to));
                }
            }
        }
    }

// add_castle_checks()

void add_castle_checks( list_t *list, board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    ASSERT(!board_is_check(board));

    sq_t *sq;
    int flag;
    int me = board->turn;
    int move;
    int to;

    // legal

    for ( int wing = 0; wing < 2; wing++ )
        {
        flag = CASTLE_FLAG(me, wing);

        if( board->flags & flag )
            {
            for ( sq = CASTLE_EMPTY(me, wing); *sq != SquareNone; sq++ )
                if( board->square[*sq] != Empty )
                    goto next;

            for ( sq = CASTLE_SAFE(me, wing); *sq != SquareNone; sq++ )
                if( is_attacked(board, *sq, COLOUR_OPP(me)) )
                    goto next;
            to = CASTLE_TO(me, wing);

            if( wing == CastleQueen && RookSquare[me][wing] == to - 1 )
                {
                int piece = board->square[to - 2];

                if( COLOUR_IS(piece, COLOUR_OPP(me)) && FLAG_IS(piece, RookFlag) )
                    continue;
                }

            move = MOVE_MAKE_FLAGS(KingSquare[me], to, MoveCastle);
            add_check(list, move, board);
            }
        next:
        ;
        }
    }

// add_check()

static void add_check( list_t *list, int move, board_t *board )
    {

    undo_t undo[1];

    ASSERT(list != NULL);
    ASSERT(move_is_ok(move));
    ASSERT(board != NULL);

    move_do(board, move, undo);

    if( IS_IN_CHECK(board, board->turn) )
        LIST_ADD(list, move);
    move_undo(board, move, undo);
    }

// move_is_check()

bool move_is_check( int move, board_t *board )
    {

    undo_t undo[1];
    bool check;
    int me, opp, king;
    int from, to, piece;

    ASSERT(move_is_ok(move));
    ASSERT(board != NULL);

    // slow test for complex moves

    if( MOVE_IS_SPECIAL(move) )
        {

        move_do(board, move, undo);
        check = IS_IN_CHECK(board, board->turn);
        move_undo(board, move, undo);

        return check;
        }

    // init

    me = board->turn;
    opp = COLOUR_OPP(me);
    king = KING_POS(board, opp);

    from = MOVE_FROM(move);
    to = MOVE_TO(move);
    piece = board->square[from];
    ASSERT(COLOUR_IS(piece, me));

    // direct check

    if( PIECE_ATTACK(board, piece, to, king) )
        return true;

    // indirect check

    if( is_pinned(board, from, opp) && DELTA_INC_LINE(king - to) != DELTA_INC_LINE(king - from) )
        {
        return true;
        }

    return false;
    }

// find_pins()

static void find_pins( int list [], const board_t *board )
    {

    int me, opp;
    int king;
    const sq_t *ptr;
    int from;
    int piece;
    int delta;
    int inc;
    int sq;
    int capture;
    int pin;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    // init

    me = board->turn;
    opp = COLOUR_OPP(me);

    king = KING_POS(board, opp);

    for ( ptr = &board->piece[me][1]; (from = *ptr) != SquareNone; ptr++ )
        {

        piece = board->square[from];

        delta = king - from;
        ASSERT(delta_is_ok(delta));

        if( PSEUDO_ATTACK(piece, delta) )
            {

            ASSERT(PIECE_IS_SLIDER(piece));

            inc = DELTA_INC_LINE(delta);
            ASSERT(inc != IncNone);

            ASSERT(SLIDER_ATTACK(piece, inc));

            sq = from;
            do sq += inc;

            while( (capture = board->square[sq]) == Empty );

            ASSERT(sq != king);

            if( COLOUR_IS(capture, me) )
                {
                pin = sq;
                do sq += inc;

                while( board->square[sq] == Empty );

                if( sq == king )
                    * list++ = pin;
                }
            }
        }

    *list = SquareNone;
    }

// end of move_check.cpp
