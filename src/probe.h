// probe.h

#ifndef PROBE_H

#define PROBE_H

// types

typedef int (*PPROBE_EGBB) (int player, int w_king, int b_king,
                            int piece1, int square1,
                            int piece2, int square2,
                            int piece3, int square3);

typedef void (*PLOAD_EGBB) (char* path,int cache_size,int load_options);

// "constants"

#define _NOTFOUND 99999

// variables

extern bool egbb_is_loaded;
extern bool use_bitbases;

// functions

extern void bitbase_parameter();
extern int LoadEgbbLibrary( char *main_path, uint32 egbb_cache_size );
extern int probe_bitbases( board_t *board, int &score );

#endif // !defined PROBE_H

// end of probe.h
