// fen.cpp

// includes

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "board.h"
#include "colour.h"
#include "fen.h"
#include "piece.h"
#include "square.h"
#include "util.h"

#pragma warning( disable : 4800 )
// Level 2 'int' : forcing value to bool 'true' or 'false' (performance warning) ->

// "constants"

const char *const StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// variables

static const bool Strict = false;

// functions

// board_from_fen()

void board_from_fen( board_t *board, const char old_fen [] )
    {

    int pos;
    int file, rank, sq;
    int c;
    int i, len, j;
    int piece;
    int pawn;
    char fen[256];

    ASSERT(board != NULL);
    ASSERT(fen != NULL);

    for ( i = 0; i < 2; i++ )
        {
        KingSquare[i] = SquareNone;

        for ( j = 0; j < 2; j++ )
            RookSquare[i][j] = SquareNone;
        }

    board_clear(board);
    strcpy(fen, old_fen);

    convert_fen(fen, true);
    Arena = strcmp(old_fen, fen);

    pos = 0;
    c = fen[pos];

    // piece placement

    for ( rank = Rank8; rank >= Rank1; rank-- )
        {
        for ( file = FileA; file <= FileH; )
            {
            if( c >= '1' && c <= '8' )
                { // empty square(s)

                len = c - '0';

                for ( i = 0; i < len; i++ )
                    {
                    if( file > FileH )
                        my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
                    board->square[SQUARE_MAKE(file, rank)] = Empty;
                    file++;
                    }
                }
            else
                { // piece

                piece = piece_from_char(c);
                sq = SQUARE_MAKE(file, rank);

                if( piece == PieceNone256 )
                    my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);

                // castling

                if( PIECE_IS_KING(piece) && PAWN_RANK(sq, PIECE_COLOUR(piece)) == Rank1 )
                    KingSquare[PIECE_COLOUR(piece)] = sq;
                board->square[sq] = piece;
                file++;
                }

            c = fen[++pos];
            }

        if( rank > Rank1 )
            {
            if( c != '/' )
                my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
            c = fen[++pos];
            }
        }

    // active colour

    if( c != ' ' )
        my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
    c = fen[++pos];

    switch( c )
        {
        case 'w':
            board->turn = White;
            break;

        case 'b':
            board->turn = Black;
            break;

        default:
            my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
            break;
        }

    c = fen[++pos];

    // castling

    if( c != ' ' )
        my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
    c = fen[++pos];

    board->flags = FlagsNone;

    if( c == '-' )
        { // no castling rights

        c = fen[++pos];
        }
    else
        {
        do
            {
            int colour = isupper(c) ? White : Black;
            sq = SQUARE_MAKE(FileA + tolower(c) - 'a', colour == White ? Rank1 : Rank8);
            int wing = sq > KingSquare[colour] ? CastleKing : CastleQueen;
            int rook = Rook64 | COLOUR_FLAG(colour);

            if( board->square[sq] == rook && KingSquare[colour] != SquareNone )
                {
                board->flags |= CASTLE_FLAG(colour, wing);
                RookSquare[colour][wing] = sq;
                int king = KingSquare[colour];

                if( king != E1 && king != E8 || sq != king + 3 && sq != king - 4 )
                    Chess960 = true;
                }
            } while ( (c = fen[++pos]) != ' ' );
        }

    if( !(board->flags &(CASTLE_FLAG(0, 0) | CASTLE_FLAG(0, 1))) )
        KingSquare[White] = SquareNone;

    if( !(board->flags &(CASTLE_FLAG(1, 0) | CASTLE_FLAG(1, 1))) )
        KingSquare[Black] = SquareNone;
    frc_init();

    // en-passant

    if( c != ' ' )
        my_fatal("board_from_fen(): bad FEN (pos=%d),\n", pos);
    c = fen[++pos];

    if( c == '-' )
        { // no en-passant

        sq = SquareNone;
        c = fen[++pos];
        }
    else
        {
        if( c < 'a' || c > 'h' )
            my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
        file = file_from_char(c);
        c = fen[++pos];

        if( c != (COLOUR_IS_WHITE(board->turn) ? '6' : '3') )
            my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
        rank = rank_from_char(c);
        c = fen[++pos];

        sq = SQUARE_MAKE(file, rank);
        pawn = SQUARE_EP_DUAL(sq);

        if( board->square[sq] != Empty || board->square[pawn] != PAWN_MAKE(COLOUR_OPP(board->turn))
            || (board->square[pawn - 1] != PAWN_MAKE(board->turn)
                && board->square[pawn + 1] != PAWN_MAKE(board->turn)) )
            {
            sq = SquareNone;
            }
        }

    board->ep_square = sq;

    // halfmove clock

    board->ply_nb = 0;

    if( c != ' ' )
        {
        if( !Strict )
            goto update;
        my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
        }
    c = fen[++pos];

    if( !isdigit(c) )
        {
        if( !Strict )
            goto update;
        my_fatal("board_from_fen(): bad FEN (pos=%d)\n", pos);
        }

    board->ply_nb = atoi(&fen[pos]);

    // board update

    update:
    board_init_list(board);
    }

// board_to_fen()

bool board_to_fen( const board_t *board, char fen [], int size )
    {

    int pos;
    int file, rank;
    int sq, piece;
    int c;
    int len;

    ASSERT(board != NULL);
    ASSERT(fen != NULL);
    ASSERT(size >= 92);

    // init

    if( size < 92 )
        return false;

    pos = 0;

    // piece placement

    for ( rank = Rank8; rank >= Rank1; rank-- )
        {
        for ( file = FileA; file <= FileH; )
            {

            sq = SQUARE_MAKE(file, rank);
            piece = board->square[sq];
            ASSERT(piece == Empty || piece_is_ok(piece));

            if( piece == Empty )
                {

                len = 0;

                for (; file <= FileH && board->square[SQUARE_MAKE(file, rank)] == Empty; file++ )
                    {
                    len++;
                    }

                ASSERT(len >= 1 && len <= 8);
                c = '0' + len;
                }
            else
                {

                c = piece_to_char(piece);
                file++;
                }

            fen[pos++] = c;
            }

        fen[pos++] = '/';
        }

    fen[pos - 1] = ' ';

    // active colour

    fen[pos++] = COLOUR_IS_WHITE(board->turn) ? 'w' : 'b';
    fen[pos++] = ' ';

    // castling

    if( board->flags == FlagsNone )
        {
        fen[pos++] = '-';
        }
    else
        { // don't bother generating bother X-FEN. Also, note Arena is always true if !Chess960.
        if( (board->flags &FlagsWhiteKingCastle) != 0 )
            fen[pos++] = 'A' + SQUARE_FILE(RookSquare[White][CastleKing]) - FileA;

        if( (board->flags &FlagsWhiteQueenCastle) != 0 )
            fen[pos++] = 'A' + SQUARE_FILE(RookSquare[White][CastleQueen]) - FileA;

        if( (board->flags &FlagsBlackKingCastle) != 0 )
            fen[pos++] = 'a' + SQUARE_FILE(RookSquare[Black][CastleKing]) - FileA;

        if( (board->flags &FlagsBlackQueenCastle) != 0 )
            fen[pos++] = 'a' + SQUARE_FILE(RookSquare[Black][CastleQueen]) - FileA;
        }

    fen[pos++] = ' ';

    // en-passant

    if( board->ep_square == SquareNone )
        {
        fen[pos++] = '-';
        }
    else
        {
        square_to_string(board->ep_square, &fen[pos], 3);
        pos += 2;
        }

    fen[pos++] = ' ';

    // halfmove clock

    sprintf(&fen[pos], "%d 1", board->ply_nb);

    if( Arena )
        convert_fen(fen, false);

    return true;
    }

// convert_fen()

void convert_fen( char fen [], bool to_shredder )
    { // no-op if it's already in the specified format
    int pos = 0;
    char rook[4] =
        {
            '\0', '\0', '\0', '\0'
        }; // in qkQK order

    char file = 'a';
    char king[2] =
        {
            '\0', '\0'
        }; // black,white

    int c;
    int i = 0;
    int case_diff = 'A' - 'a';
    int size = strlen(fen);

    for ( int colour = 0; colour < 2; colour++ )
        {
        while( pos < size && (c = fen[pos++]) != '/' && c != ' ' )
            {
            if( c >= '1' && c <= '7' )
                file += c - '1';

            if( c == 'r' + colour * case_diff )
                {
                if( king[colour] )
                    rook[2 * colour + 1] = file;

                else if( !rook[2*colour] )
                    rook[2*colour] = file;
                }

            if( c == 'k' + colour * case_diff )
                king[colour] = file;
            file++;
            }

        while( i++ < 6 ) // skip ranks 7 through 2
            while( pos < size && fen[pos++] != '/' );
        file = 'A';
        }

    pos++;

    while( pos < size - 1 && (c = fen[++pos]) != ' ' )
        {
        if( to_shredder )
            {
            if( c == 'q' && rook[0] )
                fen[pos] = rook[0]; // avoid mangling FENs that were incorrect in the first place

            if( c == 'k' && rook[1] )
                fen[pos] = rook[1];

            if( c == 'Q' && rook[2] )
                fen[pos] = rook[2];

            if( c == 'K' && rook[3] )
                fen[pos] = rook[3];
            }
        else
            { // to X-FEN, then.
            if( c >= 'a' && c <= 'h' )
                {
                if( c < king[0] && c == rook[0] )
                    fen[pos] = 'q'; // don't do anything to inner rooks because I'm lazy and no compatible GUI uses it

                if( c > king[0] && c == rook[1] )
                    fen[pos] = 'k';
                }
            else if( c >= 'A' && c <= 'H' )
                {
                if( c < king[1] && c == rook[2] )
                    fen[pos] = 'Q';

                if( c > king[1] && c == rook[3] )
                    fen[pos] = 'K';
                }
            }
        }
    }

// end of fen.cpp
