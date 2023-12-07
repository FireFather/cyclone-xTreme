// init.h

#ifndef INIT_H
#define INIT_H

#include "util.h"

// lazy eval
extern bool LazyEval;
extern int LazyEvalMargin;

// tempo
extern bool UseTempo;
extern int TempoOpening;
extern int TempoEndgame;

//mobility
extern int PieceActivityWeightOpening;
extern int PieceActivityWeightEndgame;
extern int KnightUnit;
extern int BishopUnit;
extern int RookUnit;
extern int QueenUnit;
extern int MobMove;
extern int MobAttack;
extern int MobDefense;
extern int KnightMobOpening;
extern int KnightMobEndgame;
extern int BishopMobOpening;
extern int BishopMobEndgame;
extern int RookMobOpening;
extern int RookMobEndgame;
extern int QueenMobOpening;
extern int QueenMobEndgame;
extern int KingMobOpening;
extern int KingMobEndgame;

//pawn
extern int PawnStructureWeightOpening;
extern int PawnStructureWeightEndgame;
extern int DoubledOpening;
extern int DoubledEndgame;
extern int IsolatedOpening;
extern int IsolatedOpeningOpen;
extern int IsolatedEndgame;
extern int BackwardOpening;
extern int BackwardOpeningOpen;
extern int BackwardEndgame;

//passed pawn
extern int PassedPawnWeightOpening;
extern int PassedPawnWeightEndgame;
extern int CandidateOpeningMin;
extern int CandidateOpeningMax;
extern int CandidateEndgameMin;
extern int CandidateEndgameMax;
extern int PassedOpeningMin;
extern int PassedOpeningMax;
extern int PassedEndgameMin;
extern int PassedEndgameMax;
extern int UnstoppablePasser;
extern int FreePasser;

//bishop
extern bool UseBishopOutpost;
extern int TrappedBishop;
extern int BlockedBishop;

//rook
extern bool UseOpenFile;
extern int RookSemiOpenFileOpening;
extern int RookSemiOpenFileEndgame;
extern int RookOpenFileOpening;
extern int RookOpenFileEndgame;
extern int RookSemiKingFileOpening;
extern int RookKingFileOpening;
extern int RookKingFileEndgame;
extern int Rook7thOpening;
extern int Rook7thEndgame;
extern int BlockedRook;

//queen
extern int Queen7thOpening;
extern int Queen7thEndgame;

//king
extern int KingSafetyWeightOpening;
extern int KingSafetyWeightEndgame;
extern bool KingSafety;
extern int KingSafetyMargin;
extern int AttackerDistance;
extern int DefenderDistance;
extern bool UseKingAttack;
extern int KingAttackOpening;
extern int KingAttackOpening_1;
extern bool UseShelter;
extern int ShelterOpening;
extern bool UseStorm;
extern int StormOpening;

// material
extern int MaterialWeightOpening;
extern int MaterialWeightEndgame;
extern int PawnOpening;
extern int PawnEndgame;
extern int KnightOpening;
extern int KnightEndgame;
extern int BishopOpening;
extern int BishopEndgame;
extern int RookOpening;
extern int RookEndgame;
extern int QueenOpening;
extern int QueenEndgame;
extern int BishopPairOpening;
extern int BishopPairEndgame;
extern int OpeningExchangePenalty;
extern int EndgameExchangePenalty;
extern bool UseMaterialImbalance;
extern int MaterialImbalanceWeightOpening;
extern int MaterialImbalanceWeightEndgame;

//pst
extern int PieceSquareWeightOpening;
extern int PieceSquareWeightEndgame;
extern int PawnFileOpening;
extern int KnightCentreOpening;
extern int KnightCentreEndgame;
extern int KnightRankOpening;
extern int KnightBackRankOpening;
extern int KnightTrapped;
extern int BishopCentreOpening;
extern int BishopCentreEndgame;
extern int BishopBackRankOpening;
extern int BishopDiagonalOpening;
extern int RookFileOpening;
extern int QueenCentreOpening;
extern int QueenCentreEndgame;
extern int QueenBackRankOpening;
extern int KingCentreEndgame;
extern int KingFileOpening;
extern int KingRankOpening;

// search
extern bool UseCpuTime;
extern bool UseEvent;
extern bool UseShortSearch;
extern int ShortSearchDepth;
extern bool UseEarly;
extern bool UseEasy;
extern int EasyThreshold;
extern bool UseBad;
extern int BadThreshold;
extern bool UseExtension;
extern bool UseWindow;
extern int WindowSize;
extern bool UseDistancePruning;
extern bool ExtendSingleReply;
extern bool UseMateValues;

// null move
extern bool UseNull;
extern int NullDepth;
extern int NullReduction;
extern bool UseAdaptiveNull;
extern int AdaptiveNullDepth;
extern bool UseVer;
extern int VerReduction;
extern bool UseVerEndgame;

// razoring
extern bool UseRazoring;
extern int RazorDepth;
extern int RazorMargin;

// internal iterative deepening
extern bool UseIID;
extern int IIDDepth;
extern int IIDReduction;

// history pruning
extern bool UseHistory;
extern int HistoryValue;
extern int HistoryDepth;
extern int HistoryMoveNb;
extern int HistoryPVMoveNb;
extern bool HistoryReSearch;

// futility pruning
extern bool UseFutility;
extern int FutilityMarginBase;
extern int FutilityMarginStep;

// quiescence search
extern bool UseDelta;
extern int DeltaMargin;

// transposition table
extern bool UseModulo;
extern bool SmartMove;
extern bool SmartValue;
extern bool SmartReplace;
extern bool AlwaysWrite;

extern void read_ini_file( char *file_name );
extern void gen_default_ini_file( char *file_name );
extern void gen_random_ini_file( char *file_name );
extern int get_rand_num( int min, int max );

#endif // !defined INIT_H
// end of init.h
