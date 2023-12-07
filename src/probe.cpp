// probe.cpp

// includes

#include <windows.h>


#include "board.h"
#include "option.h"
#include "probe.h"
#include "protocol.h"
#include "value.h"

#pragma warning( disable : 4800 )
// Level 2 forcing value to bool 'true' or 'false' (performance warning) ->

// "constants"

enum
    {
    _EMPTY,
    _WKING,
    _WQUEEN,
    _WROOK,
    _WBISHOP,
    _WKNIGHT,
    _WPAWN,
    _BKING,
    _BQUEEN,
    _BROOK,
    _BBISHOP,
    _BKNIGHT,
    _BPAWN
    };

enum
    {
    LOAD_NONE,
    LOAD_4MEN,
    SMART_LOAD,
    LOAD_5MEN
    };

static int egbb_load_type = LOAD_4MEN;

// macros

// Load the dll and get the address of the load and probe functions.

//#define EGBB_64

#ifdef EGBB_64
#define EGBB_NAME "egbb64dll.dll"
#else
#define EGBB_NAME "egbbdll.dll"
#endif

// variables

// variables

bool use_bitbases = true;
bool egbb_is_loaded;
static int egbb_cache = 0;
static char *egbb_path = "";
static PPROBE_EGBB probe_egbb;

// functions

void bitbase_parameter()
    {
    // UCI options

    use_bitbases = option_get_bool("Use Bitbases");

    if( use_bitbases )
        {
        egbb_cache = (option_get_int("Bitbase Cache") * 1024 * 1024);
        egbb_path = (char *)option_get_string("Bitbase Path");
        egbb_is_loaded = LoadEgbbLibrary(egbb_path, egbb_cache);
        }
    }

// functions

int LoadEgbbLibrary( char *main_path, uint32 egbb_cache_size )
    {
    static HMODULE hmod;
    PLOAD_EGBB load_egbb;
    char path[256];

    strcpy(path, main_path);
    strcat(path, EGBB_NAME);

    if( hmod )
        FreeLibrary(hmod);

    if( hmod = LoadLibrary(path) )
        {
        load_egbb = (PLOAD_EGBB)GetProcAddress(hmod, "load_egbb_5men");
        probe_egbb = (PPROBE_EGBB)GetProcAddress(hmod, "probe_egbb_5men");
        load_egbb(main_path, egbb_cache_size, egbb_load_type);
        printf("\n");
        return true;
        }
    else
        {
        return false;
        }
    }

// probe_bitbases

int probe_bitbases( board_t *board, int &score )
    {

    ASSERT(board != NULL);
    ASSERT(board->piece_nb <= 5);

    int piece[5];
    int square[5];
    int from;
    int count;
    sq_t *ptr;

    count = 0;
    piece[0] = _EMPTY;
    piece[1] = _EMPTY;
    piece[2] = _EMPTY;
    square[0] = _EMPTY;
    square[1] = _EMPTY;
    square[2] = _EMPTY;

    for ( ptr = &board->piece[White][1]; (from = *ptr) != SquareNone; ptr++ )
        {
        square[count] = SquareTo64[from];
        piece[count++] = -((PIECE_TO_12(board->square[from]) >> 1) - 6);
        }

    for ( ptr = &board->pawn[White][0]; (from = *ptr) != SquareNone; ptr++ )
        {
        square[count] = SquareTo64[from];
        piece[count++] = _WPAWN;
        }

    for ( ptr = &board->piece[Black][1]; (from = *ptr) != SquareNone; ptr++ )
        {
        square[count] = SquareTo64[from];
        piece[count++] = -((PIECE_TO_12(board->square[from]) >> 1) - 12);
        }

    for ( ptr = &board->pawn[Black][0]; (from = *ptr) != SquareNone; ptr++ )
        {
        square[count] = SquareTo64[from];
        piece[count++] = _BPAWN;
        }

    score =
        probe_egbb(board->turn, SquareTo64[board->piece[White][0]], SquareTo64[board->piece[Black][0]], piece[0],
            square[0], piece[1], square[1], piece[2], square[2]);

    if( score < 0 && board_is_mate(board) )
        score = VALUE_MATE(0);

    if( score != _NOTFOUND )
        {
        return true;
        }

    return false;
    }

// end of probe.cpp
