// sort.h

#ifndef SORT_H
#define SORT_H

// includes

#include "attack.h"
#include "board.h"
#include "list.h"
#include "util.h"

// constants

const int KillerNb = 5;
const int KillerNoPruning = 3;

// types

struct sort_t
    {
    int depth;
    int height;
    int trans_killer;
    int killer[KillerNb];
    int gen;
    int test;
    int pos;
    int value;
    int valuePV;
    int capture_nb;
    board_t *board;
    const attack_t *attack;
    list_t list[1];
    list_t bad[1];
    };

// functions

extern void sort_init( int ThreadId );
extern void sort_init( sort_t *sort, board_t *board, const attack_t *attack, int depth, int height, int trans_killer,
    int ThreadId );
extern int sort_next( sort_t *sort, int ThreadId );
extern void sort_init_qs( sort_t *sort, board_t *board, const attack_t *attack, bool check );
extern int sort_next_qs( sort_t *sort );
extern void good_move( int move, const board_t *board, int depth, int height, int ThreadId );
extern void history_good( int move, const board_t *board, int ThreadId );
extern void history_bad( int move, const board_t *board, int ThreadId );
extern void history_reset( int move, const board_t *board, int ThreadId );
extern void note_moves( list_t *list, const board_t *board, int height, int trans_killer, int ThreadId );

#endif // !defined SORT_H

// end of sort.h
