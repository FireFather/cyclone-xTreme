// pawn.h

#ifndef PAWN_H
#define PAWN_H

// includes

#include "board.h"
#include "colour.h"
#include "util.h"

// macros

#define BIT(n)       (BitEQ[n])
#define BIT_FIRST(b) (BitFirst[b])
#define BIT_LAST(b)  (BitLast[b])
#define BIT_COUNT(b) (BitCount[b])
#define WEAK_SQUARE_WHITE(sq) (board->square[sq-15] != WP && board->square[sq-17] != WP)
#define WEAK_SQUARE_BLACK(sq) (board->square[sq+15] != BP && board->square[sq+17] != BP)

// constants

const int BackRankFlag = 1 << 0;

// types

struct pawn_info_t
    {
    uint32 lock;
    sint16 opening;
    sint16 endgame;
    uint8 flags[ColourNb];
    uint8 passed_bits[ColourNb];
    uint8 single_file[ColourNb];
    uint16 pad;
    };

// variables

extern int BitEQ[16];
extern int BitLT[16];
extern int BitLE[16];
extern int BitGT[16];
extern int BitGE[16];
extern int BitFirst[0x100];
extern int BitLast[0x100];
extern int BitCount[0x100];
extern int BitRev[0x100];

// functions

extern void pawn_init_bit();
extern void pawn_init();
extern void pawn_alloc();
extern void pawn_clear( int ThreadId );
extern void pawn_get_info( pawn_info_t *info, const board_t *board, int ThreadId );
extern int quad( int y_min, int y_max, int x );
extern bool is_passed( const board_t *board, int to, int me );
extern bool pawn_fork( const board_t *board, int to );

#endif // !defined PAWN_H

// end of pawn.h
