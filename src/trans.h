// trans.h

#ifndef TRANS_H
#define TRANS_H

// includes

#include "util.h"

// constants

const int TransUpperFlag = 1 << 0;
const int TransLowerFlag = 1 << 1;
const int TransEGBBFlag = 1 << 2;
const int TransUnknown = 0;
const int TransUpper = TransUpperFlag;
const int TransLower = TransLowerFlag;
const int TransExact = TransUpperFlag | TransLowerFlag;
const int TransEGBB = TransEGBBFlag;
const int TransFlags = 3;

// macros

#define TRANS_IS_UPPER(flags) (((flags)&TransUpperFlag)!=0)
#define TRANS_IS_LOWER(flags) (((flags)&TransLowerFlag)!=0)
#define TRANS_IS_EXACT(flags) ((flags)==TransExact)

struct entry_t
    {
    uint64 key;
    uint16 move;
    uint8 depth;
    uint8 date_flags;
    sint16 value;
    uint16 nproc;
    };

// types

typedef struct trans trans_t;

// variables

extern trans_t Trans[1];
extern bool trans_endgame;

// functions

extern void trans_parameter();
extern bool trans_is_ok( const trans_t *trans );
extern void trans_init( trans_t *trans );
extern void trans_alloc( trans_t *trans );
extern void trans_free( trans_t *trans );
extern void trans_clear( trans_t *trans );
extern void trans_inc_date( trans_t *trans );
extern void trans_store( trans_t *trans, uint64 key, int move, int depth, int flags, int value );
extern bool trans_retrieve( trans_t *trans, uint64 key, int *move, int *depth, int *flags, int *value );
extern void trans_stats( const trans_t *trans );

#endif // !defined TRANS_H

// end of trans.h
