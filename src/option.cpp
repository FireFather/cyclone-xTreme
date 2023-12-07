// option.cpp

// includes

#include <cstdlib>

#include "option.h"
#include "protocol.h"
#include "util.h"

// variables

option_t Option[256] =
    {
    { "Use Bitbases", true, "true", "check", "", NULL },
    { "Bitbase Path", true, "c:/egbb/", "string", "", NULL },
    { "Bitbase Cache", true, "16", "spin", "min 16 max 1024", NULL },
    { "OwnBook", true, "true", "check", "", NULL },
    { "BookFile", true, "performance.bin", "string", "", NULL },	
    { "Dynamic Hash", true, "true", "check", "", NULL },
    { "Nodes Per Second", true, "750000", "spin", "min 10000 max 10000000", NULL },
    { "Seconds Per Move", true, "10", "spin", "min 1 max 3600", NULL },
    { "Hash", true, "128", "spin", "min 8 max 1024", NULL },
    { "Max Threads", true, "32", "spin", "min 1 max 32", NULL },
    { "MultiPV", true, "1", "spin", "min 1 max 10", NULL },
    { "Ponder", true, "false", "check", "", NULL },
    { "Search Depth", true, "0", "spin", "min 0 max 20", NULL },
    { "Search Time", true, "0", "spin", "min 0 max 3600", NULL },
    { "UCI_Chess960", true, "false", "check", "", NULL },
    { NULL, false, NULL, NULL, NULL, NULL, },
    };

// prototypes

static option_t *option_find( option_t *options, const char var [] );

// functions

// option_init()

void option_init()
    {

    option_t *opt;

    for ( opt = &Option[0]; opt->var != NULL; opt++ )
        {
        option_set(Option, opt->var, opt->init);
        }
    }

// option_list()

void option_list()
    {

    option_t *opt;

    for ( opt = &Option[0]; opt->var != NULL; opt++ )
        {
        if( opt->declare )
            {
            if( opt->extra != NULL && *opt->extra != '\0' )
                {
                send("option name %s type %s default %s %s", opt->var, opt->type, opt->val, opt->extra);
                }
            else
                {
                send("option name %s type %s default %s", opt->var, opt->type, opt->val);
                }
            }
        }
    }

// option_set()

bool option_set( option_t *options, const char var [], const char val [] )
    {

    option_t *opt;

    ASSERT(var != NULL);
    ASSERT(val != NULL);

    opt = option_find(options, var);

    if( opt == NULL )
        {
        send("Option not found: %s", var);
        return false;
        }

    my_string_set(&opt->val, val);

    return true;
    }

// option_get()

const char *option_get( const char var [] )
    {

    option_t *opt;

    ASSERT(var != NULL);

    opt = option_find(Option, var);

    if( opt == NULL )
        my_fatal("option_get(): unknown option \"%s\"\n", var);

    return opt->val;
    }

// option_get_bool()

bool option_get_bool( const char var [] )
    {

    const char *val;

    val = option_get(var);

    if( false ) { }
    else if( my_string_equal(val, "true") || my_string_equal(val, "yes") || my_string_equal(val, "1") )
        {
        return true;
        }
    else if( my_string_equal(val, "false") || my_string_equal(val, "no") || my_string_equal(val, "0") )
        {
        return false;
        }

    ASSERT(false);

    return false;
    }

// option_get_int()

int option_get_int( const char var [] )
    {

    const char *val;

    val = option_get(var);

    return atoi(val);
    }

// option_get_string()

const char *option_get_string( const char var [] )
    {

    const char *val;

    val = option_get(var);

    return val;
    }

// option_find()

static option_t *option_find( option_t *options, const char var [] )
    {

    option_t *opt;

    ASSERT(var != NULL);

    for ( opt = &options[0]; opt->var != NULL; opt++ )
        {
        if( my_string_equal(opt->var, var) )
            return opt;
        }

    return NULL;
    }

// end of option.cpp
