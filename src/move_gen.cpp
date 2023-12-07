// move_gen.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "list.h"
#include "move.h"
#include "move_do.h"
#include "move_evasion.h"
#include "move_gen.h"
#include "move_legal.h"
#include "pawn.h"
#include "piece.h"
#include "util.h"

// prototypes

static void add_moves( list_t *list, const board_t *board );
static void add_captures( list_t *list, const board_t *board );
static void add_quiet_moves( list_t *list, const board_t *board );
static void add_promotes( list_t *list, const board_t *board );
static void add_en_passant_captures( list_t *list, const board_t *board );
static void add_castle_moves( list_t *list, const board_t *board );

// functions

// perft()

sint64 perft( board_t *board, int depth )
    {
    list_t list[1];
    undo_t undo[1];
    int move;
    sint64 value = 0;

    gen_legal_moves(list, board);

    if( !depth )
        return list->size;

    for ( int pos = 0; pos < list->size; pos++ )
        {
        move = LIST_MOVE(list, pos);
        move_do(board, move, undo);
        value += perft(board, depth - 1);
        move_undo(board, move, undo);
        }
    return value;
    }

// gen_legal_moves()

void gen_legal_moves( list_t *list, board_t *board )
    {

    attack_t attack[1];

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    attack_set(attack, board);

    if( ATTACK_IN_CHECK(attack) )
        {
        gen_legal_evasions(list, board, attack);
        }
    else
        {
        gen_moves(list, board);
        list_filter(list, board, &pseudo_is_legal, true);
        }

    // debug

    ASSERT(list_is_ok(list));
    }

// gen_moves()

void gen_moves( list_t *list, const board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    ASSERT(!board_is_check(board));

    LIST_CLEAR(list);

    add_moves(list, board);

    add_en_passant_captures(list, board);
    add_castle_moves(list, board);

    // debug

    ASSERT(list_is_ok(list));
    }

// gen_captures()

void gen_captures( list_t *list, const board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    LIST_CLEAR(list);

    add_captures(list, board);
    add_en_passant_captures(list, board);

    // debug

    ASSERT(list_is_ok(list));
    }

// gen_quiet_moves()

void gen_quiet_moves( list_t *list, const board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    ASSERT(!board_is_check(board));

    LIST_CLEAR(list);

    add_quiet_moves(list, board);
    add_castle_moves(list, board);

    // debug

    ASSERT(list_is_ok(list));
    }

// add_moves()

static void add_moves( list_t *list, const board_t *board )
    {

    int me, opp;
    int opp_flag;
    const sq_t *ptr;
    int from, to;
    int piece, capture;
    const inc_t *inc_ptr;
    int inc;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    me = board->turn;
    opp = COLOUR_OPP(me);

    opp_flag = COLOUR_FLAG(opp);

    // piece moves

    for ( ptr = &board->piece[me][0]; (from = *ptr) != SquareNone; ptr++ )
        {

        piece = board->square[from];
        inc_ptr = PIECE_INC(piece);

        if( PIECE_IS_SLIDER(piece) )
            {
            for (; (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                for ( to = from + inc; (capture = board->square[to]) == Empty; to += inc )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }

                if( FLAG_IS(capture, opp_flag) )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                }
            }
        else
            {
            for (; (inc = *inc_ptr) != IncNone; inc_ptr++ )
                {
                to = from + inc;
                capture = board->square[to];

                if( capture == Empty || FLAG_IS(capture, opp_flag) )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                }
            }
        }

    // pawn moves

    inc = PAWN_MOVE_INC(me);

    for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
        {

        to = from + (inc - 1);

        if( FLAG_IS(board->square[to], opp_flag) )
            {
            add_pawn_move(list, from, to);
            }

        to = from + (inc + 1);

        if( FLAG_IS(board->square[to], opp_flag) )
            {
            add_pawn_move(list, from, to);
            }

        to = from + inc;

        if( board->square[to] == Empty )
            {
            add_pawn_move(list, from, to);

            if( PAWN_RANK(from, me) == Rank2 )
                {
                to = from + (2 * inc);

                if( board->square[to] == Empty )
                    {
                    ASSERT(!SQUARE_IS_PROMOTE(to));
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                }
            }
        }
    }

// add_captures()

static void add_captures( list_t *list, const board_t *board )
    {

    int me, opp;
    int opp_flag;
    const sq_t *ptr;
    int from, to;
    int piece, capture;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    me = board->turn;
    opp = COLOUR_OPP(me);

    opp_flag = COLOUR_FLAG(opp);

    // piece captures

    for ( ptr = &board->piece[me][0]; (from = *ptr) != SquareNone; ptr++ )
        {

        piece = board->square[from];

        switch( PIECE_TYPE(piece) )
            {
            case Knight64:
                to = from - 33;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 31;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 18;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 14;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 14;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 18;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 31;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 33;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            case Bishop64:
                for ( to = from - 17; (capture = board->square[to]) == Empty; to -= 17 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from - 15; (capture = board->square[to]) == Empty; to -= 15 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 15; (capture = board->square[to]) == Empty; to += 15 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 17; (capture = board->square[to]) == Empty; to += 17 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            case Rook64:
                for ( to = from - 16; (capture = board->square[to]) == Empty; to -= 16 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from - 1; (capture = board->square[to]) == Empty; to -= 1 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 1; (capture = board->square[to]) == Empty; to += 1 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 16; (capture = board->square[to]) == Empty; to += 16 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            case Queen64:
                for ( to = from - 17; (capture = board->square[to]) == Empty; to -= 17 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from - 16; (capture = board->square[to]) == Empty; to -= 16 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from - 15; (capture = board->square[to]) == Empty; to -= 15 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from - 1; (capture = board->square[to]) == Empty; to -= 1 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 1; (capture = board->square[to]) == Empty; to += 1 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 15; (capture = board->square[to]) == Empty; to += 15 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 16; (capture = board->square[to]) == Empty; to += 16 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));
                for ( to = from + 17; (capture = board->square[to]) == Empty; to += 17 )
                ;
                if( FLAG_IS(capture, opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            case King64:

                to = from - 17;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 16;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 15;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 1;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 1;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 15;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 16;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 17;
                if( FLAG_IS(board->square[to], opp_flag) )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            default:

                ASSERT(false);
                break;
            }
        }

    // pawn captures

    if( COLOUR_IS_WHITE(me) )
        {
        for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
            {

            to = from + 15;

            if( FLAG_IS(board->square[to], opp_flag) )
                add_pawn_move(list, from, to);

            to = from + 17;

            if( FLAG_IS(board->square[to], opp_flag) )
                add_pawn_move(list, from, to);

            // promote

            if( SQUARE_RANK(from) == Rank7 )
                {
                to = from + 16;

                if( board->square[to] == Empty )
                    {
                    add_promote(list, MOVE_MAKE(from, to));
                    }
                }
            }
        }
    else
        { // black
        for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
            {

            to = from - 17;

            if( FLAG_IS(board->square[to], opp_flag) )
                add_pawn_move(list, from, to);

            to = from - 15;

            if( FLAG_IS(board->square[to], opp_flag) )
                add_pawn_move(list, from, to);

            // promote

            if( SQUARE_RANK(from) == Rank2 )
                {
                to = from - 16;

                if( board->square[to] == Empty )
                    {
                    add_promote(list, MOVE_MAKE(from, to));
                    }
                }
            }
        }
    }

// add_quiet_moves()

static void add_quiet_moves( list_t *list, const board_t *board )
    {

    int me;
    const sq_t *ptr;
    int from, to;
    int piece;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    me = board->turn;

    // piece moves

    for ( ptr = &board->piece[me][0]; (from = *ptr) != SquareNone; ptr++ )
        {

        piece = board->square[from];

        switch( PIECE_TYPE(piece) )
            {
            case Knight64:
                to = from - 33;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 31;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 18;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 14;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 14;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 18;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 31;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 33;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            case Bishop64:
                for ( to = from - 17; board->square[to] == Empty; to -= 17 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from - 15; board->square[to] == Empty; to -= 15 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 15; board->square[to] == Empty; to += 15 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 17; board->square[to] == Empty; to += 17 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }

                break;

            case Rook64:
                for ( to = from - 16; board->square[to] == Empty; to -= 16 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from - 1; board->square[to] == Empty; to -= 1 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 1; board->square[to] == Empty; to += 1 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 16; board->square[to] == Empty; to += 16 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }

                break;

            case Queen64:
                for ( to = from - 17; board->square[to] == Empty; to -= 17 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from - 16; board->square[to] == Empty; to -= 16 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from - 15; board->square[to] == Empty; to -= 15 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from - 1; board->square[to] == Empty; to -= 1 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 1; board->square[to] == Empty; to += 1 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 15; board->square[to] == Empty; to += 15 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 16; board->square[to] == Empty; to += 16 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }
                for ( to = from + 17; board->square[to] == Empty; to += 17 )
                    {
                    LIST_ADD(list, MOVE_MAKE(from, to));
                    }

                break;

            case King64:

                to = from - 17;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 16;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 15;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from - 1;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 1;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 15;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 16;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                to = from + 17;
                if( board->square[to] == Empty )
                    LIST_ADD(list, MOVE_MAKE(from, to));

                break;

            default:

                ASSERT(false);
                break;
            }
        }

    // pawn moves

    if( COLOUR_IS_WHITE(me) )
        {
        for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
            {

            // non promotes

            if( SQUARE_RANK(from) != Rank7 )
                {
                to = from + 16;

                if( board->square[to] == Empty )
                    {
                    ASSERT(!SQUARE_IS_PROMOTE(to));
                    LIST_ADD(list, MOVE_MAKE(from, to));

                    if( SQUARE_RANK(from) == Rank2 )
                        {
                        to = from + 32;

                        if( board->square[to] == Empty )
                            {
                            ASSERT(!SQUARE_IS_PROMOTE(to));
                            LIST_ADD(list, MOVE_MAKE(from, to));
                            }
                        }
                    }
                }
            }
        }
    else
        { // black
        for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
            {

            // non promotes

            if( SQUARE_RANK(from) != Rank2 )
                {
                to = from - 16;

                if( board->square[to] == Empty )
                    {
                    ASSERT(!SQUARE_IS_PROMOTE(to));
                    LIST_ADD(list, MOVE_MAKE(from, to));

                    if( SQUARE_RANK(from) == Rank7 )
                        {
                        to = from - 32;

                        if( board->square[to] == Empty )
                            {
                            ASSERT(!SQUARE_IS_PROMOTE(to));
                            LIST_ADD(list, MOVE_MAKE(from, to));
                            }
                        }
                    }
                }
            }
        }
    }

// add_dangerous_pawn_moves()

void add_dangerous_pawn_moves( list_t *list, const board_t *board )
    {
    int from;
    int to;
    int inc = PAWN_MOVE_INC(board->turn);
    const sq_t *ptr;

    for ( ptr = &board->pawn[board->turn][0]; (from = *ptr) != SquareNone; ptr++ )
        {
        if( PAWN_RANK(from, board->turn) != Rank7 )
            {
            to = from + inc;

            if( is_passed(board, to, board->turn) || pawn_fork(board, to) )
                LIST_ADD(list, MOVE_MAKE(from, to));
            }
        }
    }

// add_promotes()

static void add_promotes( list_t *list, const board_t *board )
    {

    int me;
    int inc;
    const sq_t *ptr;
    int from, to;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    me = board->turn;

    inc = PAWN_MOVE_INC(me);

    for ( ptr = &board->pawn[me][0]; (from = *ptr) != SquareNone; ptr++ )
        {
        if( PAWN_RANK(from, me) == Rank7 )
            {
            to = from + inc;

            if( board->square[to] == Empty )
                {
                add_promote(list, MOVE_MAKE(from, to));
                }
            }
        }
    }

// add_en_passant_captures()

static void add_en_passant_captures( list_t *list, const board_t *board )
    {

    int from, to;
    int me;
    int inc;
    int pawn;

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    to = board->ep_square;

    if( to != SquareNone )
        {

        me = board->turn;

        inc = PAWN_MOVE_INC(me);
        pawn = PAWN_MAKE(me);

        from = to - (inc - 1);

        if( board->square[from] == pawn )
            {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list, MOVE_MAKE_FLAGS(from, to, MoveEnPassant));
            }

        from = to - (inc + 1);

        if( board->square[from] == pawn )
            {
            ASSERT(!SQUARE_IS_PROMOTE(to));
            LIST_ADD(list, MOVE_MAKE_FLAGS(from, to, MoveEnPassant));
            }
        }
    }

// add_castle_moves()

void add_castle_moves( list_t *list, const board_t *board )
    {

    ASSERT(list != NULL);
    ASSERT(board != NULL);

    ASSERT(!board_is_check(board));

    sq_t *sq;
    int flag;
    int me = board->turn;

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
            int to = CASTLE_TO(me, wing);

            if( wing == CastleQueen && RookSquare[me][wing] == to - 1 )
                {
                int piece = board->square[to - 2];

                if( COLOUR_IS(piece, COLOUR_OPP(me)) && FLAG_IS(piece, RookFlag) )
                    continue;
                }

            LIST_ADD(list, MOVE_MAKE_FLAGS(KingSquare[me], to, MoveCastle));
            }
        next:
        ;
        }
    }
// add_pawn_move()

void add_pawn_move( list_t *list, int from, int to )
    {

    int move;

    ASSERT(list != NULL);
    ASSERT(SQUARE_IS_OK(from));
    ASSERT(SQUARE_IS_OK(to));

    move = MOVE_MAKE(from, to);

    if( SQUARE_IS_PROMOTE(to) )
        {
        LIST_ADD(list, move | MovePromoteQueen);
        LIST_ADD(list, move | MovePromoteKnight);
        LIST_ADD(list, move | MovePromoteRook);
        LIST_ADD(list, move | MovePromoteBishop);
        }
    else
        {
        LIST_ADD(list, move);
        }
    }

// add_promote()

void add_promote( list_t *list, int move )
    {

    ASSERT(list != NULL);
    ASSERT(move_is_ok(move));

    ASSERT((move & ~07777) == 0);
    ASSERT(SQUARE_IS_PROMOTE(MOVE_TO(move)));

    LIST_ADD(list, move | MovePromoteQueen);
    LIST_ADD(list, move | MovePromoteKnight);
    LIST_ADD(list, move | MovePromoteRook);
    LIST_ADD(list, move | MovePromoteBishop);
    }

// end of move_gen.cpp
