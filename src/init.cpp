// init.cpp

// includes
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "eval.h"
#include "init.h"
#include "material.h"
#include "option.h"
#include "pawn.h"
#include "search.h"
#include "sort.h"

// lazy eval

bool LazyEval = true;
int LazyEvalMargin = 200;

// tempo
bool UseTempo = true;
int TempoOpening = 20;
int TempoEndgame = 10;

// mobility
int PieceActivityWeightOpening = 256;
int PieceActivityWeightEndgame = 256;
int KnightUnit = 4;
int BishopUnit = 6;
int RookUnit = 7;
int QueenUnit = 13;
int MobMove = 1;
int MobAttack = 1;
int MobDefense = 0;
int KnightMobOpening = 4;
int KnightMobEndgame = 4;
int BishopMobOpening = 5;
int BishopMobEndgame = 5;
int RookMobOpening = 2;
int RookMobEndgame = 4;
int QueenMobOpening = 2;
int QueenMobEndgame = 4;
int KingMobOpening = 0;
int KingMobEndgame = 0;

// pawn
int PawnStructureWeightOpening = 256;
int PawnStructureWeightEndgame = 256;
int DoubledOpening = 10;
int DoubledEndgame = 20;
int IsolatedOpening = 10;
int IsolatedOpeningOpen = 20;
int IsolatedEndgame = 20;
int BackwardOpening = 8;
int BackwardOpeningOpen = 16;
int BackwardEndgame = 10;

// passed pawn
int PassedPawnWeightOpening = 256;
int PassedPawnWeightEndgame = 256;
int CandidateOpeningMin = 5;
int CandidateOpeningMax = 55;
int CandidateEndgameMin = 10;
int CandidateEndgameMax = 110;
int PassedOpeningMin = 10;
int PassedOpeningMax = 70;
int PassedEndgameMin = 20;
int PassedEndgameMax = 140;
int UnstoppablePasser = 800;
int FreePasser = 60;

// bishop
bool UseBishopOutpost = true;
int TrappedBishop = 100;
int BlockedBishop = 50;

// rook
bool UseOpenFile = true;
int RookSemiOpenFileOpening = 10;
int RookSemiOpenFileEndgame = 10;
int RookOpenFileOpening = 20;
int RookOpenFileEndgame = 20;
int RookSemiKingFileOpening = 10;
int RookKingFileOpening = 20;
int RookKingFileEndgame = 10;
int Rook7thOpening = 20;
int Rook7thEndgame = 40;
int BlockedRook = 50;

// queen
int Queen7thOpening = 10;
int Queen7thEndgame = 20;

// king
int KingSafetyWeightOpening = 256;
int KingSafetyWeightEndgame = 256;
bool KingSafety = false;
int KingSafetyMargin = 1600;
int AttackerDistance = 5;
int DefenderDistance = 20;
bool UseKingAttack = true;
int KingAttackOpening = 20;
int KingAttackOpening_1 = 20;
bool UseShelter = true;
int ShelterOpening = 256;
bool UseStorm = true;
int StormOpening = 10;

// material
int MaterialWeightOpening = 256;
int MaterialWeightEndgame = 256;
int PawnOpening = 80;
int PawnEndgame = 90;
int KnightOpening = 325;
int KnightEndgame = 325;
int BishopOpening = 325;
int BishopEndgame = 325;
int RookOpening = 500;
int RookEndgame = 500;
int QueenOpening = 975;
int QueenEndgame = 975;
int BishopPairOpening = 50;
int BishopPairEndgame = 50;
int OpeningExchangePenalty = 30;
int EndgameExchangePenalty = 30;
bool UseMaterialImbalance = false;
int MaterialImbalanceWeightOpening = 256;
int MaterialImbalanceWeightEndgame = 256;

// pst
int PieceSquareWeightOpening = 256;
int PieceSquareWeightEndgame = 256;
int PawnFileOpening = 5;
int KnightCentreOpening = 5;
int KnightCentreEndgame = 5;
int KnightRankOpening = 5;
int KnightBackRankOpening = 0;
int KnightTrapped = 100;
int BishopCentreOpening = 2;
int BishopCentreEndgame = 3;
int BishopBackRankOpening = 10;
int BishopDiagonalOpening = 4;
int RookFileOpening = 3;
int QueenCentreOpening = 0;
int QueenCentreEndgame = 4;
int QueenBackRankOpening = 5;
int KingCentreEndgame = 12;
int KingFileOpening = 10;
int KingRankOpening = 10;

// search
bool UseCpuTime = false;
bool UseEvent = true;
bool UseShortSearch = true;
int ShortSearchDepth = 1;
bool UseEasy = true;
int EasyThreshold = 150;
bool UseEarly = true;
bool UseBad = true;
int BadThreshold = 50;
bool UseExtension = true;
bool UseWindow = true;
int WindowSize = 20;
bool UseDistancePruning = true;
bool ExtendSingleReply = true;
bool UseMateValues = true;

// null move
bool UseNull = true;
int NullDepth = 2;
int NullReduction = 3;
bool UseAdaptiveNull = true;
int AdaptiveNullDepth = 7;
bool UseVer = true;
int VerReduction = 5;
bool UseVerEndgame = false;

// razoring
bool UseRazoring = false;
int RazorDepth = 3;
int RazorMargin = 300;

// internal iterative deepening
bool UseIID = true;
int IIDDepth = 3;
int IIDReduction = 2;

// history pruning
bool UseHistory = true;
int HistoryDepth = 3;
int HistoryMoveNb = 3;
int HistoryPVMoveNb = 10;
int HistoryValue = 9830;
bool HistoryReSearch = false;

// futility pruning
bool UseFutility = true;
int FutilityMarginBase = 300;
int FutilityMarginStep = 50;

// quiescence search
bool UseDelta = true;
int DeltaMargin = 50;

// trans
bool UseModulo = false;
bool AlwaysWrite = true;
bool SmartMove = true;
bool SmartValue = false;
bool SmartReplace = true;

// parse_option
static void parse_option( const char *str )
    {
    char arg[4][256];
    arg[0][0] = arg[1][0] = '\0';
    sscanf(str, "%s %s", arg[0], arg[1]);

    if( !strcmp(arg[0], "//") ) { }

    // lazy eval ----------------
    if( !strcmp(arg[0], "LazyEval") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            LazyEval = true;
            printf("LazyEval: %s\n", "true");
            }
        else
            {
            LazyEval = false;
            printf("LazyEval: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "LazyEvalMargin") )
        {
        LazyEvalMargin = atoi(arg[1]);
        printf("LazyEvalMargin: %d\n", LazyEvalMargin);
        }

    // tempo ----------------
    if( !strcmp(arg[0], "UseTempo") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseTempo = true;
            printf("UseTempo: %s\n", "true");
            }
        else
            {
            UseTempo = false;
            printf("UseTempo: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "TempoOpening") )
        {
        TempoOpening = atoi(arg[1]);
        printf("TempoOpening: %d\n", TempoOpening);
        }

    if( !strcmp(arg[0], "TempoEndgame") )
        {
        TempoEndgame = atoi(arg[1]);
        printf("TempoEndgame: %d\n", TempoEndgame);
        }

    // mobility ----------------
    if( !strcmp(arg[0], "PieceActivityWeightOpening") )
        {
        PieceActivityWeightOpening = atoi(arg[1]);
        printf("PieceActivityWeightOpening: %d\n", PieceActivityWeightOpening);
        }

    if( !strcmp(arg[0], "PieceActivityWeightEndgame") )
        {
        PieceActivityWeightEndgame = atoi(arg[1]);
        printf("PieceActivityWeightEndgame: %d\n", PieceActivityWeightEndgame);
        }

    if( !strcmp(arg[0], "KnightUnit") )
        {
        KnightUnit = atoi(arg[1]);
        printf("KnightUnit: %d\n", KnightUnit);
        }

    if( !strcmp(arg[0], "BishopUnit") )
        {
        BishopUnit = atoi(arg[1]);
        printf("BishopUnit: %d\n", BishopUnit);
        }

    if( !strcmp(arg[0], "RookUnit") )
        {
        RookUnit = atoi(arg[1]);
        printf("RookUnit: %d\n", RookUnit);
        }

    if( !strcmp(arg[0], "QueenUnit") )
        {
        QueenUnit = atoi(arg[1]);
        printf("QueenUnit: %d\n", QueenUnit);
        }

    if( !strcmp(arg[0], "MobMove") )
        {
        MobMove = atoi(arg[1]);
        printf("MobMove: %d\n", MobMove);
        }

    if( !strcmp(arg[0], "MobAttack") )
        {
        MobAttack = atoi(arg[1]);
        printf("MobAttack: %d\n", MobAttack);
        }

    if( !strcmp(arg[0], "MobDefense") )
        {
        MobDefense = atoi(arg[1]);
        printf("MobDefense: %d\n", MobDefense);
        }

    if( !strcmp(arg[0], "KnightMobOpening") )
        {
        KnightMobOpening = atoi(arg[1]);
        printf("KnightMobOpening: %d\n", KnightMobOpening);
        }

    if( !strcmp(arg[0], "KnightMobEndgame") )
        {
        KnightMobEndgame = atoi(arg[1]);
        printf("KnightMobEndgame: %d\n", KnightMobEndgame);
        }

    if( !strcmp(arg[0], "BishopMobOpening") )
        {
        BishopMobOpening = atoi(arg[1]);
        printf("BishopMobOpening: %d\n", BishopMobOpening);
        }

    if( !strcmp(arg[0], "BishopMobEndgame") )
        {
        BishopMobEndgame = atoi(arg[1]);
        printf("BishopMobEndgame: %d\n", BishopMobEndgame);
        }

    if( !strcmp(arg[0], "RookMobOpening") )
        {
        RookMobOpening = atoi(arg[1]);
        printf("RookMobOpening: %d\n", RookMobOpening);
        }

    if( !strcmp(arg[0], "RookMobEndgame") )
        {
        RookMobEndgame = atoi(arg[1]);
        printf("RookMobEndgame: %d\n", RookMobEndgame);
        }

    if( !strcmp(arg[0], "QueenMobOpening") )
        {
        QueenMobOpening = atoi(arg[1]);
        printf("QueenMobOpening: %d\n", QueenMobOpening);
        }

    if( !strcmp(arg[0], "QueenMobEndgame") )
        {
        QueenMobEndgame = atoi(arg[1]);
        printf("QueenMobEndgame: %d\n", QueenMobEndgame);
        }

    if( !strcmp(arg[0], "KingMobOpening") )
        {
        KingMobOpening = atoi(arg[1]);
        printf("KingMobOpening: %d\n", KingMobOpening);
        }

    if( !strcmp(arg[0], "KingMobEndgame") )
        {
        KingMobEndgame = atoi(arg[1]);
        printf("KingMobEndgame: %d\n", KingMobEndgame);
        }

    // pawn ----------------
    if( !strcmp(arg[0], "PawnStructureWeightOpening") )
        {
        PawnStructureWeightOpening = atoi(arg[1]);
        printf("PawnStructureWeightOpening: %d\n", PawnStructureWeightOpening);
        }

    if( !strcmp(arg[0], "PawnStructureWeightEndgame") )
        {
        PawnStructureWeightEndgame = atoi(arg[1]);
        printf("PawnStructureWeightEndgame: %d\n", PawnStructureWeightEndgame);
        }

    if( !strcmp(arg[0], "DoubledOpening") )
        {
        DoubledOpening = atoi(arg[1]);
        printf("DoubledOpening: %d\n", DoubledOpening);
        }

    if( !strcmp(arg[0], "DoubledEndgame") )
        {
        DoubledEndgame = atoi(arg[1]);
        printf("DoubledEndgame: %d\n", DoubledEndgame);
        }

    if( !strcmp(arg[0], "IsolatedOpening") )
        {
        IsolatedOpening = atoi(arg[1]);
        printf("IsolatedOpening: %d\n", IsolatedOpening);
        }

    if( !strcmp(arg[0], "IsolatedOpeningOpen") )
        {
        IsolatedOpeningOpen = atoi(arg[1]);
        printf("IsolatedOpeningOpen: %d\n", IsolatedOpeningOpen);
        }

    if( !strcmp(arg[0], "IsolatedEndgame") )
        {
        IsolatedEndgame = atoi(arg[1]);
        printf("IsolatedEndgame: %d\n", IsolatedEndgame);
        }

    if( !strcmp(arg[0], "BackwardOpening") )
        {
        BackwardOpening = atoi(arg[1]);
        printf("BackwardOpening: %d\n", BackwardOpening);
        }

    if( !strcmp(arg[0], "BackwardOpeningOpen") )
        {
        BackwardOpeningOpen = atoi(arg[1]);
        printf("BackwardOpeningOpen: %d\n", BackwardOpeningOpen);
        }

    if( !strcmp(arg[0], "BackwardEndgame") )
        {
        BackwardEndgame = atoi(arg[1]);
        printf("BackwardEndgame: %d\n", BackwardEndgame);
        }

    // passed pawn ----------------
    if( !strcmp(arg[0], "PassedPawnWeightOpening") )
        {
        PassedPawnWeightOpening = atoi(arg[1]);
        printf("PassedPawnWeightOpening: %d\n", PassedPawnWeightOpening);
        }

    if( !strcmp(arg[0], "PassedPawnWeightEndgame") )
        {
        PassedPawnWeightEndgame = atoi(arg[1]);
        printf("PassedPawnWeightEndgame: %d\n", PassedPawnWeightEndgame);
        }

    if( !strcmp(arg[0], "CandidateOpeningMin") )
        {
        CandidateOpeningMin = atoi(arg[1]);
        printf("CandidateOpeningMin: %d\n", CandidateOpeningMin);
        }

    if( !strcmp(arg[0], "CandidateOpeningMax") )
        {
        CandidateOpeningMax = atoi(arg[1]);
        printf("CandidateOpeningMax: %d\n", CandidateOpeningMax);
        }

    if( !strcmp(arg[0], "CandidateEndgameMin") )
        {
        CandidateEndgameMin = atoi(arg[1]);
        printf("CandidateEndgameMin: %d\n", CandidateEndgameMin);
        }

    if( !strcmp(arg[0], "CandidateEndgameMax") )
        {
        CandidateEndgameMax = atoi(arg[1]);
        printf("CandidateEndgameMax: %d\n", CandidateEndgameMax);
        }

    if( !strcmp(arg[0], "PassedOpeningMin") )
        {
        PassedOpeningMin = atoi(arg[1]);
        printf("PassedOpeningMin: %d\n", PassedOpeningMin);
        }

    if( !strcmp(arg[0], "PassedOpeningMax") )
        {
        PassedOpeningMax = atoi(arg[1]);
        printf("PassedOpeningMax: %d\n", PassedOpeningMax);
        }

    if( !strcmp(arg[0], "PassedEndgameMin") )
        {
        PassedEndgameMin = atoi(arg[1]);
        printf("PassedEndgameMin: %d\n", PassedEndgameMin);
        }

    if( !strcmp(arg[0], "PassedEndgameMax") )
        {
        PassedEndgameMax = atoi(arg[1]);
        printf("PassedEndgameMax: %d\n", PassedEndgameMax);
        }

    if( !strcmp(arg[0], "UnstoppablePasser") )
        {
        UnstoppablePasser = atoi(arg[1]);
        printf("UnstoppablePasser: %d\n", UnstoppablePasser);
        }

    if( !strcmp(arg[0], "FreePasser") )
        {
        FreePasser = atoi(arg[1]);
        printf("FreePasser: %d\n", FreePasser);
        }

    // bishop ----------------
    if( !strcmp(arg[0], "UseBishopOutpost") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseBishopOutpost = true;
            printf("UseBishopOutpost: %s\n", "true");
            }
        else
            {
            UseBishopOutpost = false;
            printf("UseBishopOutpost: %s\n", "false");
            }
        }
    if( !strcmp(arg[0], "TrappedBishop") )
        {
        TrappedBishop = atoi(arg[1]);
        printf("TrappedBishop: %d\n", TrappedBishop);
        }

    if( !strcmp(arg[0], "BlockedBishop") )
        {
        BlockedBishop = atoi(arg[1]);
        printf("BlockedBishop: %d\n", BlockedBishop);
        }

    // rook ----------------
    if( !strcmp(arg[0], "UseOpenFile") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseOpenFile = true;
            printf("UseOpenFile: %s\n", "true");
            }
        else
            {
            UseOpenFile = false;
            printf("UseOpenFile: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "RookSemiOpenFileOpening") )
        {
        RookSemiOpenFileOpening = atoi(arg[1]);
        printf("RookSemiOpenFileOpening: %d\n", RookSemiOpenFileOpening);
        }

    if( !strcmp(arg[0], "RookSemiOpenFileEndgame") )
        {
        RookSemiOpenFileEndgame = atoi(arg[1]);
        printf("RookSemiOpenFileEndgame: %d\n", RookSemiOpenFileEndgame);
        }

    if( !strcmp(arg[0], "RookOpenFileOpening") )
        {
        RookOpenFileOpening = atoi(arg[1]);
        printf("RookOpenFileOpening: %d\n", RookOpenFileOpening);
        }

    if( !strcmp(arg[0], "RookOpenFileEndgame") )
        {
        RookOpenFileEndgame = atoi(arg[1]);
        printf("RookOpenFileEndgame: %d\n", RookOpenFileEndgame);
        }

    if( !strcmp(arg[0], "RookSemiKingFileOpening") )
        {
        RookSemiKingFileOpening = atoi(arg[1]);
        printf("RookSemiKingFileOpening: %d\n", RookSemiKingFileOpening);
        }

    if( !strcmp(arg[0], "RookKingFileOpening") )
        {
        RookKingFileOpening = atoi(arg[1]);
        printf("RookKingFileOpening: %d\n", RookKingFileOpening);
        }

    if( !strcmp(arg[0], "RookKingFileEndgame") )
        {
        RookKingFileOpening = atoi(arg[1]);
        printf("RookKingFileEndgame: %d\n", RookKingFileEndgame);
        }

    if( !strcmp(arg[0], "Rook7thOpening") )
        {
        Rook7thOpening = atoi(arg[1]);
        printf("Rook7thOpening: %d\n", Rook7thOpening);
        }

    if( !strcmp(arg[0], "Rook7thEndgame") )
        {
        Rook7thEndgame = atoi(arg[1]);
        printf("Rook7thEndgame: %d\n", Rook7thEndgame);
        }

    if( !strcmp(arg[0], "BlockedRook") )
        {
        BlockedRook = atoi(arg[1]);
        printf("BlockedRook: %d\n", BlockedRook);
        }

    // queen ----------------
    if( !strcmp(arg[0], "Queen7thOpening") )
        {
        Queen7thOpening = atoi(arg[1]);
        printf("Queen7thOpening: %d\n", Queen7thOpening);
        }

    if( !strcmp(arg[0], "Queen7thEndgame") )
        {
        Queen7thEndgame = atoi(arg[1]);
        printf("Queen7thEndgame: %d\n", Queen7thEndgame);
        }

    // king ----------------
    if( !strcmp(arg[0], "KingSafety") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            KingSafety = true;
            printf("KingSafety: %s\n", "true");
            }
        else
            {
            KingSafety = false;
            printf("KingSafety: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "KingSafetyWeightOpening") )
        {
        KingSafetyWeightOpening = atoi(arg[1]);
        printf("KingSafetyWeightOpening: %d\n", KingSafetyWeightOpening);
        }

    if( !strcmp(arg[0], "KingSafetyWeightEndgame") )
        {
        KingSafetyWeightEndgame = atoi(arg[1]);
        printf("KingSafetyWeightEndgame: %d\n", KingSafetyWeightEndgame);
        }

    if( !strcmp(arg[0], "KingSafetyMargin") )
        {
        KingSafetyMargin = atoi(arg[1]);
        printf("KingSafetyMargin: %d\n", KingSafetyMargin);
        }

    if( !strcmp(arg[0], "AttackerDistance") )
        {
        AttackerDistance = atoi(arg[1]);
        printf("AttackerDistance: %d\n", AttackerDistance);
        }

    if( !strcmp(arg[0], "DefenderDistance") )
        {
        DefenderDistance = atoi(arg[1]);
        printf("DefenderDistance: %d\n", DefenderDistance);
        }

    if( !strcmp(arg[0], "UseKingAttack") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseKingAttack = true;
            printf("UseKingAttack: %s\n", "true");
            }
        else
            {
            UseKingAttack = false;
            printf("UseKingAttack: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "KingAttackOpening") )
        {
        KingAttackOpening = atoi(arg[1]);
        printf("KingAttackOpening: %d\n", KingAttackOpening);
        }

    if( !strcmp(arg[0], "KingAttackOpening_1") )
        {
        KingAttackOpening_1 = atoi(arg[1]);
        printf("KingAttackOpening_1: %d\n", KingAttackOpening_1);
        }

    if( !strcmp(arg[0], "UseShelter") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseShelter = true;
            printf("UseShelter: %s\n", "true");
            }
        else
            {
            UseShelter = false;
            printf("UseShelter: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "ShelterOpening") )
        {
        ShelterOpening = atoi(arg[1]);
        printf("ShelterOpening: %d\n", ShelterOpening);
        }

    if( !strcmp(arg[0], "UseStorm") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseStorm = true;
            printf("UseStorm: %s\n", "true");
            }
        else
            {
            UseStorm = false;
            printf("UseStorm: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "StormOpening") )
        {
        StormOpening = atoi(arg[1]);
        printf("StormOpening: %d\n", StormOpening);
        }

    // material ----------------
    if( !strcmp(arg[0], "MaterialWeightOpening") )
        {
        MaterialWeightOpening = atoi(arg[1]);
        printf("MaterialWeightOpening: %d\n", MaterialWeightOpening);
        }

    if( !strcmp(arg[0], "MaterialWeightEndgame") )
        {
        MaterialWeightEndgame = atoi(arg[1]);
        printf("MaterialWeightEndgame: %d\n", MaterialWeightEndgame);
        }

    if( !strcmp(arg[0], "PawnOpening") )
        {
        PawnOpening = atoi(arg[1]);
        printf("PawnOpening: %d\n", PawnOpening);
        }

    if( !strcmp(arg[0], "PawnEndgame") )
        {
        PawnEndgame = atoi(arg[1]);
        printf("PawnEndgame: %d\n", PawnEndgame);
        }

    if( !strcmp(arg[0], "KnightOpening") )
        {
        KnightOpening = atoi(arg[1]);
        printf("KnightOpening: %d\n", KnightOpening);
        }

    if( !strcmp(arg[0], "KnightEndgame") )
        {
        KnightEndgame = atoi(arg[1]);
        printf("KnightEndgame: %d\n", KnightEndgame);
        }

    if( !strcmp(arg[0], "BishopOpening") )
        {
        BishopOpening = atoi(arg[1]);
        printf("BishopOpening: %d\n", BishopOpening);
        }

    if( !strcmp(arg[0], "BishopEndgame") )
        {
        BishopEndgame = atoi(arg[1]);
        printf("BishopEndgame: %d\n", BishopEndgame);
        }

    if( !strcmp(arg[0], "RookOpening") )
        {
        RookOpening = atoi(arg[1]);
        printf("RookOpening: %d\n", RookOpening);
        }

    if( !strcmp(arg[0], "RookEndgame") )
        {
        RookEndgame = atoi(arg[1]);
        printf("RookEndgame: %d\n", RookEndgame);
        }

    if( !strcmp(arg[0], "QueenOpening") )
        {
        QueenOpening = atoi(arg[1]);
        printf("QueenOpening: %d\n", QueenOpening);
        }

    if( !strcmp(arg[0], "QueenEndgame") )
        {
        QueenEndgame = atoi(arg[1]);
        printf("QueenEndgame: %d\n", QueenEndgame);
        }

    if( !strcmp(arg[0], "BishopPairOpening") )
        {
        BishopPairOpening = atoi(arg[1]);
        printf("BishopPairOpening: %d\n", BishopPairOpening);
        }

    if( !strcmp(arg[0], "BishopPairEndgame") )
        {
        BishopPairEndgame = atoi(arg[1]);
        printf("BishopPairEndgame: %d\n", BishopPairEndgame);
        }

    if( !strcmp(arg[0], "OpeningExchangePenalty") )
        {
        OpeningExchangePenalty = atoi(arg[1]);
        printf("OpeningExchangePenalty: %d\n", OpeningExchangePenalty);
        }

    if( !strcmp(arg[0], "EndgameExchangePenalty") )
        {
        EndgameExchangePenalty = atoi(arg[1]);
        printf("EndgameExchangePenalty: %d\n", EndgameExchangePenalty);
        }

    if( !strcmp(arg[0], "UseMaterialImbalance") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseMaterialImbalance = true;
            printf("UseMaterialImbalance: %s\n", "true");
            }
        else
            {
            UseMaterialImbalance = false;
            printf("UseMaterialImbalance: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "MaterialImbalanceWeightOpening") )
        {
        MaterialImbalanceWeightOpening = atoi(arg[1]);
        printf("MaterialImbalanceWeightOpening: %d\n", MaterialImbalanceWeightOpening);
        }

    if( !strcmp(arg[0], "MaterialImbalanceWeightEndgame") )
        {
        MaterialImbalanceWeightEndgame = atoi(arg[1]);
        printf("MaterialImbalanceWeightEndgame: %d\n", MaterialImbalanceWeightEndgame);
        }

    // pst ----------------
    if( !strcmp(arg[0], "PieceSquareWeightOpening") )
        {
        PieceSquareWeightOpening = atoi(arg[1]);
        printf("PieceSquareWeightOpening: %d\n", PieceSquareWeightOpening);
        }

    if( !strcmp(arg[0], "PieceSquareWeightEndgame") )
        {
        PieceSquareWeightEndgame = atoi(arg[1]);
        printf("PieceSquareWeightEndgame: %d\n", PieceSquareWeightEndgame);
        }

    if( !strcmp(arg[0], "PawnFileOpening") )
        {
        PawnFileOpening = atoi(arg[1]);
        printf("PawnFileOpening: %d\n", PawnFileOpening);
        }

    if( !strcmp(arg[0], "KnightCentreOpening") )
        {
        KnightCentreOpening = atoi(arg[1]);
        printf("KnightCentreOpening: %d\n", KnightCentreOpening);
        }

    if( !strcmp(arg[0], "KnightCentreEndgame") )
        {
        KnightCentreEndgame = atoi(arg[1]);
        printf("KnightCentreEndgame: %d\n", KnightCentreEndgame);
        }

    if( !strcmp(arg[0], "KnightRankOpening") )
        {
        KnightRankOpening = atoi(arg[1]);
        printf("KnightRankOpening: %d\n", KnightRankOpening);
        }

    if( !strcmp(arg[0], "KnightBackRankOpening") )
        {
        KnightBackRankOpening = atoi(arg[1]);
        printf("KnightBackRankOpening: %d\n", KnightBackRankOpening);
        }

    if( !strcmp(arg[0], "KnightTrapped") )
        {
        KnightTrapped = atoi(arg[1]);
        printf("KnightTrapped: %d\n", KnightTrapped);
        }

    if( !strcmp(arg[0], "BishopCentreOpening") )
        {
        BishopCentreOpening = atoi(arg[1]);
        printf("BishopCentreOpening: %d\n", BishopCentreOpening);
        }

    if( !strcmp(arg[0], "BishopCentreEndgame") )
        {
        BishopCentreEndgame = atoi(arg[1]);
        printf("BishopCentreEndgame: %d\n", BishopCentreEndgame);
        }

    if( !strcmp(arg[0], "BishopBackRankOpening") )
        {
        BishopBackRankOpening = atoi(arg[1]);
        printf("BishopBackRankOpening: %d\n", BishopBackRankOpening);
        }

    if( !strcmp(arg[0], "BishopDiagonalOpening") )
        {
        BishopDiagonalOpening = atoi(arg[1]);
        printf("BishopDiagonalOpening: %d\n", BishopDiagonalOpening);
        }

    if( !strcmp(arg[0], "RookFileOpening") )
        {
        RookFileOpening = atoi(arg[1]);
        printf("RookFileOpening: %d\n", RookFileOpening);
        }

    if( !strcmp(arg[0], "QueenCentreOpening") )
        {
        QueenCentreOpening = atoi(arg[1]);
        printf("QueenCentreOpening: %d\n", QueenCentreOpening);
        }

    if( !strcmp(arg[0], "QueenCentreEndgame") )
        {
        QueenCentreEndgame = atoi(arg[1]);
        printf("QueenCentreEndgame: %d\n", QueenCentreEndgame);
        }

    if( !strcmp(arg[0], "QueenBackRankOpening") )
        {
        QueenBackRankOpening = atoi(arg[1]);
        printf("QueenBackRankOpening: %d\n", QueenBackRankOpening);
        }

    if( !strcmp(arg[0], "KingCentreEndgame") )
        {
        KingCentreEndgame = atoi(arg[1]);
        printf("KingCentreEndgame: %d\n", KingCentreEndgame);
        }

    if( !strcmp(arg[0], "KingFileOpening") )
        {
        KingFileOpening = atoi(arg[1]);
        printf("KingFileOpening: %d\n", KingFileOpening);
        }

    if( !strcmp(arg[0], "KingRankOpening") )
        {
        KingRankOpening = atoi(arg[1]);
        printf("KingRankOpening: %d\n", KingRankOpening);
        }

    // search ----------------
    if( !strcmp(arg[0], "UseCpuTime") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseCpuTime = true;
            printf("UseCpuTime: %s\n", "true");
            }
        else
            {
            UseCpuTime = false;
            printf("UseCpuTime: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseEvent") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseEvent = true;
            printf("UseEvent: %s\n", "true");
            }
        else
            {
            UseEvent = false;
            printf("UseEvent: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseShortSearch") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseShortSearch = true;
            printf("UseShortSearch: %s\n", "true");
            }
        else
            {
            UseShortSearch = false;
            printf("UseShortSearch: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "ShortSearchDepth") )
        {
        ShortSearchDepth = atoi(arg[1]);
        printf("ShortSearchDepth: %d\n", ShortSearchDepth);
        }

    if( !strcmp(arg[0], "UseEarly") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseEarly = true;
            printf("UseEarly: %s\n", "true");
            }
        else
            {
            UseEarly = false;
            printf("UseEarly: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseEasy") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseEasy = true;
            printf("UseEasy: %s\n", "true");
            }
        else
            {
            UseEasy = false;
            printf("UseEasy: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "EasyThreshold") )
        {
        EasyThreshold = atoi(arg[1]);
        printf("EasyThreshold: %d\n", EasyThreshold);
        }

    if( !strcmp(arg[0], "UseBad") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseBad = true;
            printf("UseBad: %s\n", "true");
            }
        else
            {
            UseBad = false;
            printf("UseBad: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "BadThreshold") )
        {
        BadThreshold = atoi(arg[1]);
        printf("BadThreshold: %d\n", BadThreshold);
        }

    if( !strcmp(arg[0], "UseExtension") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseExtension = true;
            printf("UseExtension: %s\n", "true");
            }
        else
            {
            UseExtension = false;
            printf("UseExtension: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseWindow") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseWindow = true;
            printf("UseWindow: %s\n", "true");
            }
        else
            {
            UseWindow = false;
            printf("UseWindow: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "WindowSize") )
        {
        WindowSize = atoi(arg[1]);
        printf("WindowSize: %d\n", WindowSize);
        }

    if( !strcmp(arg[0], "UseDistancePruning") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseDistancePruning = true;
            printf("UseDistancePruning: %s\n", "true");
            }
        else
            {
            UseDistancePruning = false;
            printf("UseDistancePruning: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "ExtendSingleReply") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            ExtendSingleReply = true;
            printf("ExtendSingleReply: %s\n", "true");
            }
        else
            {
            ExtendSingleReply = false;
            printf("ExtendSingleReply: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseMateValues") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseMateValues = true;
            printf("UseMateValues: %s\n", "true");
            }
        else
            {
            UseMateValues = false;
            printf("UseMateValues: %s\n", "false");
            }
        }

    // null move ----------------
    if( !strcmp(arg[0], "UseNull") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseNull = true;
            printf("UseNull: %s\n", "true");
            }
        else
            {
            UseNull = false;
            printf("UseNull: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "NullDepth") )
        {
        NullDepth = atoi(arg[1]);
        printf("NullDepth: %d\n", NullDepth);
        }

    if( !strcmp(arg[0], "NullReduction") )
        {
        NullReduction = atoi(arg[1]);
        printf("NullReduction: %d\n", NullReduction);
        }

    if( !strcmp(arg[0], "UseAdaptiveNull") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseAdaptiveNull = true;
            printf("UseAdaptiveNull: %s\n", "true");
            }
        else
            {
            UseNull = false;
            printf("UseAdaptiveNull: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "AdaptiveNullDepth") )
        {
        AdaptiveNullDepth = atoi(arg[1]);
        printf("AdaptiveNullDepth: %d\n", AdaptiveNullDepth);
        }

    if( !strcmp(arg[0], "UseVer") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseVer = true;
            printf("UseVer: %s\n", "true");
            }
        else
            {
            UseVer = false;
            printf("UseVer: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "UseVerEndgame") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseVerEndgame = true;
            printf("UseVerEndgame: %s\n", "true");
            }
        else
            {
            UseVerEndgame = false;
            printf("UseVerEndgame: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "VerReduction") )
        {
        VerReduction = atoi(arg[1]);
        printf("VerReduction: %d\n", VerReduction);
        }

    // razoring ----------------
    if( !strcmp(arg[0], "UseRazoring") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseRazoring = true;
            printf("UseRazoring: %s\n", "true");
            }
        else
            {
            UseRazoring = false;
            printf("UseRazoring: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "RazorDepth") )
        {
        RazorDepth = atoi(arg[1]);
        printf("RazorDepth: %d\n", RazorDepth);
        }

    if( !strcmp(arg[0], "RazorMargin") )
        {
        RazorMargin = atoi(arg[1]);
        printf("RazorMargin: %d\n", RazorMargin);
        }

    // internal iterative deepening ----------------
    if( !strcmp(arg[0], "UseIID") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseIID = true;
            printf("UseIID: %s\n", "true");
            }
        else
            {
            UseIID = false;
            printf("UseIID: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "IIDDepth") )
        {
        IIDDepth = atoi(arg[1]);
        printf("IIDDepth: %d\n", IIDDepth);
        }

    if( !strcmp(arg[0], "IIDReduction") )
        {
        IIDReduction = atoi(arg[1]);
        printf("IIDReduction: %d\n", IIDReduction);
        }

    // history pruning ----------------
    if( !strcmp(arg[0], "UseHistory") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseHistory = true;
            printf("UseHistory: %s\n", "true");
            }
        else
            {
            UseHistory = false;
            printf("UseHistory: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "HistoryValue") )
        {
        HistoryValue = atoi(arg[1]);
        printf("HistoryValue: %d\n", HistoryValue);
        }

    if( !strcmp(arg[0], "HistoryDepth") )
        {
        HistoryDepth = atoi(arg[1]);
        printf("HistoryDepth: %d\n", HistoryDepth);
        }

    if( !strcmp(arg[0], "HistoryMoveNb") )
        {
        HistoryMoveNb = atoi(arg[1]);
        printf("HistoryMoveNb: %d\n", HistoryMoveNb);
        }

    if( !strcmp(arg[0], "HistoryPVMoveNb") )
        {
        HistoryPVMoveNb = atoi(arg[1]);
        printf("HistoryPVMoveNb: %d\n", HistoryPVMoveNb);
        }

    if( !strcmp(arg[0], "HistoryReSearch") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            HistoryReSearch = true;
            printf("HistoryReSearch: %s\n", "true");
            }
        else
            {
            HistoryReSearch = false;
            printf("HistoryReSearch: %s\n", "false");
            }
        }

    // futility pruning ----------------
    if( !strcmp(arg[0], "UseFutility") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseFutility = true;
            printf("UseFutility: %s\n", "true");
            }
        else
            {
            UseFutility = false;
            printf("UseFutility: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "FutilityMarginBase") )
        {
        FutilityMarginBase = atoi(arg[1]);
        printf("FutilityMarginBase: %d\n", FutilityMarginBase);
        }

    if( !strcmp(arg[0], "FutilityMarginStep") )
        {
        FutilityMarginStep = atoi(arg[1]);
        printf("FutilityMarginStep: %d\n", FutilityMarginStep);
        }

    // quiescence search ----------------
    if( !strcmp(arg[0], "UseDelta") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseDelta = true;
            printf("UseDelta: %s\n", "true");
            }
        else
            {
            UseDelta = false;
            printf("UseDelta: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "DeltaMargin") )
        {
        DeltaMargin = atoi(arg[1]);
        printf("DeltaMargin: %d\n", DeltaMargin);
        }

	// transposition table ----------------
    if( !strcmp(arg[0], "UseModulo") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            UseModulo = true;
            printf("UseModulo: %s\n", "true");
            }
        else
            {
            UseModulo = false;
            printf("UseModulo: %s\n", "false");
            }
        }

	if( !strcmp(arg[0], "AlwaysWrite") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            AlwaysWrite = true;
            printf("AlwaysWrite: %s\n", "true");
            }
        else
            {
            AlwaysWrite = false;
            printf("AlwaysWrite: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "SmartMove") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            SmartMove = true;
            printf("SmartMove: %s\n", "true");
            }
        else
            {
            SmartMove = false;
            printf("SmartMove: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "SmartValue") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            SmartValue = true;
            printf("SmartValue: %s\n", "true");
            }
        else
            {
            SmartValue = false;
            printf("SmartValue: %s\n", "false");
            }
        }

    if( !strcmp(arg[0], "SmartReplace") )
        {
        int input = atoi(arg[1]);

        if( input == 1 )
            {
            SmartReplace = true;
            printf("SmartReplace: %s\n", "true");
            }
        else
            {
            SmartReplace = false;
            printf("SmartReplace: %s\n", "false");
            }
        }

	}

// read_ini_file()
void read_ini_file( char *file_name )
    {
    char parambuf[256];
    FILE *iniFile;
    iniFile = fopen(file_name, "rt");

    if( iniFile )
        {
        printf("%s found:\n", "cyclone.cfg");

        while( !feof(iniFile) )
            {
            strcpy(parambuf, "");
            fgets(parambuf, 256, iniFile);
            parse_option(parambuf);
            }
        fclose(iniFile);
        iniFile = NULL;
        printf("\n");
        }
    else
        {
        printf("%s not found:\n", "cyclone.cfg");
        printf("using default values...\n\n");
        }
    }

// gen_default_ini_file()
void gen_default_ini_file( char *file_name )
    {
	LazyEval = true;
	LazyEvalMargin = 200;

	// tempo
	UseTempo = true;
	TempoOpening = 20;
	TempoEndgame = 10;

	// mobility
	PieceActivityWeightOpening = 256;
	PieceActivityWeightEndgame = 256;
	KnightUnit = 4;
	BishopUnit = 6;
	RookUnit = 7;
	QueenUnit = 13;
	MobMove = 1;
	MobAttack = 1;
	MobDefense = 0;
	KnightMobOpening = 4;
	KnightMobEndgame = 4;
	BishopMobOpening = 5;
	BishopMobEndgame = 5;
	RookMobOpening = 2;
	RookMobEndgame = 4;
	QueenMobOpening = 2;
	QueenMobEndgame = 4;
	KingMobOpening = 0;
	KingMobEndgame = 0;

	// pawn
	PawnStructureWeightOpening = 256;
	PawnStructureWeightEndgame = 256;
	DoubledOpening = 10;
	DoubledEndgame = 20;
	IsolatedOpening = 10;
	IsolatedOpeningOpen = 20;
	IsolatedEndgame = 20;
	BackwardOpening = 8;
	BackwardOpeningOpen = 16;
	BackwardEndgame = 10;

	// passed pawn
	PassedPawnWeightOpening = 256;
	PassedPawnWeightEndgame = 256;
	CandidateOpeningMin = 5;
	CandidateOpeningMax = 55;
	CandidateEndgameMin = 10;
	CandidateEndgameMax = 110;
	PassedOpeningMin = 10;
	PassedOpeningMax = 70;
	PassedEndgameMin = 20;
	PassedEndgameMax = 140;
	UnstoppablePasser = 800;
	FreePasser = 60;

	// bishop
	UseBishopOutpost = true;
	TrappedBishop = 100;
	BlockedBishop = 50;

	// rook
	UseOpenFile = true;
	RookSemiOpenFileOpening = 10;
	RookSemiOpenFileEndgame = 10;
	RookOpenFileOpening = 20;
	RookOpenFileEndgame = 20;
	RookSemiKingFileOpening = 10;
	RookKingFileOpening = 20;
	RookKingFileEndgame = 10;
	Rook7thOpening = 20;
	Rook7thEndgame = 40;
	BlockedRook = 50;

	// queen
	Queen7thOpening = 10;
	Queen7thEndgame = 20;

	// king
	KingSafetyWeightOpening = 256;
	KingSafetyWeightEndgame = 256;
	KingSafety = false;
	KingSafetyMargin = 1600;
	AttackerDistance = 5;
	DefenderDistance = 20;
	UseKingAttack = true;
	KingAttackOpening = 20;
	KingAttackOpening_1 = 20;
	UseShelter = true;
	ShelterOpening = 256;
	UseStorm = true;
	StormOpening = 10;

	// material
	MaterialWeightOpening = 256;
	MaterialWeightEndgame = 256;
	PawnOpening = 80;
	PawnEndgame = 90;
	KnightOpening = 325;
	KnightEndgame = 325;
	BishopOpening = 325;
	BishopEndgame = 325;
	RookOpening = 500;
	RookEndgame = 500;
	QueenOpening = 975;
	QueenEndgame = 975;
	BishopPairOpening = 50;
	BishopPairEndgame = 50;
	OpeningExchangePenalty = 30;
	EndgameExchangePenalty = 30;
	UseMaterialImbalance = false;
	MaterialImbalanceWeightOpening = 256;
	MaterialImbalanceWeightEndgame = 256;

	// pst
	PieceSquareWeightOpening = 256;
	PieceSquareWeightEndgame = 256;
	PawnFileOpening = 5;
	KnightCentreOpening = 5;
	KnightCentreEndgame = 5;
	KnightRankOpening = 5;
	KnightBackRankOpening = 0;
	KnightTrapped = 100;
	BishopCentreOpening = 2;
	BishopCentreEndgame = 3;
	BishopBackRankOpening = 10;
	BishopDiagonalOpening = 4;
	RookFileOpening = 3;
	QueenCentreOpening = 0;
	QueenCentreEndgame = 4;
	QueenBackRankOpening = 5;
	KingCentreEndgame = 12;
	KingFileOpening = 10;
	KingRankOpening = 10;

	// search
	UseCpuTime = false;
	UseEvent = true;
	UseShortSearch = true;
	ShortSearchDepth = 1;
	UseEasy = true;
	EasyThreshold = 150;
	UseEarly = true;
	UseBad = true;
	BadThreshold = 50;
	UseExtension = true;
	UseWindow = true;
	WindowSize = 20;
	UseDistancePruning = true;
	ExtendSingleReply = true;
	UseMateValues = true;

	// null move
	UseNull = true;
	NullDepth = 2;
	NullReduction = 3;
	UseAdaptiveNull = true;
	AdaptiveNullDepth = 7;
	UseVer = true;
	VerReduction = 5;
	UseVerEndgame = false;

	// razoring
	UseRazoring = false;
	RazorDepth = 3;
	RazorMargin = 300;

	// internal iterative deepening
	UseIID = true;
	IIDDepth = 3;
	IIDReduction = 2;

	// history pruning
	UseHistory = true;
	HistoryDepth = 3;
	HistoryMoveNb = 3;
	HistoryPVMoveNb = 10;
	HistoryValue = 9830;
	HistoryReSearch = false;

	// futility pruning
	UseFutility = true;
	FutilityMarginBase = 300;
	FutilityMarginStep = 50;

	// quiescence search
	UseDelta = true;
	DeltaMargin = 50;

	// trans
	UseModulo = false;
	AlwaysWrite = true;
	SmartMove = true;
	SmartValue = false;
	SmartReplace = true;

    printf("writing default cyclone.cfg...\n");
    FILE *fp;
    fp = fopen("cyclone.cfg", "w");
    fprintf(fp, "// Cyclone xTreme default parameters file\n");
    fprintf(fp, "\n");
    fprintf(fp, "// lazy eval\n");
    fprintf(fp, "LazyEval %d\n", LazyEval);
    fprintf(fp, "LazyEvalMargin %d\n", LazyEvalMargin);
    fprintf(fp, "\n");
    fprintf(fp, "// tempo\n");
    fprintf(fp, "UseTempo %d\n", UseTempo);
    fprintf(fp, "TempoOpening %d\n", TempoOpening);
    fprintf(fp, "TempoEndgame %d\n", TempoEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// mobility\n");
    fprintf(fp, "PieceActivityWeightOpening %d\n", PieceActivityWeightOpening);
    fprintf(fp, "PieceActivityWeightEndgame %d\n", PieceActivityWeightEndgame);
    fprintf(fp, "KnightUnit %d\n", KnightUnit);
    fprintf(fp, "BishopUnit %d\n", BishopUnit);
    fprintf(fp, "RookUnit %d\n", RookUnit);
    fprintf(fp, "QueenUnit %d\n", QueenUnit);
    fprintf(fp, "MobMove %d\n", MobMove);
    fprintf(fp, "MobAttack %d\n", MobAttack);
    fprintf(fp, "MobDefense %d\n", MobDefense);
    fprintf(fp, "KnightMobOpening %d\n", KnightMobOpening);
    fprintf(fp, "KnightMobEndgame %d\n", KnightMobEndgame);
    fprintf(fp, "BishopMobOpening %d\n", BishopMobOpening);
    fprintf(fp, "BishopMobEndgame %d\n", BishopMobEndgame);
    fprintf(fp, "RookMobOpening %d\n", RookMobOpening);
    fprintf(fp, "RookMobEndgame %d\n", RookMobEndgame);
    fprintf(fp, "QueenMobOpening %d\n", QueenMobOpening);
    fprintf(fp, "QueenMobEndgame %d\n", QueenMobEndgame);
    fprintf(fp, "KingMobOpening %d\n", KingMobOpening);
    fprintf(fp, "KingMobEndgame %d\n", KingMobEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// pawn\n");
    fprintf(fp, "PawnStructureWeightOpening %d\n", PawnStructureWeightOpening);
    fprintf(fp, "PawnStructureWeightEndgame %d\n", PawnStructureWeightEndgame);
    fprintf(fp, "DoubledOpening %d\n", DoubledOpening);
    fprintf(fp, "DoubledEndgame %d\n", DoubledEndgame);
    fprintf(fp, "IsolatedOpening %d\n", IsolatedOpening);
    fprintf(fp, "IsolatedOpeningOpen %d\n", IsolatedOpeningOpen);
    fprintf(fp, "IsolatedEndgame %d\n", IsolatedEndgame);
    fprintf(fp, "BackwardOpening %d\n", BackwardOpening);
    fprintf(fp, "BackwardOpeningOpen %d\n", BackwardOpeningOpen);
    fprintf(fp, "BackwardEndgame %d\n", BackwardEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// passed pawn\n");
    fprintf(fp, "PassedPawnWeightOpening %d\n", PassedPawnWeightOpening);
    fprintf(fp, "PassedPawnWeightEndgame %d\n", PassedPawnWeightEndgame);
    fprintf(fp, "CandidateOpeningMin %d\n", CandidateOpeningMin);
    fprintf(fp, "CandidateOpeningMax %d\n", CandidateOpeningMax);
    fprintf(fp, "CandidateEndgameMin %d\n", CandidateEndgameMin);
    fprintf(fp, "CandidateEndgameMax %d\n", CandidateEndgameMax);
    fprintf(fp, "PassedOpeningMin %d\n", PassedOpeningMin);
    fprintf(fp, "PassedOpeningMax %d\n", PassedOpeningMax);
    fprintf(fp, "PassedEndgameMin %d\n", PassedEndgameMin);
    fprintf(fp, "PassedEndgameMax %d\n", PassedEndgameMax);
    fprintf(fp, "UnstoppablePasser %d\n", UnstoppablePasser);
    fprintf(fp, "FreePasser %d\n", FreePasser);
    fprintf(fp, "\n");
    fprintf(fp, "// bishop\n");
    fprintf(fp, "UseBishopOutpost %d\n", UseBishopOutpost);
    fprintf(fp, "TrappedBishop %d\n", TrappedBishop);
    fprintf(fp, "BlockedBishop %d\n", BlockedBishop);
    fprintf(fp, "\n");
    fprintf(fp, "// rook\n");
    fprintf(fp, "UseOpenFile %d\n", UseOpenFile);
    fprintf(fp, "RookSemiOpenFileOpening %d\n", RookSemiOpenFileOpening);
    fprintf(fp, "RookSemiOpenFileEndgame %d\n", RookSemiOpenFileEndgame);
    fprintf(fp, "RookOpenFileOpening %d\n", RookOpenFileOpening);
    fprintf(fp, "RookOpenFileEndgame %d\n", RookOpenFileEndgame);
    fprintf(fp, "RookSemiKingFileOpening %d\n", RookSemiKingFileOpening);
    fprintf(fp, "RookKingFileOpening %d\n", RookKingFileOpening);
    fprintf(fp, "RookKingFileEndgame %d\n", RookKingFileEndgame);
    fprintf(fp, "Rook7thOpening %d\n", Rook7thOpening);
    fprintf(fp, "Rook7thEndgame %d\n", Rook7thEndgame);
    fprintf(fp, "BlockedRook %d\n", BlockedRook);
    fprintf(fp, "\n");
    fprintf(fp, "// queen\n");
    fprintf(fp, "Queen7thOpening %d\n", Queen7thOpening);
    fprintf(fp, "Queen7thEndgame %d\n", Queen7thEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// king\n");
    fprintf(fp, "KingSafety %d\n", KingSafety);
    fprintf(fp, "KingSafetyWeightOpening %d\n", KingSafetyWeightOpening);
    fprintf(fp, "KingSafetyWeightEndgame %d\n", KingSafetyWeightEndgame);
    fprintf(fp, "KingSafetyMargin %d\n", KingSafetyMargin);
    fprintf(fp, "AttackerDistance %d\n", AttackerDistance);
    fprintf(fp, "DefenderDistance %d\n", DefenderDistance);
    fprintf(fp, "UseKingAttack %d\n", UseKingAttack);
    fprintf(fp, "KingAttackOpening %d\n", KingAttackOpening);
    fprintf(fp, "KingAttackOpening_1 %d\n", KingAttackOpening_1);
    fprintf(fp, "UseShelter %d\n", UseShelter);
    fprintf(fp, "ShelterOpening %d\n", ShelterOpening);
    fprintf(fp, "UseStorm %d\n", UseStorm);
    fprintf(fp, "StormOpening %d\n", StormOpening);
    fprintf(fp, "\n");
    fprintf(fp, "// material\n");
    fprintf(fp, "MaterialWeightOpening %d\n", MaterialWeightOpening);
    fprintf(fp, "MaterialWeightEndgame %d\n", MaterialWeightEndgame);
    fprintf(fp, "PawnOpening %d\n", PawnOpening);
    fprintf(fp, "PawnEndgame %d\n", PawnEndgame);
    fprintf(fp, "KnightOpening %d\n", KnightOpening);
    fprintf(fp, "KnightEndgame %d\n", KnightEndgame);
    fprintf(fp, "BishopOpening %d\n", BishopOpening);
    fprintf(fp, "BishopEndgame %d\n", BishopEndgame);
    fprintf(fp, "RookOpening %d\n", RookOpening);
    fprintf(fp, "RookEndgame %d\n", RookEndgame);
    fprintf(fp, "QueenOpening %d\n", QueenOpening);
    fprintf(fp, "QueenEndgame %d\n", QueenEndgame);
    fprintf(fp, "BishopPairOpening %d\n", BishopPairOpening);
    fprintf(fp, "BishopPairEndgame %d\n", BishopPairEndgame);
    fprintf(fp, "OpeningExchangePenalty %d\n", OpeningExchangePenalty);
    fprintf(fp, "EndgameExchangePenalty %d\n", EndgameExchangePenalty);
    fprintf(fp, "UseMaterialImbalance %d\n", UseMaterialImbalance);
    fprintf(fp, "MaterialImbalanceWeightOpening %d\n", MaterialImbalanceWeightOpening);
    fprintf(fp, "MaterialImbalanceWeightEndgame %d\n", MaterialImbalanceWeightEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// pst\n");
    fprintf(fp, "PieceSquareWeightOpening %d\n", PieceSquareWeightOpening);
    fprintf(fp, "PieceSquareWeightEndgame %d\n", PieceSquareWeightEndgame);
    fprintf(fp, "PawnFileOpening %d\n", PawnFileOpening);
    fprintf(fp, "KnightCentreOpening %d\n", KnightCentreOpening);
    fprintf(fp, "KnightCentreEndgame %d\n", KnightCentreEndgame);
    fprintf(fp, "KnightRankOpening %d\n", KnightRankOpening);
    fprintf(fp, "KnightBackRankOpening %d\n", KnightBackRankOpening);
    fprintf(fp, "KnightTrapped %d\n", KnightTrapped);
    fprintf(fp, "BishopCentreOpening %d\n", BishopCentreOpening);
    fprintf(fp, "BishopCentreEndgame %d\n", BishopCentreEndgame);
    fprintf(fp, "BishopBackRankOpening %d\n", BishopBackRankOpening);
    fprintf(fp, "BishopDiagonalOpening %d\n", BishopDiagonalOpening);
    fprintf(fp, "RookFileOpening %d\n", RookFileOpening);
    fprintf(fp, "QueenCentreOpening %d\n", QueenCentreOpening);
    fprintf(fp, "QueenCentreEndgame %d\n", QueenCentreEndgame);
    fprintf(fp, "QueenBackRankOpening %d\n", QueenBackRankOpening);
    fprintf(fp, "KingCentreEndgame %d\n", KingCentreEndgame);
    fprintf(fp, "KingFileOpening %d\n", KingFileOpening);
    fprintf(fp, "KingRankOpening %d\n", KingRankOpening);
    fprintf(fp, "\n");
    fprintf(fp, "// search\n");
    fprintf(fp, "UseCpuTime %d\n", UseCpuTime);
    fprintf(fp, "UseEvent %d\n", UseEvent);
    fprintf(fp, "UseShortSearch %d\n", UseShortSearch);
    fprintf(fp, "ShortSearchDepth %d\n", ShortSearchDepth);
    fprintf(fp, "UseEarly %d\n", UseEarly);
    fprintf(fp, "UseEasy %d\n", UseEasy);
    fprintf(fp, "EasyThreshold %d\n", EasyThreshold);
    fprintf(fp, "UseBad %d\n", UseBad);
	fprintf(fp, "BadThreshold %d\n", BadThreshold);
    fprintf(fp, "UseExtension %d\n", UseExtension);
    fprintf(fp, "UseWindow %d\n", UseWindow);
    fprintf(fp, "WindowSize %d\n", WindowSize);
    fprintf(fp, "UseDistancePruning %d\n", UseDistancePruning);
    fprintf(fp, "ExtendSingleReply %d\n", ExtendSingleReply);
    fprintf(fp, "UseMateValues %d\n", UseMateValues);
    fprintf(fp, "\n");
    fprintf(fp, "// null move\n");
    fprintf(fp, "UseNull %d\n", UseNull);
    fprintf(fp, "NullDepth %d\n", NullDepth);
    fprintf(fp, "NullReduction %d\n", NullReduction);
    fprintf(fp, "UseAdaptiveNull %d\n", UseAdaptiveNull);
    fprintf(fp, "AdaptiveNullDepth %d\n", AdaptiveNullDepth);
    fprintf(fp, "UseVer %d\n", UseVer);
    fprintf(fp, "VerReduction %d\n", VerReduction);
    fprintf(fp, "UseVerEndgame %d\n", UseVerEndgame);
    fprintf(fp, "\n");
    fprintf(fp, "// razoring\n");
    fprintf(fp, "UseRazoring %d\n", UseRazoring);
    fprintf(fp, "RazorDepth %d\n", RazorDepth);
    fprintf(fp, "RazorMargin %d\n", RazorMargin);
    fprintf(fp, "\n");
    fprintf(fp, "// internal iterative deepening\n");
    fprintf(fp, "UseIID %d\n", UseIID);
    fprintf(fp, "IIDDepth %d\n", IIDDepth);
    fprintf(fp, "IIDReduction %d\n", IIDReduction);
    fprintf(fp, "\n");
    fprintf(fp, "// history pruning\n");
    fprintf(fp, "UseHistory %d\n", UseHistory);
    fprintf(fp, "HistoryValue %d\n", HistoryValue);
    fprintf(fp, "HistoryDepth %d\n", HistoryDepth);
    fprintf(fp, "HistoryMoveNb %d\n", HistoryMoveNb);
    fprintf(fp, "HistoryPVMoveNb %d\n", HistoryPVMoveNb);
    fprintf(fp, "HistoryReSearch %d\n", HistoryReSearch);
    fprintf(fp, "\n");
    fprintf(fp, "// futility pruning\n");
    fprintf(fp, "UseFutility %d\n", UseFutility);
    fprintf(fp, "FutilityMarginBase %d\n", FutilityMarginBase);
    fprintf(fp, "FutilityMarginStep %d\n", FutilityMarginStep);
    fprintf(fp, "\n");
    fprintf(fp, "// quiescence search\n");
    fprintf(fp, "UseDelta %d\n", UseDelta);
    fprintf(fp, "DeltaMargin %d\n", DeltaMargin);
    fprintf(fp, "\n");
    fprintf(fp, "// transposition table\n");
    fprintf(fp, "UseModulo %d\n", UseModulo);
    fprintf(fp, "AlwaysWrite %d\n", AlwaysWrite);
    fprintf(fp, "SmartMove %d\n", SmartMove);
    fprintf(fp, "SmartValue %d\n", SmartValue);
    fprintf(fp, "SmartReplace %d\n", SmartReplace);
    fclose(fp);
    printf("done.\n\n");
    }

// gen_random_ini_file()

void gen_random_ini_file( char *file_name )
    {
    printf("writing random cyclone.cfg...\n");
    FILE *fp;
    fp = fopen("cyclone.cfg", "w");
    fprintf(fp, "// Cyclone xTreme random parameters file\n");
    fprintf(fp, "\n");
    fprintf(fp, "// lazy eval\n");
    fprintf(fp, "LazyEval %d\n", get_rand_num(0, 1));
    fprintf(fp, "LazyEvalMargin %d\n", get_rand_num(150, 250));
    fprintf(fp, "\n");
    fprintf(fp, "// tempo\n");
    fprintf(fp, "UseTempo %d\n", get_rand_num(0, 1));
    fprintf(fp, "TempoOpening %d\n", get_rand_num(0, 40));
    fprintf(fp, "TempoEndgame %d\n", get_rand_num(0, 20));
    fprintf(fp, "\n");
    fprintf(fp, "// mobility\n");
    fprintf(fp, "PieceActivityWeightOpening %d\n", get_rand_num(90, 110));
    fprintf(fp, "PieceActivityWeightEndgame %d\n", get_rand_num(90, 110));
    fprintf(fp, "KnightUnit %d\n", get_rand_num(3, 5));
    fprintf(fp, "BishopUnit %d\n", get_rand_num(5, 7));
    fprintf(fp, "RookUnit %d\n", get_rand_num(6, 8));
    fprintf(fp, "QueenUnit %d\n", get_rand_num(12, 14));
    fprintf(fp, "MobMove %d\n", get_rand_num(0, 2));
    fprintf(fp, "MobAttack %d\n", get_rand_num(0, 2));
    fprintf(fp, "MobDefense %d\n", get_rand_num(0, 1));
    fprintf(fp, "KnightMobOpening %d\n", get_rand_num(3, 5));
    fprintf(fp, "KnightMobEndgame %d\n", get_rand_num(3, 5));
    fprintf(fp, "BishopMobOpening %d\n", get_rand_num(4, 6));
    fprintf(fp, "BishopMobEndgame %d\n", get_rand_num(4, 6));
    fprintf(fp, "RookMobOpening %d\n", get_rand_num(1, 3));
    fprintf(fp, "RookMobEndgame %d\n", get_rand_num(3, 5));
    fprintf(fp, "QueenMobOpening %d\n", get_rand_num(1, 3));
    fprintf(fp, "QueenMobEndgame %d\n", get_rand_num(3, 5));
    fprintf(fp, "KingMobOpening %d\n", get_rand_num(0, 1));
    fprintf(fp, "KingMobEndgame %d\n", get_rand_num(0, 1));
    fprintf(fp, "\n");
    fprintf(fp, "// pawn\n");
    fprintf(fp, "PawnStructureWeight %d\n", get_rand_num(90, 110));
    fprintf(fp, "DoubledOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "DoubledEndgame %d\n", get_rand_num(15, 25));
    fprintf(fp, "IsolatedOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "IsolatedOpeningOpen %d\n", get_rand_num(15, 25));
    fprintf(fp, "IsolatedEndgame %d\n", get_rand_num(15, 25));
    fprintf(fp, "BackwardOpening %d\n", get_rand_num(4, 12));
    fprintf(fp, "BackwardOpeningOpen %d\n", get_rand_num(11, 21));
    fprintf(fp, "BackwardEndgame %d\n", get_rand_num(5, 15));
    fprintf(fp, "\n");
    fprintf(fp, "// passed pawn\n");
    fprintf(fp, "PassedPawnWeightOpening %d\n", get_rand_num(90, 110));
    fprintf(fp, "PassedPawnWeightEndgame %d\n", get_rand_num(90, 110));
    fprintf(fp, "CandidateOpeningMin %d\n", get_rand_num(0, 10));
    fprintf(fp, "CandidateOpeningMax %d\n", get_rand_num(35, 75));
    fprintf(fp, "CandidateEndgameMin %d\n", get_rand_num(5, 15));
    fprintf(fp, "CandidateEndgameMax %d\n", get_rand_num(80, 140));
    fprintf(fp, "PassedOpeningMin %d\n", get_rand_num(5, 15));
    fprintf(fp, "PassedOpeningMax %d\n", get_rand_num(50, 90));
    fprintf(fp, "PassedEndgameMin %d\n", get_rand_num(10, 30));
    fprintf(fp, "PassedEndgameMax %d\n", get_rand_num(100, 180));
    fprintf(fp, "UnstoppablePasser %d\n", get_rand_num(600, 1000));
    fprintf(fp, "FreePasser %d\n", get_rand_num(20, 100));
    fprintf(fp, "\n");
    fprintf(fp, "// bishop\n");
    fprintf(fp, "UseBishopOutpost %d\n", get_rand_num(0, 1));
    fprintf(fp, "TrappedBishop %d\n", get_rand_num(90, 110));
    fprintf(fp, "BlockedBishop %d\n", get_rand_num(40, 60));
    fprintf(fp, "\n");
    fprintf(fp, "// rook\n");
    fprintf(fp, "UseOpenFile %d\n", get_rand_num(0, 1));
    fprintf(fp, "RookSemiOpenFileOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "RookSemiOpenFileEndgame %d\n", get_rand_num(5, 15));
    fprintf(fp, "RookOpenFileOpening %d\n", get_rand_num(15, 25));
    fprintf(fp, "RookOpenFileEndgame %d\n", get_rand_num(15, 25));
    fprintf(fp, "RookSemiKingFileOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "RookKingFileOpening %d\n", get_rand_num(15, 25));
    fprintf(fp, "RookKingFileEndgame %d\n", get_rand_num(5, 15));
    fprintf(fp, "Rook7thOpening %d\n", get_rand_num(15, 25));
    fprintf(fp, "Rook7thEndgame %d\n", get_rand_num(30, 50));
    fprintf(fp, "BlockedRook %d\n", get_rand_num(40, 60));
    fprintf(fp, "\n");
    fprintf(fp, "// queen\n");
    fprintf(fp, "Queen7thOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "Queen7thEndgame %d\n", get_rand_num(15, 25));
    fprintf(fp, "\n");
    fprintf(fp, "// king\n");
    fprintf(fp, "KingSafety %d\n", get_rand_num(0, 1));
    fprintf(fp, "KingSafetyWeightOpening %d\n", get_rand_num(90, 110));
    fprintf(fp, "KingSafetyWeightEndgame %d\n", get_rand_num(90, 110));
    fprintf(fp, "KingSafetyMargin %d\n", get_rand_num(1400, 1800));
    fprintf(fp, "AttackerDistance %d\n", get_rand_num(3, 7));
    fprintf(fp, "DefenderDistance %d\n", get_rand_num(15, 25));
    fprintf(fp, "UseKingAttack %d\n", get_rand_num(0, 1));
    fprintf(fp, "KingAttackOpening %d\n", get_rand_num(15, 25));
    fprintf(fp, "KingAttackOpening_1 %d\n", get_rand_num(15, 25));
    fprintf(fp, "UseShelter %d\n", get_rand_num(0, 1));
    fprintf(fp, "ShelterOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "UseStorm %d\n", get_rand_num(0, 1));
    fprintf(fp, "StormOpening %d\n", get_rand_num(5, 15));
    fprintf(fp, "\n");
    fprintf(fp, "// material\n");
    fprintf(fp, "MaterialWeightOpening %d\n", get_rand_num(90, 110));
    fprintf(fp, "MaterialWeightEndgame %d\n", get_rand_num(90, 110));
    fprintf(fp, "PawnOpening %d\n", get_rand_num(70, 90));
    fprintf(fp, "PawnEndgame %d\n", get_rand_num(80, 100));
    fprintf(fp, "KnightOpening %d\n", get_rand_num(300, 350));
    fprintf(fp, "KnightEndgame %d\n", get_rand_num(300, 350));
    fprintf(fp, "BishopOpening %d\n", get_rand_num(300, 350));
    fprintf(fp, "BishopEndgame %d\n", get_rand_num(300, 350));
    fprintf(fp, "RookOpening %d\n", get_rand_num(480, 520));
    fprintf(fp, "RookEndgame %d\n", get_rand_num(480, 520));
    fprintf(fp, "QueenOpening %d\n", get_rand_num(900, 1050));
    fprintf(fp, "QueenEndgame %d\n", get_rand_num(900, 1050));
    fprintf(fp, "BishopPairOpening %d\n", get_rand_num(30, 70));
    fprintf(fp, "BishopPairEndgame %d\n", get_rand_num(40, 80));
    fprintf(fp, "OpeningExchangePenalty %d\n", get_rand_num(20, 40));
    fprintf(fp, "EndgameExchangePenalty %d\n", get_rand_num(20, 40));
    fprintf(fp, "UseMaterialImbalance %d\n", get_rand_num(0, 1));
    fprintf(fp, "MaterialImbalanceWeight %d\n", get_rand_num(90, 110));
    fprintf(fp, "\n");
    fprintf(fp, "// pst\n");
    fprintf(fp, "PieceSquareWeightOpening %d\n", get_rand_num(90, 110));
    fprintf(fp, "PieceSquareWeightEndgame %d\n", get_rand_num(90, 110));
    fprintf(fp, "PawnFileOpening %d\n", get_rand_num(3, 7));
    fprintf(fp, "KnightCentreOpening %d\n", get_rand_num(3, 7));
    fprintf(fp, "KnightCentreEndgame %d\n", get_rand_num(3, 7));
    fprintf(fp, "KnightRankOpening %d\n", get_rand_num(3, 7));
    fprintf(fp, "KnightBackRankOpening %d\n", get_rand_num(0, 1));
    fprintf(fp, "KnightTrapped %d\n", get_rand_num(80, 120));
    fprintf(fp, "BishopCentreOpening %d\n", get_rand_num(1, 3));
    fprintf(fp, "BishopCentreEndgame %d\n", get_rand_num(2, 4));
    fprintf(fp, "BishopBackRankOpening %d\n", get_rand_num(8, 12));
    fprintf(fp, "BishopDiagonalOpening %d\n", get_rand_num(3, 5));
    fprintf(fp, "RookFileOpening %d\n", get_rand_num(2, 4));
    fprintf(fp, "QueenCentreOpening %d\n", get_rand_num(0, 1));
    fprintf(fp, "QueenCentreEndgame %d\n", get_rand_num(3, 5));
    fprintf(fp, "QueenBackRankOpening %d\n", get_rand_num(4, 6));
    fprintf(fp, "KingCentreEndgame %d\n", get_rand_num(10, 14));
    fprintf(fp, "KingFileOpening %d\n", get_rand_num(8, 12));
    fprintf(fp, "KingRankOpening %d\n", get_rand_num(8, 12));
    fprintf(fp, "\n");
    fprintf(fp, "// search\n");
    fprintf(fp, "UseCpuTime %d\n", get_rand_num(0, 1));
    fprintf(fp, "UseEvent %d\n", get_rand_num(0, 1));
    fprintf(fp, "UseShortSearch %d\n", get_rand_num(0, 1));
    fprintf(fp, "ShortSearchDepth %d\n", get_rand_num(0, 2));
    fprintf(fp, "UseEarly %d\n", get_rand_num(0, 1));
    fprintf(fp, "UseEasy %d\n", get_rand_num(0, 1));
    fprintf(fp, "EasyThreshold %d\n", get_rand_num(0, 300));
    fprintf(fp, "UseBad %d\n", get_rand_num(0, 1));
    fprintf(fp, "BadThreshold %d\n", get_rand_num(0, 100));
    fprintf(fp, "UseExtension %d\n", get_rand_num(0, 1));
    fprintf(fp, "UseWindow %d\n", get_rand_num(0, 1));
    fprintf(fp, "WindowSize %d\n", get_rand_num(1, 40));
    fprintf(fp, "UseDistancePruning %d\n", get_rand_num(0, 1));
    fprintf(fp, "ExtendSingleReply %d\n", get_rand_num(0, 1));
    fprintf(fp, "UseMateValues %d\n", get_rand_num(0, 1));
    fprintf(fp, "\n");
    fprintf(fp, "// null move\n");
    fprintf(fp, "UseNull %d\n", get_rand_num(0, 1));
    fprintf(fp, "NullDepth %d\n", get_rand_num(1, 3));
    fprintf(fp, "NullReduction %d\n", get_rand_num(2, 4));
    fprintf(fp, "UseAdaptiveNull %d\n", get_rand_num(0, 1));
    fprintf(fp, "AdaptiveNullDepth %d\n", get_rand_num(3, 10));
    fprintf(fp, "UseVer %d\n", get_rand_num(0, 1));
    fprintf(fp, "VerReduction %d\n", get_rand_num(4, 6));
    fprintf(fp, "UseVerEndgame %d\n", get_rand_num(0, 1));
    fprintf(fp, "\n");
    fprintf(fp, "// razoring\n");
    fprintf(fp, "UseRazoring %d\n", get_rand_num(0, 1));
    fprintf(fp, "RazorDepth %d\n", get_rand_num(1, 5));
    fprintf(fp, "RazorMargin %d\n", get_rand_num(100, 500));
    fprintf(fp, "\n");
    fprintf(fp, "// internal iterative deepening\n");
    fprintf(fp, "UseIID %d\n", get_rand_num(0, 1));
    fprintf(fp, "IIDDepth %d\n", get_rand_num(2, 4));
    fprintf(fp, "IIDReduction %d\n", get_rand_num(1, 3));
    fprintf(fp, "\n");
    fprintf(fp, "// history pruning\n");
    fprintf(fp, "UseHistory %d\n", get_rand_num(0, 1));
    fprintf(fp, "HistoryValue %d\n", get_rand_num(8192, 16384));
    fprintf(fp, "HistoryDepth %d\n", get_rand_num(2, 4));
    fprintf(fp, "HistoryMoveNb %d\n", get_rand_num(2, 4));
    fprintf(fp, "HistoryPVMoveNb %d\n", get_rand_num(5, 15));
    fprintf(fp, "HistoryReSearch %d\n", get_rand_num(0, 1));
    fprintf(fp, "\n");
    fprintf(fp, "// futility pruning\n");
    fprintf(fp, "UseFutility %d\n", get_rand_num(0, 1));
    fprintf(fp, "FutilityPruningBase %d\n", get_rand_num(200, 400));
    fprintf(fp, "FutilityPruningStep %d\n", get_rand_num(0, 100));
    fprintf(fp, "\n");
    fprintf(fp, "// quiescence search\n");
    fprintf(fp, "UseDelta %d\n", get_rand_num(0, 1));
    fprintf(fp, "DeltaMargin %d\n", get_rand_num(40, 60));
    fprintf(fp, "\n");
    fprintf(fp, "// transposition table\n");
    fprintf(fp, "UseModulo %d\n", get_rand_num(0, 1));
    fprintf(fp, "AlwaysWrite %d\n", get_rand_num(0, 1));
    fprintf(fp, "SmartMove %d\n", get_rand_num(0, 1));
    fprintf(fp, "SmartValue %d\n", get_rand_num(0, 1));
    fprintf(fp, "SmartReplace %d\n", get_rand_num(0, 1));
    fclose(fp);
    printf("done.\n\n");
    }

// get_rand_num
static int get_rand_num( int min, int max )
    {
    int range;
    int r;
    unsigned first = time(NULL);
    srand(first);
    range = max - min + 1;
    r = (rand() % (range)) + min;
    return r;
    }

// end of init.cpp
