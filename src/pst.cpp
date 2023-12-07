// pst.cpp

// includes

#include "init.h"
#include "option.h"
#include "piece.h"
#include "pst.h"
#include "util.h"

// macros

#define P(piece_12,square_64,stage) (Pst[(piece_12)][(square_64)][(stage)])

// constants

static const int A1 = 000, H1 = 007;
static const int A8 = 070, H8 = 077;

// "constants"

static const int PawnFile[8] =
    {
    -3, -1, +0, +1, +1, +0, -1, -3,
    };

static const int KnightLine[8] =
    {
    -4, -2, +0, +1, +1, +0, -2, -4,
    };

static const int KnightRank[8] =
    {
    -2, -1, +0, +1, +2, +3, +2, +1,
    };

static const int BishopLine[8] =
    {
    -3, -1, +0, +1, +1, +0, -1, -3,
    };

static const int RookFile[8] =
    {
    -2, -1, +0, +1, +1, +0, -1, -2,
    };

static const int QueenLine[8] =
    {
    -3, -1, +0, +1, +1, +0, -1, -3,
    };

static const int KingLine[8] =
    {
    -3, -1, +0, +1, +1, +0, -1, -3,
    };

static const int KingFile[8] =
    {
    +3, +4, +2, +0, +0, +2, +4, +3,
    };

static const int KingRank[8] =
    {
    +1, +0, -2, -3, -4, -5, -6, -7,
    };

// variables

sint16 Pst[12][64][StageNb];

// prototypes

static int square_make( int file, int rank );
static int square_file( int square );
static int square_rank( int square );
static int square_opp( int square );

// functions

// pst_init()

void pst_init()
    {

    int i;
    int piece, sq, stage;

    // init

    for ( piece = 0; piece < 12; piece++ )
        {
        for ( sq = 0; sq < 64; sq++ )
            {
            for ( stage = 0; stage < StageNb; stage++ )
                {
                P(piece, sq, stage) = 0;
                }
            }
        }

    // pawns

    piece = WhitePawn12;

    // file

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += PawnFile[square_file(sq)] * PawnFileOpening;
        }

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * PawnStructureWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PawnStructureWeightEndgame) / 256;
        }

    // knights

    piece = WhiteKnight12;

    // centre

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += KnightLine[square_file(sq)] * KnightCentreOpening;
        P(piece, sq, Opening) += KnightLine[square_rank(sq)] * KnightCentreOpening;
        P(piece, sq, Endgame) += KnightLine[square_file(sq)] * KnightCentreEndgame;
        P(piece, sq, Endgame) += KnightLine[square_rank(sq)] * KnightCentreEndgame;
        }

    // rank

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += KnightRank[square_rank(sq)] * KnightRankOpening;
        }

    // back rank

    for ( sq = A1; sq <= H1; sq++ )
        {
        P(piece, sq, Opening) -= KnightBackRankOpening;
        }

    // "trapped"

    P(piece, A8, Opening) -= KnightTrapped;
    P(piece, H8, Opening) -= KnightTrapped;

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceSquareWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceSquareWeightEndgame) / 256;
        }

    // bishops

    piece = WhiteBishop12;

    // centre

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += BishopLine[square_file(sq)] * BishopCentreOpening;
        P(piece, sq, Opening) += BishopLine[square_rank(sq)] * BishopCentreOpening;
        P(piece, sq, Endgame) += BishopLine[square_file(sq)] * BishopCentreEndgame;
        P(piece, sq, Endgame) += BishopLine[square_rank(sq)] * BishopCentreEndgame;
        }

    // back rank

    for ( sq = A1; sq <= H1; sq++ )
        {
        P(piece, sq, Opening) -= BishopBackRankOpening;
        }

    // main diagonals

    for ( i = 0; i < 8; i++ )
        {
        sq = square_make(i, i);
        P(piece, sq, Opening) += BishopDiagonalOpening;
        P(piece, square_opp(sq), Opening) += BishopDiagonalOpening;
        }

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceSquareWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceSquareWeightEndgame) / 256;
        }

    // rooks

    piece = WhiteRook12;

    // file

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += RookFile[square_file(sq)] * RookFileOpening;
        }

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceSquareWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceSquareWeightEndgame) / 256;
        }

    // queens

    piece = WhiteQueen12;

    // centre

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += QueenLine[square_file(sq)] * QueenCentreOpening;
        P(piece, sq, Opening) += QueenLine[square_rank(sq)] * QueenCentreOpening;
        P(piece, sq, Endgame) += QueenLine[square_file(sq)] * QueenCentreEndgame;
        P(piece, sq, Endgame) += QueenLine[square_rank(sq)] * QueenCentreEndgame;
        }

    // back rank

    for ( sq = A1; sq <= H1; sq++ )
        {
        P(piece, sq, Opening) -= QueenBackRankOpening;
        }

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceSquareWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceSquareWeightEndgame) / 256;
        }

    // kings

    piece = WhiteKing12;

    // centre

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Endgame) += KingLine[square_file(sq)] * KingCentreEndgame;
        P(piece, sq, Endgame) += KingLine[square_rank(sq)] * KingCentreEndgame;
        }

    // file

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += KingFile[square_file(sq)] * KingFileOpening;
        }

    // rank

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) += KingRank[square_rank(sq)] * KingRankOpening;
        }

    // weight

    for ( sq = 0; sq < 64; sq++ )
        {
        P(piece, sq, Opening) = (P(piece, sq, Opening) * KingSafetyWeightOpening) / 256;
        P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceSquareWeightEndgame) / 256;
        }

    // symmetry copy for black

    for ( piece = 0; piece < 12; piece += 2 )
        {
        for ( sq = 0; sq < 64; sq++ )
            {
            for ( stage = 0; stage < StageNb; stage++ )
                {
                P(piece + 1, sq, stage) = -P(piece, square_opp(sq), stage);
                }
            }
        }
    }

// square_make()

static int square_make( int file, int rank )
    {

    ASSERT(file >= 0 && file < 8);
    ASSERT(rank >= 0 && rank < 8);

    return (rank << 3) | file;
    }

// square_file()

static int square_file( int square )
    {

    ASSERT(square >= 0 && square < 64);

    return square & 7;
    }

// square_rank()

static int square_rank( int square )
    {

    ASSERT(square >= 0 && square < 64);

    return square >> 3;
    }

// square_opp()

static int square_opp( int square )
    {

    ASSERT(square >= 0 && square < 64);

    return square ^ 070;
    }

// end of pst.cpp
