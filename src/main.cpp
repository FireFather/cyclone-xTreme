// main.cpp

// includes

#include <cstdio>
#include <cstdlib>

#include "attack.h"
#include "book.h"
#include "hash.h"
#include "init.h"
#include "move_do.h"
#include "option.h"
#include "pawn.h"
#include "piece.h"
#include "probe.h"
#include "protocol.h"
#include "random.h"
#include "search.h"
#include "square.h"
#include "trans.h"
#include "util.h"
#include "value.h"
#include "vector.h"

// functions

// main()

int main()
    {
    util_init();
    my_random_init();
    show_version();
    option_init();
    square_init();
    piece_init();
    pawn_init_bit();
    value_init();
    vector_init();
    attack_init();
    move_do_init();
    random_init();
    hash_init();
    trans_init(Trans);
    read_ini_file("cyclone.cfg");
    trans_parameter();
    book_init();
	book_parameter();
    bitbase_parameter();
	threads_init();
    init();

    return EXIT_SUCCESS;
    }

// end of main.cpp
