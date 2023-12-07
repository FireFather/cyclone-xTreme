// trans.cpp

// includes

#include <stdlib.h>

#include "hash.h"
#include "init.h"
#include "move.h"
#include "option.h"
#include "protocol.h"
#include "trans.h"
#include "util.h"
#include "value.h"

#pragma warning( disable : 4244 )
// Level 2 conversion from 'int' to 'short', possible loss of data ->

// macros

#define ENTRY_DATE(entry)  ((entry)->date_flags>>4)
#define ENTRY_FLAGS(entry) ((entry)->date_flags&TransFlags)

// constants

static const int DateSize = 16;
static const int ClusterSize = 4;
static const int DepthNone = -128;

struct trans
    {
    entry_t *table;
    uint32 size;
    uint32 mask;
    int date;
    int age[DateSize];
    uint32 used;
    sint64 read_nb;
    sint64 read_hit;
    sint64 write_nb;
    sint64 write_hit;
    sint64 write_collision;
    };

// variables

trans_t Trans[1];
static uint32 hash_size = 128;

// prototypes

static void trans_set_date( trans_t *trans, int date );
static int trans_age( const trans_t *trans, int date );
static entry_t *trans_entry( trans_t *trans, uint64 key );
static bool entry_is_ok( const entry_t *entry );

// functions

// trans_parameter()

void trans_parameter()
    {
    static bool DynamicHash = false;
    static int spm = 10;
    static int nps = 750000;
    static int npm = 3750000;

    // UCI options

    DynamicHash = (option_get_bool("Dynamic Hash"));

    if( DynamicHash )
        {
        nps = (option_get_int("Nodes Per Second"));
        spm = (option_get_int("Seconds Per Move"));
        npm = spm * nps;
        hash_size = abs(npm * 32 / 1024 / 1024);
        trans_free(Trans);
        trans_alloc(Trans);
        }
    else
        {
        hash_size = option_get_int("Hash");
        trans_free(Trans);
        trans_alloc(Trans);
        }
    }

// trans_is_ok()

bool trans_is_ok( const trans_t *trans )
    {

    int date;

    if( trans == NULL )
        return false;

    if( trans->table == NULL )
        return false;

    if( trans->size == 0 )
        return false;

    if( trans->mask == 0 || trans->mask >= trans->size )
        return false;

    if( trans->date >= DateSize )
        return false;

    for ( date = 0; date < DateSize; date++ )
        {
        if( trans->age[date] != trans_age(trans, date) )
            return false;
        }

    return true;
    }

// trans_init()

void trans_init( trans_t *trans )
    {

    ASSERT(trans != NULL);

    ASSERT(sizeof(entry_t) == 16);

    trans->size = 0;
    trans->mask = 0;
    trans->table = NULL;

    trans_set_date(trans, 0);

    trans_clear(trans);
    }

// trans_alloc()

void trans_alloc( trans_t *trans )
    {

    uint32 size, target;

    ASSERT(trans != NULL);

    // calculate size

    target = hash_size;

    if( target < 8 )
        target = 8;

    if( target > 1024 )
        target = 1024;

    target *= 1024 * 1024;

    for ( size = 1; size != 0 && size <= target; size *= 2 )
    ;

    size /= 2;
    ASSERT(size > 0 && size <= target);

    // allocate table

    size /= sizeof(entry_t);
    ASSERT(size != 0 && (size &(size - 1)) == 0); // power of 2

    trans->size = size + (ClusterSize - 1);
    trans->mask = size - 1;

    trans->table = (entry_t *)malloc(trans->size * sizeof(entry_t));
    send("hash: %d MB\n", trans->size * sizeof(entry_t) / 1024 / 1024);

    trans_clear(trans);

    ASSERT(trans_is_ok(trans));
    }

// trans_free()

void trans_free( trans_t *trans )
    {

    ASSERT(trans_is_ok(trans));

    my_free(trans->table);

    trans->table = NULL;
    trans->size = 0;
    trans->mask = 0;
    }

// trans_clear()

void trans_clear( trans_t *trans )
    {

    entry_t clear_entry[1];
    entry_t *entry;
    uint32 index;

    ASSERT(trans != NULL);

    trans_set_date(trans, 0);

    clear_entry->key = 0;
    clear_entry->move = MoveNone;
    clear_entry->depth = DepthNone;
    clear_entry->date_flags = (trans->date << 4);

    ASSERT(entry_is_ok(clear_entry));

    entry = trans->table;

    for ( index = 0; index < trans->size; index++ )
        {
        *entry++ = *clear_entry;
        }
    }

// trans_inc_date()

void trans_inc_date( trans_t *trans )
    {

    ASSERT(trans != NULL);

    trans_set_date(trans, (trans->date + 1) % DateSize);
    }

// trans_set_date()

static void trans_set_date( trans_t *trans, int date )
    {

    ASSERT(trans != NULL);
    ASSERT(date >= 0 && date < DateSize);

    trans->date = date;

    for ( date = 0; date < DateSize; date++ )
        {
        trans->age[date] = trans_age(trans, date);
        }

    trans->used = 0;
    trans->read_nb = 0;
    trans->read_hit = 0;
    trans->write_nb = 0;
    trans->write_hit = 0;
    trans->write_collision = 0;
    }

// trans_age()

static int trans_age( const trans_t *trans, int date )
    {

    int age;

    ASSERT(trans != NULL);
    ASSERT(date >= 0 && date < DateSize);

    age = trans->date - date;

    if( age < 0 )
        age += DateSize;

    ASSERT(age >= 0 && age < DateSize);

    return age;
    }

// trans_store()

void trans_store( trans_t *trans, uint64 key, int move, int depth, int flags, int value )
    {

    entry_t *entry, *best_entry;
    int score, best_score;
    int i;

    ASSERT(trans_is_ok(trans));
    ASSERT(move >= 0 && move < 65536);
    ASSERT(depth >= 0 && depth < 256);
    ASSERT((flags & ~TransFlags) == 0);
    ASSERT(value >= -32767 && value <= +32767);

    // init

    trans->write_nb++;

    // probe

    best_entry = NULL;
    best_score = -32767;

    entry = trans_entry(trans, key);

    for ( i = 0; i < ClusterSize; i++, entry++ )
        {
        if( entry->key == key )
            {

            // hash hit => update existing entry

            trans->write_hit++;

            if( ENTRY_DATE(entry) != trans->date )
                trans->used++;

            if( entry->depth <= depth )
                {
                if( SmartMove && move == MoveNone )
                    move = entry->move;

                if( SmartValue && entry->depth == depth && entry->value == value )
                    {
                    flags |= ENTRY_FLAGS(entry);
                    }

                ASSERT(entry->key == key);
                entry->move = move;
                entry->depth = depth;
                entry->date_flags = (trans->date << 4) | flags;
                entry->value = value;
                }
            else
                { // deeper entry
                if( SmartMove && entry->move == MoveNone )
                    entry->move = move;
                entry->date_flags = (trans->date << 4) | ENTRY_FLAGS(entry);
                }

            return;
            }

        // evaluate replacement score

        score = trans->age[ENTRY_DATE(entry)] * 256 - entry->depth;

        if( SmartReplace )
            score = score * 4 - ENTRY_FLAGS(entry);

        ASSERT(score > -32767);

        if( score > best_score )
            {
            best_entry = entry;
            best_score = score;
            }
        }

    // "best" entry found

    entry = best_entry;
    ASSERT(entry != NULL);
    ASSERT(entry->key != key);

    if( ENTRY_DATE(entry) == trans->date )
        {

        trans->write_collision++;

        if( !AlwaysWrite && entry->depth > depth )
            {
            return; // do not replace deeper entries
            }
        }
    else
        {

        trans->used++;
        }

    // store

    ASSERT(entry != NULL);

    entry->key = key;
    entry->move = move;
    entry->depth = depth;
    entry->date_flags = (trans->date << 4) | flags;
    entry->value = value;
    }

// trans_retrieve()

bool trans_retrieve( trans_t *trans, uint64 key, int *move, int *depth, int *flags, int *value )
    {

    int i;
    entry_t *entry;

    ASSERT(trans_is_ok(trans));
    ASSERT(move != NULL);
    ASSERT(depth != NULL);
    ASSERT(flags != NULL);
    ASSERT(value != NULL);

    // init

    trans->read_nb++;

    // probe

    entry = trans_entry(trans, key);

    for ( i = 0; i < ClusterSize; i++, entry++ )
        {
        if( entry->key == key )
            {

            // found

            trans->read_hit++;

            *move = entry->move;
            *depth = entry->depth;
            *flags = ENTRY_FLAGS(entry);
            *value = entry->value;

            return true;
            }
        }

    // not found

    return false;
    }

// trans_stats()

void trans_stats( const trans_t *trans )
    {

    double full;

    ASSERT(trans_is_ok(trans));
    full = double(trans->used) / double(trans->size);
    send("info hashfull %.0f", full * 1000.0);
    }

// trans_entry()

static entry_t *trans_entry( trans_t *trans, uint64 key )
    {

    uint32 index;

    ASSERT(trans_is_ok(trans));

    if( UseModulo )
        {
        index = key % (trans->mask + 1);
        }
    else
        {
        index = key & trans->mask;
        }

    ASSERT(index <= trans->mask);

    return &trans->table[index];
    }

// entry_is_ok()

static bool entry_is_ok( const entry_t *entry )
    {
    if( entry == NULL )
        return false;

    if( ENTRY_DATE(entry) >= DateSize )
        return false;

    if( entry->move == MoveNone )
        return false;

    return true;
    }

// end of trans.cpp
