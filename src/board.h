// board.h

#ifndef BOARD_H
#define BOARD_H

// includes

#include "colour.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// constants

const int Empty = 0;
const int Edge = Knight64;
const int WP = WhitePawn256;
const int WN = WhiteKnight256;
const int WB = WhiteBishop256;
const int WR = WhiteRook256;
const int WQ = WhiteQueen256;
const int WK = WhiteKing256;
const int BP = BlackPawn256;
const int BN = BlackKnight256;
const int BB = BlackBishop256;
const int BR = BlackRook256;
const int BQ = BlackQueen256;
const int BK = BlackKing256;
const int FlagsNone = 0;
const int FlagsWhiteKingCastle = 1 << 0;
const int FlagsWhiteQueenCastle = 1 << 1;
const int FlagsBlackKingCastle = 1 << 2;
const int FlagsBlackQueenCastle = 1 << 3;
const int CastleQueen = 1;
const int CastleKing = 0;
const int StackSize = 4096;

// FRC variables

extern bool Chess960;
extern bool Arena;
extern int KingSquare[ColourNb];
extern int RookSquare[ColourNb][2];
extern int CastleSafe[ColourNb*2*6];
extern int CastleEmpty[ColourNb*2*6];

// macros

#define KING_POS(board,colour) ((board)->piece[colour][0])
#define CASTLE_FLAG(colour,wing) (1<<(2*(colour)+(wing)))
#define CASTLE_SAFE(colour,wing) (&CastleSafe[6*(2*(colour)+(wing))])
#define CASTLE_EMPTY(colour,wing)(&CastleEmpty[6*(2*(colour)+(wing))])
#define CASTLE_TO(colour,wing)  (((G1^((wing)*0x0F))+(wing))^((colour)*0xF0))

// types

struct board_t
    {

    int piece_material[ColourNb];

    int square[SquareNb];
    int pos[SquareNb];

    sq_t piece[ColourNb][32]; // only 17 are needed
    int piece_size[ColourNb];

    sq_t pawn[ColourNb][16];  // only 9 are needed
    int pawn_size[ColourNb];

    int piece_nb;
    int number[16]; // only 12 are needed

    int pawn_file[ColourNb][FileNb];

    int turn;
    int flags;
    int ep_square;
    int ply_nb;
    int sp; // TODO: MOVE ME?

    int cap_sq;
    int moving_piece;

    int opening;
    int endgame;

    uint64 key;
    uint64 pawn_key;
    uint64 material_key;

    uint64 stack[StackSize];
    };

// functions

extern bool board_is_ok( const board_t *board );
extern void board_clear( board_t *board );
extern void board_copy( board_t *dst, const board_t *src );
extern void board_init_list( board_t *board );
extern bool board_is_legal( const board_t *board );
extern bool board_is_check( const board_t *board );
extern bool board_is_mate( const board_t *board );
extern bool board_is_stalemate( board_t *board );
extern bool board_is_repetition( const board_t *board );
extern int board_material( const board_t *board );
extern int board_opening( const board_t *board );
extern int board_endgame( const board_t *board );
extern void frc_init();

#endif // !defined BOARD_H

// end of board.h
