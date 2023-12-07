// option.h

#ifndef OPTION_H
#define OPTION_H

// includes

#include "util.h"

// types

struct option_t
    {
    const char *var;
    bool declare;
    const char *init;
    const char *type;
    const char *extra;
    const char *val;
    };

// constants and variables

const int OptionNb = 256;
extern option_t Option[OptionNb];

// functions

extern void option_init();
extern void option_list();
extern bool option_set( option_t *options, const char var [], const char val [] );
extern const char *option_get( const char var [] );
extern bool option_get_bool( const char var [] );
extern int option_get_int( const char var [] );
extern const char *option_get_string( const char var [] );

#endif // !defined OPTION_H

// end of option.h
