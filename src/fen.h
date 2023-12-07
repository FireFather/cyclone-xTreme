// fen.h

#ifndef FEN_H
#define FEN_H

// includes

#include "board.h"
#include "util.h"

// "constants"

extern const char * const StartFen;

// functions

extern void board_from_fen( board_t *board, const char fen [] );
extern bool board_to_fen( const board_t *board, char fen [], int size );
extern void convert_fen( char fen [], bool to_shredder );

#endif // !defined FEN_H

// end of fen.h
