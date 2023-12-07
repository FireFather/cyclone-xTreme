// protocol.h

#ifndef PROTOCOL_H
#define PROTOCOL_H

#define ENGINE "Cyclone"
#define VERSION "xTreme"
#define AUTHOR "Kranium"

// includes

#include "util.h"

// variables

// functions

extern void init();
extern void event();

extern void get( char string [], int size );
extern void send( const char format [], ... );

#endif // !defined PROTOCOL_H

// end of protocol.h
