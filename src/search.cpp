// search.cpp

// includes

#include <windows.h>
#include <csetjmp>
#include <cstring>
#include <process.h>

#include "attack.h"
#include "board.h"
#include "book.h"
#include "colour.h"
#include "eval.h"
#include "init.h"
#include "list.h"
#include "material.h"
#include "move.h"
#include "move_check.h"
#include "move_do.h"
#include "move_gen.h"
#include "option.h"
#include "pawn.h"
#include "piece.h"
#include "protocol.h"
#include "pst.h"
#include "pv.h"
#include "recog.h"
#include "search.h"
#include "see.h"
#include "sort.h"
#include "trans.h"
#include "util.h"
#include "value.h"
#include "probe.h"

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define NODE_OPP(type)     (-(type))
#define usable_egbb_value(probe_score,alpha,beta) (((beta > -ValueEvalInf || probe_score >= 0) \
         && (alpha < ValueEvalInf || probe_score <= 0)) || probe_score < -ValueEvalInf)
#define NODE_NEXT(type)     (type == NodePV ? NodePV : NodeAll)
#define DEPTH_MATCH(d1,d2) ((d1)>=(d2))

// constants

static const bool DispBest = true;
static const bool DispDepthStart = true;
static const bool DispDepthEnd = true;
static const bool DispRoot = true;
static const bool DispStat = true;

static const double EasyRatio = 0.20;
static const double EarlyRatio = 0.60;

// transposition table

static const bool UseTrans = true;
static const int TransDepth = 1;
static const bool UseExact = true;

static int CheckDepth[MaxThreads];

// misc

static const int NodeAll = -1;
static const int NodePV = 0;
static const int NodeCut = +1;

// variables

CRITICAL_SECTION CriticalSection;
static search_multipv_t save_multipv[MultiPVMax];
static HANDLE thread_handle[MaxThreads];
int NumberThreads = 16;

search_input_t SearchInput[1];
search_info_t SearchInfo[MaxThreads][1];
search_root_t SearchRoot[MaxThreads][1];
search_current_t SearchCurrent[MaxThreads][1];
search_best_t SearchBest[MaxThreads][MultiPVMax];

// prototypes

static void search_send_stat( int ThreadId );
unsigned __stdcall search_thread( void *param );
static int full_root( list_t *list, board_t *board, int alpha, int beta, int depth, int height, int search_type,
    int ThreadId );
static int full_search( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int node_type,
    bool extended, int ThreadId );
static int full_no_null( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int node_type,
    int trans_move, int *best_move, bool extended, int ThreadId );
static int full_quiescence( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int ThreadId );
static int full_new_depth( int depth, int move, board_t *board, bool single_reply, bool in_pv, int height,
    bool extended, bool *cap_extended, int ThreadId );
static bool do_null( const board_t *board );
static bool do_ver( const board_t *board );
static void pv_fill( const mv_t pv [], board_t *board );
static bool move_is_dangerous( int move, const board_t *board );
static bool capture_is_dangerous( int move, const board_t *board );
static bool simple_stalemate( const board_t *board );
static bool passed_pawn_move( int move, const board_t *board );
static int capture_value( int move, const board_t *board );

// functions

// init_threads()

void threads_init()
    {
    SYSTEM_INFO sys_info;
    int thread_count, option_max;
    GetSystemInfo(&sys_info);
    thread_count = sys_info.dwNumberOfProcessors;

    if( thread_count > MaxThreads )
        thread_count = MaxThreads;

    if( !MP )
        thread_count = 1;

    option_max = option_get_int("Max Threads");

    if( thread_count > option_max )
        thread_count = option_max;

    NumberThreads = thread_count;
    }

// depth_is_ok()

bool depth_is_ok( int depth )
    {

    return depth > -128 && depth < DepthMax;
    }

// height_is_ok()

bool height_is_ok( int height )
    {

    return height >= 0 && height < HeightMax;
    }

// search_clear()

void search_clear()
    {

    int ThreadId;

    // SearchInput

    SearchInput->infinite = false;
    SearchInput->depth_is_limited = false;
    SearchInput->depth_limit = 0;
    SearchInput->time_is_limited = false;
    SearchInput->time_limit_1 = 0.0;
    SearchInput->time_limit_2 = 0.0;

    // SearchInfo

    for ( ThreadId = 0; ThreadId < NumberThreads; ThreadId++ )
        {
        SearchInfo[ThreadId]->can_stop = false;
        SearchInfo[ThreadId]->stop = false;
        SearchInfo[ThreadId]->stopped = false;
        SearchInfo[ThreadId]->check_nb = 10000;
        SearchInfo[ThreadId]->check_inc = 10000;
        SearchInfo[ThreadId]->last_time = 0.0;

        // SearchBest

        SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].move = MoveNone;
        SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value = 0;
        SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].flags = SearchUnknown;
        SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].depth = 0;
        PV_CLEAR(SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].pv);

        // SearchRoot

        SearchRoot[ThreadId]->depth = 0;
        SearchRoot[ThreadId]->move = MoveNone;
        SearchRoot[ThreadId]->move_pos = 0;
        SearchRoot[ThreadId]->move_nb = 0;
        SearchRoot[ThreadId]->last_value = 0;
        SearchRoot[ThreadId]->bad_1 = false;
        SearchRoot[ThreadId]->bad_2 = false;
        SearchRoot[ThreadId]->change = false;
        SearchRoot[ThreadId]->easy = false;
        SearchRoot[ThreadId]->flag = false;

        // SearchCurrent

        SearchCurrent[ThreadId]->max_depth = 0;
        SearchCurrent[ThreadId]->node_nb = 0;
        SearchCurrent[ThreadId]->time = 0.0;
        SearchCurrent[ThreadId]->speed = 0.0;
        SearchCurrent[ThreadId]->cpu = 0.0;
        }
    }

void start_suspend_threads()
    {

    static int ThreadIds[MaxThreads];
    static unsigned internalThreadIds[MaxThreads];
    int i;

    // start and suspend threads

    for ( i = 1; i < NumberThreads; i++ )
        {
        ThreadIds[i - 1] = i;
        thread_handle[i - 1] =
            (HANDLE)_beginthreadex(NULL, 0, &search_thread, &ThreadIds[i - 1], CREATE_SUSPENDED,
                &internalThreadIds[i - 1]);
        }
    }

void resume_threads()
    {

    int i;

    // resume threads

    for ( i = 1; i < NumberThreads; i++ )
        {
        ResumeThread(thread_handle[i - 1]);
        }
    }

// search()

void search()
    {

    int i;
	int move;
    bool all_stopped;
    int ThreadId;

    for ( i = 0; i < MultiPVMax; i++ )
        {
        save_multipv[i].mate = 0;
        save_multipv[i].depth = 0;
        save_multipv[i].max_depth = 0;
        save_multipv[i].value = 0;
        save_multipv[i].time = 0;
        save_multipv[i].node_nb = 0;
        strcpy(save_multipv[i].pv_string, "");
        }

    SearchInput->multipv = option_get_int("MultiPV") - 1;

    for ( ThreadId = 0; ThreadId < NumberThreads; ThreadId++ )
        {
        SearchCurrent[ThreadId]->multipv = 0;
        }

    ASSERT(board_is_ok(SearchInput->board));

    // opening book

    if( option_get_bool("OwnBook") && !SearchInput->infinite )
        {

        move = book_move(SearchInput->board);

        if( move != MoveNone )
            {

            // play book move

            SearchBest[0][SearchCurrent[ThreadId]->multipv].move = move;
            SearchBest[0][SearchCurrent[ThreadId]->multipv].value = 1;
            SearchBest[0][SearchCurrent[ThreadId]->multipv].flags = SearchExact;
            SearchBest[0][SearchCurrent[ThreadId]->multipv].depth = 1;
            SearchBest[0][SearchCurrent[ThreadId]->multipv].pv[0] = move;
            SearchBest[0][SearchCurrent[ThreadId]->multipv].pv[1] = MoveNone;

            search_update_best(0);

            return;
            }
        }

    // SearchInput

    gen_legal_moves(SearchInput->list, SearchInput->board);

    if( LIST_SIZE(SearchInput->searchmoves) != 0 )
        {
        list_filter(SearchInput->list, (board_t *)SearchInput->searchmoves, (move_test_t)list_contain, true);
        }

    if( LIST_SIZE(SearchInput->list) < SearchInput->multipv + 1 )
        {
        SearchInput->multipv = LIST_SIZE(SearchInput->list) - 1;
        }

    if( LIST_SIZE(SearchInput->list) <= 1 )
        {
        SearchInput->depth_is_limited = true;
        SearchInput->depth_limit = 4;
        }

    trans_inc_date(Trans);

    // resume threads

    resume_threads();
    search_smp(0);

    for ( ThreadId = 1; ThreadId < NumberThreads; ThreadId++ )
        { // stop threads
        SearchInfo[ThreadId]->stop = true;
        }

    all_stopped = false;

    while( !all_stopped )
        {
        all_stopped = true;

        for ( ThreadId = 1; ThreadId < NumberThreads; ThreadId++ )
            {
            if( !SearchInfo[ThreadId]->stopped )
                all_stopped = false;
            }
        }
    }

unsigned __stdcall search_thread( void *param )
    {

    int ThreadId = *((int *)param);

    while( !SearchInput->exit_engine )
        {
        search_smp(ThreadId);
        SearchInfo[ThreadId]->stopped = true;
        SuspendThread(thread_handle[ThreadId - 1]);
        }

    _endthreadex(0);
    return 0;
    }

// search_smp()

void search_smp( int ThreadId )
    {

    int depth;
    int i;
    bool search_ready;
    sint64 node_nb;
    double speed;

    // SearchInfo

    if( setjmp(SearchInfo[ThreadId]->buf) != 0 )
        {
        ASSERT(SearchInfo[ThreadId]->can_stop);
        ASSERT(SearchBest[ThreadId]->move != MoveNone);
        search_update_current(ThreadId);
        return;
        }

    // SearchRoot

    list_copy(SearchRoot[ThreadId]->list, SearchInput->list);

    // SearchCurrent

    board_copy(SearchCurrent[ThreadId]->board, SearchInput->board);
    my_timer_reset(SearchCurrent[ThreadId]->timer);
    my_timer_start(SearchCurrent[ThreadId]->timer);

    // init

    sort_init(ThreadId);
    search_full_init(SearchRoot[ThreadId]->list, SearchCurrent[ThreadId]->board, ThreadId);

    // iterative deepening

    search_ready = false;

    if( ThreadId == 0 )
        { // main thread
        for ( depth = 1; depth < DepthMax; depth++ )
            {
            for ( SearchCurrent[ThreadId]->multipv = 0; SearchCurrent[ThreadId]->multipv <= SearchInput->multipv;
                SearchCurrent[ThreadId]->multipv++ )
                {
                if( DispDepthStart && SearchCurrent[ThreadId]->multipv == 0 )
                    send("info depth %d", depth);

                SearchCurrent[ThreadId]->act_iteration = depth;
                SearchRoot[ThreadId]->bad_1 = false;
                SearchRoot[ThreadId]->change = false;

                board_copy(SearchCurrent[ThreadId]->board, SearchInput->board);

                if( UseShortSearch && depth <= ShortSearchDepth )
                    {
                    search_full_root(SearchRoot[ThreadId]->list, SearchCurrent[ThreadId]->board, depth, SearchShort,
                        ThreadId);
                    }
                else
                    {
                    search_full_root(SearchRoot[ThreadId]->list, SearchCurrent[ThreadId]->board, depth, SearchNormal,
                        ThreadId);
                    }

                search_update_current(ThreadId);

                node_nb = 0;
                speed = 0;

                for ( i = 0; i < NumberThreads; i++ )
                    {
                    node_nb += SearchCurrent[ThreadId]->node_nb;
                    speed += SearchCurrent[ThreadId]->speed;
                    }

                if( DispDepthEnd && SearchCurrent[ThreadId]->multipv == SearchInput->multipv )
                    {
                    send("info depth %d seldepth %d time %.0f nodes " S64_FORMAT " nps %.0f",
                 depth,SearchCurrent[ThreadId]->max_depth,SearchCurrent[ThreadId]->time*1000.0,node_nb,speed);
                    }

                // update search info

                if( depth >= 1 )
                    SearchInfo[ThreadId]->can_stop = true;

                if( depth == 1 && LIST_SIZE(SearchRoot[ThreadId]->list) >= 2
                    && LIST_VALUE(SearchRoot[ThreadId]->list, 0) >= LIST_VALUE(SearchRoot[ThreadId]->list, 1)
                    + EasyThreshold )
                    {
                    SearchRoot[ThreadId]->easy = true;
                    }

                if( UseBad && depth > 1 )
                    {
                    SearchRoot[ThreadId]->bad_2 = SearchRoot[ThreadId]->bad_1;
                    SearchRoot[ThreadId]->bad_1 = false;
                    ASSERT(SearchRoot[ThreadId]->bad_2
                        == (SearchBest[ThreadId]->value <= SearchRoot[ThreadId]->last_value - BadThreshold));
                    }

                SearchRoot[ThreadId]->last_value = SearchBest[ThreadId][0].value;

                // stop search?

                if( SearchInput->depth_is_limited && SearchCurrent[ThreadId]->multipv >= SearchInput->multipv
                    && depth >= SearchInput->depth_limit )
                    {
                    SearchRoot[ThreadId]->flag = true;
                    }

                if( SearchInput->time_is_limited && SearchCurrent[ThreadId]->time >= SearchInput->time_limit_1
                    && !SearchRoot[ThreadId]->bad_2 )
                    {
                    SearchRoot[ThreadId]->flag = true;
                    }

                if( UseEasy
					&& SearchInput->time_is_limited
					&& SearchCurrent[ThreadId]->time >= SearchInput->time_limit_1 * EasyRatio
					&& SearchRoot[ThreadId]->easy )
                    {
                    ASSERT(!SearchRoot[ThreadId]->bad_2);
                    ASSERT(!SearchRoot[ThreadId]->change);
                    SearchRoot[ThreadId]->flag = true;
                    }

                if( UseEarly
					&& SearchInput->time_is_limited
					&& SearchCurrent[ThreadId]->time >= SearchInput->time_limit_1 * EarlyRatio
					&& !SearchRoot[ThreadId]->bad_2
					&& !SearchRoot[ThreadId]->change )
                    {
                    SearchRoot[ThreadId]->flag = true;
                    }

                if( SearchInfo[ThreadId]->can_stop
                    && (SearchInfo[ThreadId]->stop || (SearchRoot[ThreadId]->flag && !SearchInput->infinite)) )
                    {
                    search_ready = true;
                    break;
                    }
                }

            if( search_ready )
                break;
            }
        }
    else
        {
        for ( depth = 1; depth < DepthMax; depth++ )
            {
            SearchCurrent[ThreadId]->act_iteration = depth;
            SearchInfo[ThreadId]->can_stop = true;

            board_copy(SearchCurrent[ThreadId]->board, SearchInput->board);

            if( UseShortSearch && depth <= ShortSearchDepth )
                {
                search_full_root(SearchRoot[ThreadId]->list, SearchCurrent[ThreadId]->board, depth, SearchShort,
                    ThreadId);
                }
            else
                {
                search_full_root(SearchRoot[ThreadId]->list, SearchCurrent[ThreadId]->board, depth, SearchNormal,
                    ThreadId);
                }

            search_update_current(ThreadId);
            }
        }
    }

// search_update_best()

void search_update_best( int ThreadId )
    {

    int move, value, flags, depth, max_depth;
    const mv_t *pv;
    double time;
    sint64 node_nb;
    int mate, i, z;
    bool found;
    char move_string[256], pv_string[512];

    search_update_current(ThreadId);

    EnterCriticalSection(&CriticalSection);

    if( ThreadId == 0 )
        {
        move = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].move;
        value = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value;
        flags = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].flags;
        depth = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].depth;
        pv = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].pv;

        max_depth = SearchCurrent[ThreadId]->max_depth;
        time = SearchCurrent[ThreadId]->time;
        node_nb = SearchCurrent[ThreadId]->node_nb;

        move_to_string(move, move_string, 256);
        pv_to_string(pv, pv_string, 512);

        mate = value_to_mate(value);

        if( SearchCurrent[ThreadId]->multipv == 0 )
            {
            save_multipv[SearchCurrent[ThreadId]->multipv].mate = mate;
            save_multipv[SearchCurrent[ThreadId]->multipv].depth = depth;
            save_multipv[SearchCurrent[ThreadId]->multipv].max_depth = max_depth;
            save_multipv[SearchCurrent[ThreadId]->multipv].value = value;
            save_multipv[SearchCurrent[ThreadId]->multipv].time = time * 1000.0;
            save_multipv[SearchCurrent[ThreadId]->multipv].node_nb = node_nb;
            strcpy(save_multipv[SearchCurrent[ThreadId]->multipv].pv_string, pv_string);
            }
        else
            {
            found = false;

            for ( i = 0; i < SearchCurrent[ThreadId]->multipv; i++ )
                {
                if( save_multipv[i].value < value )
                    {
                    found = true;
                    break;
                    }
                }

            if( found )
                {
                for ( z = SearchCurrent[ThreadId]->multipv; z > i; z-- )
                    {
                    save_multipv[z].mate = save_multipv[z - 1].mate;
                    save_multipv[z].depth = save_multipv[z - 1].depth;
                    save_multipv[z].max_depth = save_multipv[z - 1].max_depth;
                    save_multipv[z].value = save_multipv[z - 1].value;
                    save_multipv[z].time = save_multipv[z - 1].time;
                    save_multipv[z].node_nb = save_multipv[z - 1].node_nb;
                    strcpy(save_multipv[z].pv_string, save_multipv[z - 1].pv_string);
                    }

                save_multipv[i].mate = mate;
                save_multipv[i].depth = depth;
                save_multipv[i].max_depth = max_depth;
                save_multipv[i].value = value;
                save_multipv[i].time = time * 1000.0;
                save_multipv[i].node_nb = node_nb;
                strcpy(save_multipv[i].pv_string, pv_string);
                }
            else
                {
                save_multipv[SearchCurrent[ThreadId]->multipv].mate = mate;
                save_multipv[SearchCurrent[ThreadId]->multipv].depth = depth;
                save_multipv[SearchCurrent[ThreadId]->multipv].max_depth = max_depth;
                save_multipv[SearchCurrent[ThreadId]->multipv].value = value;
                save_multipv[SearchCurrent[ThreadId]->multipv].time = time * 1000.0;
                save_multipv[SearchCurrent[ThreadId]->multipv].node_nb = node_nb;
                strcpy(save_multipv[SearchCurrent[ThreadId]->multipv].pv_string, pv_string);
                }
            }

        if( depth > 1 || (depth == 1 && SearchCurrent[ThreadId]->multipv == SearchInput->multipv) )
            {
            for ( i = 0; i <= SearchInput->multipv; i++ )
                {
                if( save_multipv[i].mate == 0 )
                    {

                    // normal evaluation

                    if( false ) { }
                    else if( flags == SearchExact )
                        {
                        send("info multipv %d depth %d seldepth %d score cp %d time %.0f nodes " S64_FORMAT " pv %s",
                            i
                                +1,
                                    save_multipv[i].depth,
                                        save_multipv[i].max_depth,
                                            save_multipv[i].value,
                                                save_multipv[i].time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    else if( flags == SearchLower )
                        {
                        send("info multipv %d depth %d seldepth %d score cp %d lowerbound time %.0f nodes "
                            S64_FORMAT " pv %s",
                                i
                                    +1,
                                        save_multipv[i].depth,
                                            save_multipv[i].max_depth,
                                                save_multipv[i].value,
                                                    save_multipv[i].
                                                        time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    else if( flags == SearchUpper )
                        {
                        send("info multipv %d depth %d seldepth %d score cp %d upperbound time %.0f nodes "
                            S64_FORMAT " pv %s",
                                i
                                    +1,
                                        save_multipv[i].depth,
                                            save_multipv[i].max_depth,
                                                save_multipv[i].value,
                                                    save_multipv[i].
                                                        time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    }
                else
                    {

                    // mate announcement

                    if( false ) { }
                    else if( flags == SearchExact )
                        {
                        send("info multipv %d depth %d seldepth %d score mate %d time %.0f nodes " S64_FORMAT " pv %s",
                            i
                                +1,
                                    save_multipv[i].depth,
                                        save_multipv[i].max_depth,
                                            save_multipv[i].mate,
                                                save_multipv[i].time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    else if( flags == SearchLower )
                        {
                        send("info multipv %d depth %d seldepth %d score mate %d lowerbound time %.0f nodes "
                            S64_FORMAT " pv %s",
                                i
                                    +1,
                                        save_multipv[i].depth,
                                            save_multipv[i].max_depth,
                                                save_multipv[i].mate,
                                                    save_multipv[i].
                                                        time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    else if( flags == SearchUpper )
                        {
                        send("info multipv %d depth %d seldepth %d score mate %d upperbound time %.0f nodes "
                            S64_FORMAT " pv %s",
                                i
                                    +1,
                                        save_multipv[i].depth,
                                            save_multipv[i].max_depth,
                                                save_multipv[i].mate,
                                                    save_multipv[i].
                                                        time,save_multipv[i].node_nb,save_multipv[i].pv_string);
                        }
                    }
                }
            }
        }

    if( UseBad && ThreadId == 0 && SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].depth > 1 )
        {
        if( SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value
            <= SearchRoot[ThreadId]->last_value - BadThreshold )
            {
            SearchRoot[ThreadId]->bad_1 = true;
            SearchRoot[ThreadId]->easy = false;
            SearchRoot[ThreadId]->flag = false;
            }
        else
            {
            SearchRoot[ThreadId]->bad_1 = false;
            }
        }
    LeaveCriticalSection(&CriticalSection);
    }

// search_update_root()

void search_update_root( int ThreadId )
    {

    int move, move_pos, move_nb;
    double time;
    sint64 node_nb;
    char move_string[256];

    if( DispRoot && ThreadId == 0 )
        {

        search_update_current(ThreadId);

        if( SearchCurrent[ThreadId]->time >= 1.0 )
            {
            move = SearchRoot[ThreadId]->move;
            move_pos = SearchRoot[ThreadId]->move_pos;
            move_nb = SearchRoot[ThreadId]->move_nb;

            time = SearchCurrent[ThreadId]->time;
            node_nb = SearchCurrent[ThreadId]->node_nb;

            move_to_string(move, move_string, 256);

            send("info currmove %s currmovenumber %d", move_string, move_pos + 1);
            }
        }
    }

// search_update_current()

void search_update_current( int ThreadId )
    {

    my_timer_t *timer;
    sint64 node_nb;
    double time, speed, cpu;

    timer = SearchCurrent[ThreadId]->timer;

    node_nb = SearchCurrent[ThreadId]->node_nb;
    time = (UseCpuTime) ? my_timer_elapsed_cpu(timer) : my_timer_elapsed_real(timer);
    speed = (time >= 1.0) ? double(node_nb) / time : 0.0;
    cpu = my_timer_cpu_usage(timer);

    SearchCurrent[ThreadId]->time = time;
    SearchCurrent[ThreadId]->speed = speed;
    SearchCurrent[ThreadId]->cpu = cpu;
    }

// search_check()

void search_check( int ThreadId )
    {
    if( ThreadId == 0 )
        {
        search_send_stat(ThreadId);

        if( UseEvent )
            event();

        if( SearchInput->depth_is_limited && SearchRoot[ThreadId]->depth > SearchInput->depth_limit )
            {
            SearchRoot[ThreadId]->flag = true;
            }

        if( SearchInput->time_is_limited && SearchCurrent[ThreadId]->time >= SearchInput->time_limit_2 )
            {
            SearchRoot[ThreadId]->flag = true;
            }

        if( SearchInput->time_is_limited && SearchCurrent[ThreadId]->time >= SearchInput->time_limit_1
            && !SearchRoot[ThreadId]->bad_1 && !SearchRoot[ThreadId]->bad_2
            && (!UseExtension || SearchRoot[ThreadId]->move_pos == 0) )
            {
            SearchRoot[ThreadId]->flag = true;
            }

        if( SearchInfo[ThreadId]->can_stop
            && (SearchInfo[ThreadId]->stop || (SearchRoot[ThreadId]->flag && !SearchInput->infinite)) )
            {
            longjmp(SearchInfo[ThreadId]->buf, 1);
            }
        }
    else
        {
        if( SearchInfo[ThreadId]->stop )
            {
            longjmp(SearchInfo[ThreadId]->buf, 1);
            }
        }
    }

// search_send_stat()

static void search_send_stat( int ThreadId )
    {

    double time, speed, cpu;
    sint64 node_nb;
    int i;

    search_update_current(ThreadId);

    if( DispStat && ThreadId == 0 && SearchCurrent[ThreadId]->time >= SearchInfo[ThreadId]->last_time + 1.0 )
        {

        SearchInfo[ThreadId]->last_time = SearchCurrent[ThreadId]->time;

        time = SearchCurrent[ThreadId]->time;
        speed = SearchCurrent[ThreadId]->speed;
        cpu = SearchCurrent[ThreadId]->cpu;
        node_nb = 0;
        speed = 0;

        for ( i = 0; i < NumberThreads; i++ )
            {
            node_nb += SearchCurrent[ThreadId]->node_nb;
            speed += SearchCurrent[ThreadId]->speed;
            }

        send("info time %.0f nodes " S64_FORMAT " nps %.0f cpuload %.0f",time*1000.0,node_nb,speed,cpu*1000.0);

        trans_stats(Trans);
        }
    }

// search_full_init()

void search_full_init( list_t *list, board_t *board, int ThreadId )
    {

    int trans_move, trans_depth, trans_flags, trans_value;

    CheckDepth[ThreadId] = 0;

    // standard sort

    list_note(list);
    list_sort(list);

    // basic sort

    trans_move = MoveNone;

    if( UseTrans )
        trans_retrieve(Trans, board->key, &trans_move, &trans_depth, &trans_flags, &trans_value);
    note_moves(list, board, 0, trans_move, ThreadId);
    list_sort(list);
    }

// search_full_root()

int search_full_root( list_t *list, board_t *board, int depth, int search_type, int ThreadId )
    {

    int value, a, b;
	
	if (UseWindow)
		{
		if( depth < 3 )
			{
			a = -ValueInf;
			b = +ValueInf;
			}
		else
			{
			if( SearchRoot[ThreadId]->last_value == ValueDraw )
				{
				a = SearchRoot[ThreadId]->last_value - 1;
				b = SearchRoot[ThreadId]->last_value + 1;
				}
			else
				{
				a = SearchRoot[ThreadId]->last_value - WindowSize;
				b = SearchRoot[ThreadId]->last_value + WindowSize;
				}
			}
		}
	else
		{
		a = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value - 30;
		b = SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value + 30;

		if( SearchInput->multipv > 0 )
			{
			a = -ValueInf;
			b = +ValueInf;
			}

		a = -ValueInf;
		b = +ValueInf;		
		}

    value = full_root(list, board, a, b, depth, 0, search_type, ThreadId);

    ASSERT(value_is_ok(value));
    ASSERT(LIST_VALUE(list, 0) == value);

    return value;
    }

// full_root()

static int full_root( list_t *list, board_t *board, int alpha, int beta, int depth, int height, int search_type,
    int ThreadId )
    {

    int old_alpha;
    int value, best_value[MultiPVMax];
    int i, move, j;
    int new_depth;
    undo_t undo[1];
    mv_t new_pv[HeightMax];
    bool found;
    bool cap_extended;
    bool reduced;

    ASSERT(list_is_ok(list));
    ASSERT(board_is_ok(board));
    ASSERT(range_is_ok(alpha, beta));
    ASSERT(depth_is_ok(depth));
    ASSERT(height_is_ok(height));
    ASSERT(search_type == SearchNormal || search_type == SearchShort);

    ASSERT(list == SearchRoot[ThreadId]->list);
    ASSERT(!LIST_IS_EMPTY(list));
    ASSERT(board == SearchCurrent[ThreadId]->board);
    ASSERT(board_is_legal(board));
    ASSERT(depth >= 1);

    // init

    SearchCurrent[ThreadId]->node_nb++;
    SearchInfo[ThreadId]->check_nb--;

    if( SearchCurrent[ThreadId]->multipv == 0 )
        for ( i = 0; i < LIST_SIZE(list); i++ )
            list->value[i] = ValueNone;

    old_alpha = alpha;
    best_value[SearchCurrent[ThreadId]->multipv] = ValueNone;

    // move loop

    for ( i = 0; i < LIST_SIZE(list); i++ )
        {

        move = LIST_MOVE(list, i);

        if( SearchCurrent[ThreadId]->multipv > 0 )
            {
            found = false;

            for ( j = 0; j < SearchCurrent[ThreadId]->multipv; j++ )
                {
                if( SearchBest[ThreadId][j].pv[0] == move )
                    {
                    found = true;
                    break;
                    }
                }

            if( found == true )
                continue;
            }

        SearchRoot[ThreadId]->depth = depth;
        SearchRoot[ThreadId]->move = move;
        SearchRoot[ThreadId]->move_pos = i;
        SearchRoot[ThreadId]->move_nb = LIST_SIZE(list);

        search_update_root(ThreadId);

        new_depth =
            full_new_depth(depth, move, board, board_is_check(board) && LIST_SIZE(list) == 1, true, height, false,
                &cap_extended, ThreadId);

        reduced = false;

        if( i > HistoryPVMoveNb && search_type == SearchNormal && depth >= 3 && new_depth < depth
            && !move_is_tactical(move, board) )
            {
            new_depth--;
            reduced = true;
            }

        move_do(board, move, undo);

        if( search_type == SearchShort || best_value[SearchCurrent[ThreadId]->multipv] == ValueNone )
            { // first move
            value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NodePV, cap_extended, ThreadId);

            if( value <= alpha )
                { // research
                value =
                    -full_search(board, -alpha, ValueInf, new_depth, height + 1, new_pv, NodePV, cap_extended,
                        ThreadId);
                old_alpha = alpha = -ValueInf;
                }
            else if( value >= beta )
                {
                value =
                    -full_search(board, -ValueInf, -beta, new_depth, height + 1, new_pv, NodePV, cap_extended,
                        ThreadId);
                beta = +ValueInf;
                }
            }
        else
            { // other moves
            value =
                -full_search(board, -alpha - 1, -alpha, new_depth, height + 1, new_pv, NodeCut, cap_extended, ThreadId);

            if( value > alpha )
                { // && value < beta
                SearchRoot[ThreadId]->change = true;
                SearchRoot[ThreadId]->easy = false;
                SearchRoot[ThreadId]->flag = false;

                if( reduced )
                    new_depth++;
                value =
                    -full_search(board, -ValueInf, -alpha, new_depth, height + 1, new_pv, NodePV, cap_extended,
                        ThreadId);

                if( value >= beta )
                    {
                    beta = +ValueInf;
                    }
                }
            }

        move_undo(board, move, undo);

        list->value[i] = value;

        if( value > best_value[SearchCurrent[ThreadId]->multipv]
            && (best_value[SearchCurrent[ThreadId]->multipv] == ValueNone || value > alpha) )
            {

            SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].move = move;
            SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].value = value;

            if( value <= alpha )
                { // upper bound
                SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].flags = SearchUpper;
                }
            else if( value >= beta )
                { // lower bound
                SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].flags = SearchLower;
                }
            else
                { // alpha < value < beta => exact value
                SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].flags = SearchExact;
                }
            SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].depth = depth;
            pv_cat(SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].pv, new_pv, move);

            search_update_best(ThreadId);
            }

        if( value > best_value[SearchCurrent[ThreadId]->multipv] )
            {
            best_value[SearchCurrent[ThreadId]->multipv] = value;

            if( value > alpha )
                {
                if( search_type == SearchNormal )
                    alpha = value;
                }
            }
        }

    ASSERT(value_is_ok(best_value));

    list_sort(list);

    ASSERT(SearchBest[ThreadId]->move == LIST_MOVE(list, 0));
    ASSERT(SearchBest[ThreadId]->value == best_value);

    if( UseTrans && best_value[SearchCurrent[ThreadId]->multipv] > old_alpha
        && best_value[SearchCurrent[ThreadId]->multipv] < beta )
        {
        pv_fill(SearchBest[ThreadId][SearchCurrent[ThreadId]->multipv].pv, board);
        }

    good_move(SearchBest[ThreadId]->move, board, depth, height, ThreadId);

    return best_value[SearchCurrent[ThreadId]->multipv];
    }

// full_search()

static int full_search( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int node_type,
    bool extended, int ThreadId )
    {

    bool in_check;
    bool single_reply;
    bool good_cap;
    int trans_move, trans_depth, trans_flags, trans_value;
    int old_alpha;
    int value, best_value;
    int move, best_move;
    int new_depth;
    int played_nb;
    int i;
    int opt_value;
    bool reduced, cap_extended;
    int probe_score;
    attack_t attack[1];
    sort_t sort[1];
    undo_t undo[1];
    mv_t new_pv[HeightMax];
    mv_t played[256];
    int FutilityMargin;

    ASSERT(board != NULL);
    ASSERT(range_is_ok(alpha, beta));
    ASSERT(depth_is_ok(depth));
    ASSERT(height_is_ok(height));
    ASSERT(pv != NULL);
    ASSERT(node_type == NodePV || node_type == NodeCut || node_type == NodeAll);
    ASSERT(board_is_legal(board));

    // horizon?

    if( depth <= 0 )
        {
        if( node_type == NodePV )
            CheckDepth[ThreadId] = -1;
        else
            CheckDepth[ThreadId] = 0;
        return full_quiescence(board, alpha, beta, 0, height, pv, ThreadId);
        }

    // init

    SearchCurrent[ThreadId]->node_nb++;
    SearchInfo[ThreadId]->check_nb--;
    PV_CLEAR(pv);

    if( height > SearchCurrent[ThreadId]->max_depth )
        SearchCurrent[ThreadId]->max_depth = height;

    if( SearchInfo[ThreadId]->check_nb <= 0 )
        {
        SearchInfo[ThreadId]->check_nb += SearchInfo[ThreadId]->check_inc;
        search_check(ThreadId);
        }

    // draw?

    if( board_is_repetition(board) )
        return ValueDraw;

    // mate-distance pruning

    if( UseDistancePruning )
        {

        // lower bound

        value = VALUE_MATE(height + 2); // does not work if the current position is mate

        if( value > alpha && board_is_mate(board) )
            value = VALUE_MATE(height);

        if( value > alpha )
            {
            alpha = value;

            if( value >= beta )
                return value;
            }

        // upper bound

        value = -VALUE_MATE(height + 1);

        if( value < beta )
            {
            beta = value;

            if( value <= alpha )
                return value;
            }
        }

    if( recog_draw(board, ThreadId) )
        return ValueDraw;

    bool can_probe = egbb_is_loaded && board->piece_nb <= 5
		&& (board->cap_sq != SquareNone || PIECE_IS_PAWN(board->moving_piece)
		|| height == (2 * SearchCurrent[ThreadId]->act_iteration) / 3);

    if( can_probe && board->piece_nb <= 4 && probe_bitbases(board, probe_score) )
        {
        probe_score = value_from_trans(probe_score, height);

        if( probe_score != 0 && board_is_mate(board) )
            return (probe_score < 0 ? 1 : -1) * VALUE_MATE(height);

        if( usable_egbb_value(probe_score, alpha, beta) )
            return probe_score;
        }

    // transposition table

    trans_move = MoveNone;

    if( UseTrans && depth >= TransDepth )
        {
        if( trans_retrieve(Trans, board->key, &trans_move, &trans_depth, &trans_flags, &trans_value) )
            {
            if( node_type != NodePV )
                {
                if( UseMateValues )
                    {
                    if( trans_depth < depth )
                        {
                        if( trans_value < -ValueEvalInf && TRANS_IS_UPPER(trans_flags) )
                            {
                            trans_depth = depth;
                            trans_flags = TransUpper;
                            }
                        else if( trans_value > +ValueEvalInf && TRANS_IS_LOWER(trans_flags) )
                            {
                            trans_depth = depth;
                            trans_flags = TransLower;
                            }
                        }

                if( trans_flags == TransEGBB && board->piece_nb == 5 && usable_egbb_value(trans_value, alpha, beta) )
                    return value_from_trans(trans_value, height);

                    if( trans_depth >= depth )
                        {
                        trans_value = value_from_trans(trans_value, height);

                        if( (UseExact && TRANS_IS_EXACT(trans_flags))
                            || (TRANS_IS_LOWER(trans_flags) && trans_value >= beta)
                                || (TRANS_IS_UPPER(trans_flags) && trans_value <= alpha) )
                            {
                            return trans_value;
                            }
                        }
                    }
                }
            }
        }

    if( can_probe && board->piece_nb == 5 && probe_bitbases(board, probe_score) )
        {
        if( probe_score > -ValueEvalInf )
            {
            trans_store(Trans, board->key, MoveNone, depth, TransEGBB, probe_score);

            if( usable_egbb_value(probe_score, alpha, beta) )
                return value_from_trans(probe_score, height);
            }
        else
            return value_from_trans(probe_score, height);
        }

    // height limit

    if( height >= HeightMax - 1 )
        return eval(board, alpha, beta, ThreadId);

    // more init

    old_alpha = alpha;
    best_value = ValueNone;
    best_move = MoveNone;
    played_nb = 0;

    attack_set(attack, board);
    in_check = ATTACK_IN_CHECK(attack);

    // null-move pruning

    if( UseNull && depth >= NullDepth && node_type != NodePV )
        {
        if( !in_check && !value_is_mate(beta) && do_null(board) )
            {

			if (UseAdaptiveNull)
				{
				if( (depth > AdaptiveNullDepth) && (board->piece_size[board->turn] >= 3) )
					NullReduction = 4;
				else
					NullReduction = 3;
				}
			else
				NullReduction = 3;

            new_depth = depth - NullReduction - 1;

            move_do_null(board, undo);
            value =
                -full_search(board, -beta, -beta + 1, new_depth, height + 1, new_pv, NODE_NEXT(node_type), false,
                    ThreadId);
            move_undo_null(board, undo);

            // verification search

            if( UseVer && depth > VerReduction )
                {
                if( value >= beta && (!UseVerEndgame || do_ver(board)) )
                    {

                    new_depth = depth - VerReduction;
                    ASSERT(new_depth > 0);

                    value =
                        full_no_null(board, alpha, beta, new_depth, height, new_pv, NodeCut, trans_move, &move, false,
                            ThreadId);

                    if( value >= beta )
                        {
                        ASSERT(move == new_pv[0]);
                        played[played_nb++] = move;
                        best_move = move;
                        best_value = value;
                        pv_copy(pv, new_pv);
                        goto cut;
                        }
                    }
                }

            // pruning

            if( value >= beta )
                {
                if( value > +ValueEvalInf )
                    value = +ValueEvalInf; // do not return unproven mates
                ASSERT(!value_is_mate(value));

                best_move = MoveNone;
                best_value = value;
                goto cut;
                }
            }
        }

    // razoring

    else if( UseRazoring && node_type != NodePV && depth <= RazorDepth
        && eval(board, alpha, beta, ThreadId) < beta - RazorMargin )
        {
        value = full_quiescence(board, alpha, beta, 0, height, pv, ThreadId);

        if( value < beta )
            return value;
        }

    // internal iterative deepening

    if( UseIID && depth >= IIDDepth && node_type == NodePV && trans_move == MoveNone )
        {

        new_depth = MIN(depth - IIDReduction, depth / 2);
        ASSERT(new_depth > 0);

        value = full_search(board, alpha, beta, new_depth, height, new_pv, node_type, false, ThreadId);

        if( value <= alpha )
            value = full_search(board, -ValueInf, beta, new_depth, height, new_pv, node_type, false, ThreadId);

        trans_move = new_pv[0];
        }

    // move generation

    sort_init(sort, board, attack, depth, height, trans_move, ThreadId);

    single_reply = false;

    if( in_check && LIST_SIZE(sort->list) == 1 )
        single_reply = true;

    // move loop

    opt_value = +ValueInf;
    good_cap = true;

    while( (move = sort_next(sort, ThreadId)) != MoveNone )
        {

        // extensions

        new_depth =
            full_new_depth(depth, move, board, single_reply, node_type == NodePV, height, extended, &cap_extended,
                ThreadId);

        // history pruning

        bool can_reduce = !in_check && new_depth < depth && !move_is_dangerous(move, board);

        value = sort->value; // history score

        if( can_reduce && depth <= 4 && node_type != NodePV && value < HistoryValue / (depth / 3 + 1) && played_nb >= 1
            + depth && !move_is_check(move, board) )
            {
            continue;
            }

        // futility pruning

        if( UseFutility )
            {
            if( node_type != NodePV && depth <= 3 )
                {
                if( !in_check && new_depth < depth && !move_is_tactical(move, board) && !move_is_check(move, board)
                    && !move_is_dangerous(move, board) )
                    {

                    ASSERT(!move_is_check(move, board));
                    // optimistic evaluation

                    if( opt_value == +ValueInf )
                        {
                        if( depth >= 2 )
                            FutilityMargin = FutilityMarginBase + (depth - 2) * FutilityMarginStep;
                        else
                            FutilityMargin = DeltaMargin;

                        opt_value = eval(board, 32768, -32768, ThreadId) + FutilityMargin; // no lazy evaluation
                        ASSERT(opt_value < +ValueInf);
                        }

                    value = opt_value;

                    // pruning

                    if( value <= alpha )
                        {
                        if( value > best_value )
                            {
                            best_value = value;
                            PV_CLEAR(pv);
                            }

                        continue;
                        }
                    }
                }
			}

        reduced = false;

        if( good_cap && !move_is_tactical(move, board) )
            {
            good_cap = false;
            }

        if( UseHistory )
            {
            if( can_reduce && played_nb >= HistoryMoveNb && depth >= HistoryDepth
                && !move_is_check(move, board) )
                {
                if( !good_cap )
                    {
                    if( node_type == NodeAll )
                        {
                        new_depth--;
                        reduced = true;
                        }
                    else
                        {
                        if( played_nb >= HistoryPVMoveNb )
                            {
                            new_depth--;
                            reduced = true;
                            }
                        }
                    }
                }
            }

        // recursive search

        move_do(board, move, undo);

        if( node_type != NodePV || best_value == ValueNone )
            { // first move
            value =
                -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_NEXT(node_type), cap_extended,
                    ThreadId);

            if( value > alpha && reduced )
                {
                new_depth++;
                value =
                    -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_NEXT(node_type),
                        cap_extended, ThreadId);
                }
            }
        else
            { // other moves
            value =
                -full_search(board, -alpha - 1, -alpha, new_depth, height + 1, new_pv, NodeCut, cap_extended, ThreadId);

            if( value > alpha )
                {
                if( reduced )
                    new_depth++;
                value =
                    -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NodePV, cap_extended, ThreadId);
                }
            }

        // history-pruning re-search

        if( HistoryReSearch && reduced && value >= beta )
            {

            ASSERT(node_type != NodePV);

            new_depth++;
            ASSERT(new_depth == depth - 1);

            value =
                -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type), cap_extended,
                    ThreadId);
            }

        move_undo(board, move, undo);

        played[played_nb++] = move;

        if( value > best_value )
            {
            best_value = value;
            pv_cat(pv, new_pv, move);

            if( value > alpha )
                {
                alpha = value;
                best_move = move;

                if( value >= beta )
                    {
                    goto cut;
                    }
                }
            }
        }

    // ALL node

    if( best_value == ValueNone )
        { // no legal move
        if( in_check )
            {
            ASSERT(board_is_mate(board));
            return VALUE_MATE(height);
            }
        else
            {
            ASSERT(board_is_stalemate(board));
            return ValueDraw;
            }
        }

    cut:

    ASSERT(value_is_ok(best_value));

    // move ordering

    if( best_move != MoveNone )
        {

        good_move(best_move, board, depth, height, ThreadId);

        if( best_value >= beta && !move_is_tactical(best_move, board) )
            {
            ASSERT(played_nb > 0 && played[played_nb - 1] == best_move);

            for ( i = 0; i < played_nb - 1; i++ )
                {
                move = played[i];
                ASSERT(move != best_move);
                history_bad(move, board, ThreadId);
                }

            history_good(best_move, board, ThreadId);
            }
        }

    // transposition table

    if( UseTrans && depth >= TransDepth )
        {

        trans_move = best_move;
        trans_depth = depth;
        trans_flags = TransUnknown;

        if( best_value > old_alpha )
            trans_flags |= TransLower;

        if( best_value < beta )
            trans_flags |= TransUpper;
        trans_value = value_to_trans(best_value, height);

        trans_store(Trans, board->key, trans_move, trans_depth, trans_flags, trans_value);
        }

    return best_value;
    }

// full_no_null()

static int full_no_null( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int node_type,
    int trans_move, int *best_move, bool extended, int ThreadId )
    {

    int value, best_value;
    int move;
    int new_depth;
    attack_t attack[1];
    sort_t sort[1];
    undo_t undo[1];
    mv_t new_pv[HeightMax];
    bool cap_extended;

    ASSERT(board != NULL);
    ASSERT(range_is_ok(alpha, beta));
    ASSERT(depth_is_ok(depth));
    ASSERT(height_is_ok(height));
    ASSERT(pv != NULL);
    ASSERT(node_type == NodePV || node_type == NodeCut || node_type == NodeAll);
    ASSERT(trans_move == MoveNone || move_is_ok(trans_move));
    ASSERT(best_move != NULL);

    ASSERT(board_is_legal(board));
    ASSERT(!board_is_check(board));
    ASSERT(depth >= 1);

    // init

    SearchCurrent[ThreadId]->node_nb++;
    SearchInfo[ThreadId]->check_nb--;
    PV_CLEAR(pv);

    if( height > SearchCurrent[ThreadId]->max_depth )
        SearchCurrent[ThreadId]->max_depth = height;

    if( SearchInfo[ThreadId]->check_nb <= 0 )
        {
        SearchInfo[ThreadId]->check_nb += SearchInfo[ThreadId]->check_inc;
        search_check(ThreadId);
        }

    attack_set(attack, board);
    ASSERT(!ATTACK_IN_CHECK(attack));

    *best_move = MoveNone;
    best_value = ValueNone;

    // move loop

    sort_init(sort, board, attack, depth, height, trans_move, ThreadId);

    while( (move = sort_next(sort, ThreadId)) != MoveNone )
        {

        new_depth = full_new_depth(depth, move, board, false, false, height, extended, &cap_extended, ThreadId);

        move_do(board, move, undo);
        value =
            -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_NEXT(node_type), cap_extended,
                ThreadId);
        move_undo(board, move, undo);

        if( value > best_value )
            {
            best_value = value;
            pv_cat(pv, new_pv, move);

            if( value > alpha )
                {
                alpha = value;
                *best_move = move;

                if( value >= beta )
                    goto cut;
                }
            }
        }

    // ALL node

    if( best_value == ValueNone )
        { // no legal move => stalemate
        ASSERT(board_is_stalemate(board));
        best_value = ValueDraw;
        }

    cut:

    ASSERT(value_is_ok(best_value));

    return best_value;
    }

// full_quiescence()

static int full_quiescence( board_t *board, int alpha, int beta, int depth, int height, mv_t pv [], int ThreadId )
    {

    bool in_check;
    int old_alpha;
    int value, best_value;
    int best_move;
    int move;
    int opt_value;
    attack_t attack[1];
    sort_t sort[1];
    undo_t undo[1];
    mv_t new_pv[HeightMax];

    ASSERT(board != NULL);
    ASSERT(range_is_ok(alpha, beta));
    ASSERT(depth_is_ok(depth));
    ASSERT(height_is_ok(height));
    ASSERT(pv != NULL);

    ASSERT(board_is_legal(board));
    ASSERT(depth <= 0);

    // init

    SearchCurrent[ThreadId]->node_nb++;
    SearchInfo[ThreadId]->check_nb--;
    PV_CLEAR(pv);

    if( height > SearchCurrent[ThreadId]->max_depth )
        SearchCurrent[ThreadId]->max_depth = height;

    if( SearchInfo[ThreadId]->check_nb <= 0 )
        {
        SearchInfo[ThreadId]->check_nb += SearchInfo[ThreadId]->check_inc;
        search_check(ThreadId);
        }

    // draw?

    if( board_is_repetition(board) )
        return ValueDraw;

    if( recog_draw(board, ThreadId) )
        return ValueDraw;

    // mate-distance pruning

    if( UseDistancePruning )
        {

        // lower bound

        value = VALUE_MATE(height + 2);

        if( value > alpha && board_is_mate(board) )
            value = VALUE_MATE(height);

        if( value > alpha )
            {
            alpha = value;

            if( value >= beta )
                return value;
            }

        // upper bound

        value = -VALUE_MATE(height + 1);

        if( value < beta )
            {
            beta = value;

            if( value <= alpha )
                return value;
            }
        }

    // more init

    attack_set(attack, board);
    in_check = ATTACK_IN_CHECK(attack);

    if( in_check )
        {
        ASSERT(depth < 0);
        depth++;
        }

    // height limit

    if( height >= HeightMax - 1 )
        return eval(board, alpha, beta, ThreadId);

    // more init

    old_alpha = alpha;
    best_value = ValueNone;
    best_move = MoveNone;

    opt_value = +ValueInf;

    if( !in_check )
        {

        // lone-king stalemate?

        if( simple_stalemate(board) )
            return ValueDraw;

        // stand pat

        value = eval(board, alpha, beta, ThreadId);

        ASSERT(value > best_value);
        best_value = value;

        if( value > alpha )
            {
            alpha = value;

            if( value >= beta )
                goto cut;
            }

        if( UseDelta )
            {
            opt_value = value + DeltaMargin;
            ASSERT(opt_value < +ValueInf);
            }
        }

    sort_init_qs(sort, board, attack, depth >= CheckDepth[ThreadId]);

    while( (move = sort_next_qs(sort)) != MoveNone )
        {

        // delta pruning

        if( UseDelta && beta == old_alpha + 1 )
            {
            if( !in_check && !move_is_check(move, board) && !capture_is_dangerous(move, board) )
                {

                ASSERT(move_is_tactical(move, board));

                value = opt_value + capture_value(move, board);

                // pruning

                if( value <= alpha )
                    {
                    if( value > best_value )
                        {
                        best_value = value;
                        PV_CLEAR(pv);
                        }

                    continue;
                    }
                }
            }

        move_do(board, move, undo);
        value = -full_quiescence(board, -beta, -alpha, depth - 1, height + 1, new_pv, ThreadId);
        move_undo(board, move, undo);

        if( value > best_value )
            {
            best_value = value;
            pv_cat(pv, new_pv, move);

            if( value > alpha )
                {
                alpha = value;
                best_move = move;

                if( value >= beta )
                    goto cut;
                }
            }
        }

    // ALL node

    if( best_value == ValueNone )
        {
        ASSERT(board_is_mate(board));
        return VALUE_MATE(height);
        }

    cut:

    ASSERT(value_is_ok(best_value));

    return best_value;
    }

// full_new_depth()

static int full_new_depth( int depth, int move, board_t *board, bool single_reply, bool in_pv, int height,
    bool extended, bool *cap_extended, int ThreadId )
    {

    ASSERT(depth_is_ok(depth));
    ASSERT(move_is_ok(move));
    ASSERT(board != NULL);
    ASSERT(single_reply == true || single_reply == false);
    ASSERT(in_pv == true || in_pv == false);
    ASSERT(depth > 0);

    *cap_extended = false;
    int to = MOVE_TO(move);
    int capture = board->square[to];
    int piece_nb = board->piece_size[White] + board->piece_size[Black];

    if( in_pv && piece_nb <= 5 && capture != Empty && !PIECE_IS_PAWN(capture) )
        return depth;

    if( single_reply && ExtendSingleReply )
        return depth;

    if( move_is_check(move, board) && (in_pv || see_move(move, board) >= -100) )
        return depth;

    if( in_pv && move_is_dangerous(move, board) )
        return depth;

    if( in_pv && capture != Empty && !extended && see_move(move, board) >= -100 )
        {
        *cap_extended = true;
        return depth;
        }
    return depth - 1;
    }

// do_null()

static bool do_null( const board_t *board )
    {

    ASSERT(board != NULL);

    // use null move if the side-to-move has at least one piece

    return board->piece_size[board->turn] >= 2; // king + one piece
    }

// do_ver()

static bool do_ver( const board_t *board )
    {

    ASSERT(board != NULL);

    // use verification if the side-to-move has at most one piece

    return board->piece_size[board->turn] <= 3; // king + one piece
    }

// pv_fill()

static void pv_fill( const mv_t pv [], board_t *board )
    {

    int move;
    int trans_move, trans_depth;
    undo_t undo[1];

    ASSERT(pv != NULL);
    ASSERT(board != NULL);

    ASSERT(UseTrans);

    move = *pv;

    if( move != MoveNone && move != MoveNull )
        {

        move_do(board, move, undo);
        pv_fill(pv + 1, board);
        move_undo(board, move, undo);

        trans_move = move;
        trans_depth = -127;

        trans_store(Trans, board->key, trans_move, trans_depth, TransUnknown, -ValueInf);
        }
    }

// move_is_dangerous()

static bool move_is_dangerous( int move, const board_t *board )
    {

    int piece;

    ASSERT(move_is_ok(move));
    ASSERT(board != NULL);

    ASSERT(!move_is_tactical(move, board));

    piece = MOVE_PIECE(move, board);

    if( PIECE_IS_PAWN(piece) && (is_passed(board, MOVE_TO(move), board->turn) || pawn_fork(board, MOVE_TO(move))) )
        return true;

    return false;
    }

// capture_is_dangerous()

static bool capture_is_dangerous( int move, const board_t *board )
    {

    int piece, capture;

    ASSERT(move_is_ok(move));
    ASSERT(board != NULL);

    ASSERT(move_is_tactical(move, board));

    piece = MOVE_PIECE(move, board);

    if( PIECE_IS_PAWN(piece) && PAWN_RANK(MOVE_TO(move), board->turn) >= Rank7 )
        {
        return true;
        }

    capture = move_capture(move, board);

    if( PIECE_IS_QUEEN(capture) )
        return true;

    if( PIECE_IS_PAWN(capture) && PAWN_RANK(MOVE_TO(move), board->turn) <= Rank2 )
        {
        return true;
        }

    return false;
    }

// simple_stalemate()

static bool simple_stalemate( const board_t *board )
    {

    int me, opp;
    int king;
    int opp_flag;
    int from, to;
    int capture;
    const inc_t *inc_ptr;
    int inc;

    ASSERT(board != NULL);

    ASSERT(board_is_legal(board));
    ASSERT(!board_is_check(board));

    // lone king?

    me = board->turn;

    if( board->piece_size[me] != 1 || board->pawn_size[me] != 0 )
        return false; // no

    // king in a corner?

    king = KING_POS(board, me);

    if( king != A1 && king != H1 && king != A8 && king != H8 )
        return false; // no

    // init

    opp = COLOUR_OPP(me);
    opp_flag = COLOUR_FLAG(opp);

    // king can move?

    from = king;

    for ( inc_ptr = KingInc; (inc = *inc_ptr) != IncNone; inc_ptr++ )
        {
        to = from + inc;
        capture = board->square[to];

        if( capture == Empty || FLAG_IS(capture, opp_flag) )
            {
            if( !is_attacked(board, to, opp) )
                return false; // legal king move
            }
        }

    // no legal move

    ASSERT(board_is_stalemate((board_t *)board));

    return true;
    }

// capture_value()

int capture_value( int move, const board_t *board )
    {

    int value;
    int to = MOVE_TO(move);
    int capture = board->square[to];
    int piece = MOVE_PIECE(move, board);
    int me = board->turn;
    int opp = COLOUR_OPP(me);
    value = 0;

    if( MOVE_IS_EN_PASSANT(move) )
        {

        value += ValuePawn;

        if( is_passed(board, SQUARE_EP_DUAL(to), COLOUR_OPP(board->turn)) )
            value += 50;
        }
    else
        {
        if( MOVE_IS_PROMOTE(move) )
            {
            if( !PIECE_IS_QUEEN(move_promote(move)) )
                return 0;
            value += ValueQueen - ValuePawn;
            }

        if( PIECE_IS_PAWN(capture) && is_passed(board, to, COLOUR_OPP(board->turn)) )
            value += 50;

        value += VALUE_PIECE(capture);
        }

    if( PIECE_IS_PAWN(piece) && !MOVE_IS_PROMOTE(move) && is_passed(board, to, board->turn) )
        value += 50;
    return value;
    }

// end of search.cpp
