
#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include "../chess/types.h"


namespace Chess::Engine{
    using Depth = int;
    using Evaluation = double;

    struct EvalByPhase{
        Evaluation midgame;
        Evaluation endgame;

        EvalByPhase() : midgame(0), endgame(0) {}
        EvalByPhase(Evaluation mg, Evaluation eg) : midgame(mg), endgame(eg) {}

        EvalByPhase operator+(EvalByPhase other){ return EvalByPhase(midgame + other.midgame, endgame + other.endgame); }
        EvalByPhase operator-(EvalByPhase other){ return EvalByPhase(midgame - other.midgame, endgame - other.endgame); }

        void operator+=(EvalByPhase other){ midgame += other.midgame; endgame += other.endgame; }
        void operator-=(EvalByPhase other){ midgame -= other.midgame; endgame -= other.endgame; }

        EvalByPhase operator*(double other){ return EvalByPhase(midgame * other, endgame * other); }
        EvalByPhase operator/(double other){ return EvalByPhase(midgame / other, endgame / other); }

        void operator*=(double other){ midgame *= other; endgame *= other; }
        void operator/=(double other){ midgame /= other; endgame /= other; }

        Evaluation getEval(double phase){ return (midgame - endgame) * phase + endgame; }
    };

    // struct PVEval{
    //     Evaluation eval;
    //     std::string pv;

    //     PVEval() : move(0), pv("") {}
    //     PVEval(Evaluation mg, Evaluation eg) : midgame(mg), endgame(eg) {}

    //     EvalByPhase operator+(EvalByPhase other){ return EvalByPhase(midgame + other.midgame, endgame + other.endgame); }
    //     EvalByPhase operator-(EvalByPhase other){ return EvalByPhase(midgame - other.midgame, endgame - other.endgame); }

    // };

    namespace Evaluations{
        constexpr Depth MAX_PLY = 256;

        constexpr Evaluation EVAL_ZERO     = 0;
        constexpr Evaluation EVAL_DRAW     = 0;
        constexpr Evaluation EVAL_INF = 1000000;

        constexpr Evaluation EVAL_MATE             = 900000;
        constexpr Evaluation EVAL_MATE_IN_MAX_PLY  = EVAL_MATE - MAX_PLY;
        constexpr Evaluation EVAL_MATED_IN_MAX_PLY = -EVAL_MATE_IN_MAX_PLY;

        constexpr Evaluation EVAL_TB                 = EVAL_MATE_IN_MAX_PLY - 1;
        constexpr Evaluation EVAL_TB_WIN_IN_MAX_PLY  = EVAL_TB - MAX_PLY;
        constexpr Evaluation EVAL_TB_LOSS_IN_MAX_PLY = -EVAL_TB_WIN_IN_MAX_PLY;


        constexpr bool is_win (Evaluation eval) { return eval >= EVAL_TB_WIN_IN_MAX_PLY; }

        constexpr bool is_loss(Evaluation eval) { return eval <= EVAL_TB_LOSS_IN_MAX_PLY; }

        constexpr bool is_decisive(Evaluation eval) { return is_win(eval) || is_loss(eval); }

        Evaluation getMateEval(Depth mate_ply){
            return EVAL_MATE - mate_ply;
        }

        Evaluation normalise(Evaluation e, Colour turn){
            if (turn == Colours::WHITE) return e;
            return -e;
        }
    }


    struct MoveOrderingStruct{
        Evaluation importance;
        Move move;
        bool raised_alpha;

        MoveOrderingStruct(Move move_, Evaluation importance_, bool raised_alpha_) : move(move_), importance(importance_), raised_alpha(raised_alpha) {}
        MoveOrderingStruct(Move move_, Evaluation importance_) : move(move_), importance(importance_), raised_alpha(0) {}
        MoveOrderingStruct() : move(0), importance(0), raised_alpha(0) {}
    };


    /// @brief all times are in milliseconds
    struct SearchLimit{
        int time  = 0;
        int wtime = 0;
        int btime = 0;
        int winc  = 0;
        int binc  = 0;
        int depth = 0;
    
        bool operator==(const SearchLimit& other) {
            return (
                time  == other.time  &&
                wtime == other.wtime &&
                btime == other.btime &&
                winc  == other.winc  &&
                binc  == other.binc  &&
                depth == other.depth
            );
        }

        friend std::ostream& operator<<(std::ostream& os, const SearchLimit& s){
            os << "Time: ";
            os << s.time;
            os << ", WTime: ";
            os << s.wtime;
            os << ", BTime: ";
            os << s.btime;
            os << ", Winc: ";
            os << s.winc;
            os << ", Binc: ";
            os << s.binc;
            os << ", Depth: ";
            os << s.depth;
            return os;
        }
    
        constexpr SearchLimit(int time_ = 0, int wtime_ = 0, int btime_ = 0, int winc_ = 0, int binc_ = 0, int depth_ = 0) : time(time_), wtime(wtime_), btime(btime_), winc(winc_), binc(binc_), depth(depth_) {}
    };
    
    namespace Lichess{
        int ping = 800; // ms
    }

    Square getSquare(Square s, Colour c){
        if (c == Colours::BLACK) return s;
        return s ^ 0b111000; // inverts the y coord while preserving the x
    }

    
    namespace Values{
        constexpr SearchLimit NO_LIMIT     (0, 0, 0, 0, 0, 0);
        constexpr SearchLimit DEFAULT_LIMIT(1000, 0, 0, 0, 0, 0);

        namespace NodeTypes{
            int EXACT = 0;
            int FAIL_LOW  = 1;
            int FAIL_HIGH = 2;
        } // namespace NodeTypes

        namespace Negamax{
            Depth NULL_MOVE_PRUNE_REDUCTION = 3;
            Evaluation RAZORING_DELTA = 200; // if (qsearch eval + this) < alpha, return alpha
        } // namespace search

        namespace QSearch{
            Evaluation BIG_DELTA = 200;
        } // namespace QSEARCH

        // evaluations for specific things
        namespace Eval{
            Evaluation PIECE_MG_VALUES[6] = {62, 337, 365, 477, 1025, 0};
            Evaluation PIECE_EG_VALUES[6] = {94, 281, 297, 552,  936, 0};

            Evaluation MG_TURN_BONUS = 30;
            Evaluation EG_TURN_BONUS = 12;

            Evaluation MG_CASTLING_BONUS = 8;

            EvalByPhase getMaterialEval(Board& board){
                EvalByPhase eval = EvalByPhase();

                eval.midgame += Values::Eval::PIECE_MG_VALUES[0] * (Bitboards::countBits(board.pieces[Pieces::W_PAWN  ]) - Bitboards::countBits(board.pieces[Pieces::B_PAWN  ]));
                eval.midgame += Values::Eval::PIECE_MG_VALUES[1] * (Bitboards::countBits(board.pieces[Pieces::W_KNIGHT]) - Bitboards::countBits(board.pieces[Pieces::B_KNIGHT]));
                eval.midgame += Values::Eval::PIECE_MG_VALUES[2] * (Bitboards::countBits(board.pieces[Pieces::W_BISHOP]) - Bitboards::countBits(board.pieces[Pieces::B_BISHOP]));
                eval.midgame += Values::Eval::PIECE_MG_VALUES[3] * (Bitboards::countBits(board.pieces[Pieces::W_ROOK  ]) - Bitboards::countBits(board.pieces[Pieces::B_ROOK  ]));
                eval.midgame += Values::Eval::PIECE_MG_VALUES[4] * (Bitboards::countBits(board.pieces[Pieces::W_QUEEN ]) - Bitboards::countBits(board.pieces[Pieces::B_QUEEN ]));
                
                eval.endgame += Values::Eval::PIECE_EG_VALUES[0] * (Bitboards::countBits(board.pieces[Pieces::W_PAWN  ]) - Bitboards::countBits(board.pieces[Pieces::B_PAWN  ]));
                eval.endgame += Values::Eval::PIECE_EG_VALUES[1] * (Bitboards::countBits(board.pieces[Pieces::W_KNIGHT]) - Bitboards::countBits(board.pieces[Pieces::B_KNIGHT]));
                eval.endgame += Values::Eval::PIECE_EG_VALUES[2] * (Bitboards::countBits(board.pieces[Pieces::W_BISHOP]) - Bitboards::countBits(board.pieces[Pieces::B_BISHOP]));
                eval.endgame += Values::Eval::PIECE_EG_VALUES[3] * (Bitboards::countBits(board.pieces[Pieces::W_ROOK  ]) - Bitboards::countBits(board.pieces[Pieces::B_ROOK  ]));
                eval.endgame += Values::Eval::PIECE_EG_VALUES[4] * (Bitboards::countBits(board.pieces[Pieces::W_QUEEN ]) - Bitboards::countBits(board.pieces[Pieces::B_QUEEN ]));

                return eval;
            }

        } // namespace Eval

        namespace Eval::PSQT{
            Evaluation MG_PAWN_TABLE[64] = {
                0,   0,   0,   0,   0,   0,  0,   0,
                98, 134,  61,  95,  68, 126, 34, -11,
                -6,   7,  26,  31,  65,  56, 25, -20,
                -14,  13,   6,  28,  32,  12, 17, -23,
                -27,  -2,  -5,  35,  52,   6, 10, -25,
                -26,  -4,  -4, -10,   3,   3, 33, -12,
                -35,  -1, -20, -23, -20,  24, 38, -22,
                0,   0,   0,   0,   0,   0,  0,   0,
            };
            
            Evaluation EG_PAWN_TABLE[64] = {
                0,   0,   0,   0,   0,   0,   0,   0,
                178, 173, 158, 134, 147, 132, 165, 187,
                94, 100,  85,  67,  56,  53,  82,  84,
                32,  24,  13,   5,  -2,   4,  17,  17,
                13,   9,  -3,  -7,  -7,  -8,   3,  -1,
                4,   7,  -6,   1,   0,  -5,  -1,  -8,
                13,   8,   8,  10,  13,   0,   2,  -7,
                0,   0,   0,   0,   0,   0,   0,   0,
            };
            
            Evaluation MG_KNIGHT_TABLE[64] = {
                -167, -89, -34, -49,  61, -97, -15, -107,
                -73, -41,  72,  36,  23,  62,   7,  -17,
                -47,  60,  37,  65,  84, 129,  73,   44,
                -9,  17,  19,  53,  37,  69,  18,   22,
                -13,   4,  16,  13,  28,  19,  21,   -8,
                -23,  -9,  12,  10,  19,  17,  25,  -16,
                -29, -53, -12,  -3,  -1,  18, -14,  -19,
                -105, -21, -58, -33, -17, -28, -19,  -23,
            };
            
            Evaluation EG_KNIGHT_TABLE[64] = {
                -58, -38, -13, -28, -31, -27, -63, -99,
                -25,  -8, -25,  -2,  -9, -25, -24, -52,
                -24, -20,  10,   9,  -1,  -9, -19, -41,
                -17,   3,  22,  22,  22,  11,   8, -18,
                -18,  -6,  16,  25,  16,  17,   4, -18,
                -23,  -3,  -1,  15,  10,  -3, -20, -22,
                -42, -20, -10,  -5,  -2, -20, -23, -44,
                -29, -51, -23, -15, -22, -18, -50, -64,
            };
            
            Evaluation MG_BISHOP_TABLE[64] = {
                -29,   4, -82, -37, -25, -42,   7,  -8,
                -26,  16, -18, -13,  30,  59,  18, -47,
                -16,  37,  43,  40,  35,  50,  37,  -2,
                -4,   5,  19,  50,  37,  37,   7,  -2,
                -6,  13,  13,  26,  34,  12,  10,   4,
                0,  15,  15,  15,  14,  27,  18,  10,
                4,  15,  16,   0,   7,  21,  33,   1,
                -33,  -3, -14, -21, -13, -12, -39, -21,
            };
            
            Evaluation EG_BISHOP_TABLE[64] = {
                -14, -21, -11,  -8, -7,  -9, -17, -24,
                -8,  -4,   7, -12, -3, -13,  -4, -14,
                2,  -8,   0,  -1, -2,   6,   0,   4,
                -3,   9,  12,   9, 14,  10,   3,   2,
                -6,   3,  13,  19,  7,  10,  -3,  -9,
                -12,  -3,   8,  10, 13,   3,  -7, -15,
                -14, -18,  -7,  -1,  4,  -9, -15, -27,
                -23,  -9, -23,  -5, -9, -16,  -5, -17,
            };
            
            Evaluation MG_ROOK_TABLE[64] = {
                32,  42,  32,  51, 63,  9,  31,  43,
                27,  32,  58,  62, 80, 67,  26,  44,
                -5,  19,  26,  36, 17, 45,  61,  16,
                -24, -11,   7,  26, 24, 35,  -8, -20,
                -36, -26, -12,  -1,  9, -7,   6, -23,
                -45, -25, -16, -17,  3,  0,  -5, -33,
                -44, -16, -20,  -9, -1, 11,  -6, -71,
                -19, -13,   1,  17, 16,  7, -37, -26,
            };
            
            Evaluation EG_ROOK_TABLE[64] = {
                13, 10, 18, 15, 12,  12,   8,   5,
                11, 13, 13, 11, -3,   3,   8,   3,
                7,  7,  7,  5,  4,  -3,  -5,  -3,
                4,  3, 13,  1,  2,   1,  -1,   2,
                3,  5,  8,  4, -5,  -6,  -8, -11,
                -4,  0, -5, -1, -7, -12,  -8, -16,
                -6, -6,  0,  2, -9,  -9, -11,  -3,
                -9,  2,  3, -1, -5, -13,   4, -20,
            };
            
            Evaluation MG_QUEEN_TABLE[64] = {
                -28,   0,  29,  12,  59,  44,  43,  45,
                -24, -39,  -5,   1, -16,  57,  28,  54,
                -13, -17,   7,   8,  29,  56,  47,  57,
                -27, -27, -16, -16,  -1,  17,  -2,   1,
                -9, -26,  -9, -10,  -2,  -4,   3,  -3,
                -14,   2, -11,  -2,  -5,   2,  14,   5,
                -35,  -8,  11,   2,   8,  15,  -3,   1,
                -1, -18,  -9,  10, -15, -25, -31, -50,
            };
            
            Evaluation EG_QUEEN_TABLE[64] = {
                -9,  22,  22,  27,  27,  19,  10,  20,
                -17,  20,  32,  41,  58,  25,  30,   0,
                -20,   6,   9,  49,  47,  35,  19,   9,
                3,  22,  24,  45,  57,  40,  57,  36,
                -18,  28,  19,  47,  31,  34,  39,  23,
                -16, -27,  15,   6,   9,  17,  10,   5,
                -22, -23, -30, -16, -16, -23, -36, -32,
                -33, -28, -22, -43,  -5, -32, -20, -41,
            };
            
            Evaluation MG_KING_TABLE[64] = {
                -65,  23,  16, -15, -56, -34,   2,  13,
                29,  -1, -20,  -7,  -8,  -4, -38, -29,
                -9,  24,   2, -16, -20,   6,  22, -22,
                -17, -20, -12, -27, -30, -25, -14, -36,
                -49,  -1, -27, -39, -46, -44, -33, -51,
                -14, -14, -22, -46, -44, -30, -15, -27,
                1,   7,  -8, -64, -43, -16,   9,   8,
                -15,  42,  30, -54,   8, -28,  50,  45,
            };
            
            Evaluation EG_KING_TABLE[64] = {
                -74, -35, -18, -18, -11,  15,   4, -17,
                -12,  17,  14,  17,  17,  38,  23,  11,
                10,  17,  23,  15,  20,  45,  44,  13,
                -8,  22,  24,  27,  26,  33,  26,   3,
                -18,  -4,  21,  24,  27,  23,   9, -11,
                -19,  -3,  11,  21,  23,  16,   7,  -9,
                -27, -11,   4,  13,  14,   4,  -5, -17,
                -53, -34, -21, -11, -28, -14, -24, -43
            };
        
            EvalByPhase addPieceSquareTables(Board& board){
                EvalByPhase eval = EvalByPhase();

                Square square;
                        
                Bitboard loop_bitboard = board.pieces[Pieces::W_PAWN];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_PAWN_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_PAWN_TABLE[getSquare(square, Colours::WHITE)];
                }

                loop_bitboard = board.pieces[Pieces::B_PAWN];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_PAWN_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_PAWN_TABLE[getSquare(square, Colours::BLACK)];
                }

                loop_bitboard = board.pieces[Pieces::W_KNIGHT];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_KNIGHT_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_KNIGHT_TABLE[getSquare(square, Colours::WHITE)];
                }
                
                loop_bitboard = board.pieces[Pieces::B_KNIGHT];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_KNIGHT_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_KNIGHT_TABLE[getSquare(square, Colours::BLACK)];
                }
                
                loop_bitboard = board.pieces[Pieces::W_BISHOP];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_BISHOP_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_BISHOP_TABLE[getSquare(square, Colours::WHITE)];
                }
                
                loop_bitboard = board.pieces[Pieces::B_BISHOP];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_BISHOP_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_BISHOP_TABLE[getSquare(square, Colours::BLACK)];
                }
                
                loop_bitboard = board.pieces[Pieces::W_ROOK];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_ROOK_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_ROOK_TABLE[getSquare(square, Colours::WHITE)];
                }
                
                loop_bitboard = board.pieces[Pieces::B_ROOK];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_ROOK_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_ROOK_TABLE[getSquare(square, Colours::BLACK)];
                }
                
                loop_bitboard = board.pieces[Pieces::W_QUEEN];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_QUEEN_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_QUEEN_TABLE[getSquare(square, Colours::WHITE)];
                }
                
                loop_bitboard = board.pieces[Pieces::B_QUEEN];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_QUEEN_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_QUEEN_TABLE[getSquare(square, Colours::BLACK)];
                }
                
                loop_bitboard = board.pieces[Pieces::W_KING];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame += MG_KING_TABLE[getSquare(square, Colours::WHITE)];
                    eval.endgame += EG_KING_TABLE[getSquare(square, Colours::WHITE)];
                }
                
                loop_bitboard = board.pieces[Pieces::B_KING];
                while (loop_bitboard){
                    square = Bitboards::popNextSquare(loop_bitboard);
                    eval.midgame -= MG_KING_TABLE[getSquare(square, Colours::BLACK)];
                    eval.endgame -= EG_KING_TABLE[getSquare(square, Colours::BLACK)];
                }

                return eval;
            }
        } // namespace Eval::PieceSquareTable
    
        namespace Eval::Mobility{
            Evaluation KNIGHT_MG[] = {-62, -53, -12,  -4, 3, 13, 22, 28, 33};
            Evaluation KNIGHT_EG[] = {-81, -56, -31, -16, 5, 11, 17, 20, 25};
    
            Evaluation BISHOP_MG[] = {-48, -20, 16, 26, 38, 51, 55, 63, 63, 68, 81, 81, 91, 98};
            Evaluation BISHOP_EG[] = {-59, -23, -3, 13, 24, 42, 54, 57, 65, 73, 78, 86, 88, 97};
    
            Evaluation ROOK_MG[]   = {-60, -20,  2,  3,  3, 11,  22,  31,  40,  40,  41,  48,  57,  57,  62};
            Evaluation ROOK_EG[]   = {-78, -17, 23, 39, 70, 99, 103, 121, 134, 139, 158, 164, 168, 169, 172};
    
            Evaluation QUEEN_MG[]  = {-30, -12, -8, -9, 20, 23, 23, 35, 38, 53, 64,  65,  65,  66,  67,  67,  72,  72,  77,  79,  93, 108, 108, 108, 110, 114, 114, 116};
            Evaluation QUEEN_EG[]  = {-48, -30, -7, 19, 40, 55, 59, 75, 78, 96, 96, 100, 121, 127, 131, 133, 136, 141, 147, 150, 151, 168, 168, 171, 182, 182, 192, 219};

            template<Colour c>
            Bitboard getMobilityArea(Board& board){
                Bitboard r = (board.pieces[Pieces::gen(Pieces::W_KING, c)]) | // where our king is
                             (Bitboards::getPawnAttacks<!c>(board.pieces[Pieces::gen(Pieces::W_PAWN, !c)])) | // where their pawns attack
                             (board.pieces[Pieces::gen(Pieces::W_PAWN, c)] &
                                    (((c == Colours::WHITE) ? (Bitboards::RANK2_BB | Bitboards::RANK3_BB) : (Bitboards::RANK6_BB | Bitboards::RANK7_BB)) | // pawns on the rank 2/3
                                     Bitboards::shiftBB<(c == Colours::WHITE) ? Directions::SOUTH : Directions::NORTH>(board.all_pieces))); // blocked pawns by either colours pieces
                return ~r;
            }

            EvalByPhase getMobilityEval(Board& board){
                EvalByPhase eval = EvalByPhase();
                Square square;
                int move_count; // moves per piece

                Bitboard white_pins = board.getPins<Colours::WHITE>();
                Bitboard black_pins = board.getPins<Colours::BLACK>();
                
                Bitboard wr = board.pieces[Pieces::W_ROOK];
                Bitboard br = board.pieces[Pieces::B_ROOK];
                
                Bitboard wq = board.pieces[Pieces::W_QUEEN];
                Bitboard bq = board.pieces[Pieces::B_QUEEN];
                
                Bitboard white_mobility_area = getMobilityArea<Colours::WHITE>(board) & ~white_pins;
                Bitboard black_mobility_area = getMobilityArea<Colours::BLACK>(board) & ~black_pins;
                
                // loop through different pieces and add a value from the tables above based on how many moves that piece has
                Bitboard loop_bitboard = board.pieces[Pieces::W_KNIGHT];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getKnightAttacks(Bitboards::popNextBit(loop_bitboard)) & white_mobility_area);
                    eval.midgame += KNIGHT_MG[move_count];
                    eval.endgame += KNIGHT_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::B_KNIGHT];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getKnightAttacks(Bitboards::popNextBit(loop_bitboard)) & black_mobility_area);
                    eval.midgame -= KNIGHT_MG[move_count];
                    eval.endgame -= KNIGHT_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::W_BISHOP];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getBishopAttacks(board.all_pieces & ~wq, Bitboards::popNextSquare(loop_bitboard)) & white_mobility_area);
                    eval.midgame += BISHOP_MG[move_count];
                    eval.endgame += BISHOP_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::B_BISHOP];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getBishopAttacks(board.all_pieces & ~bq, Bitboards::popNextSquare(loop_bitboard)) & black_mobility_area);
                    eval.midgame -= BISHOP_MG[move_count];
                    eval.endgame -= BISHOP_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::W_ROOK];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getRookAttacks(board.all_pieces & ~wq & ~wr, Bitboards::popNextSquare(loop_bitboard)) & white_mobility_area);
                    eval.midgame += ROOK_MG[move_count];
                    eval.endgame += ROOK_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::B_ROOK];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getRookAttacks(board.all_pieces & ~bq & ~br, Bitboards::popNextSquare(loop_bitboard)) & black_mobility_area);
                    eval.midgame -= ROOK_MG[move_count];
                    eval.endgame -= ROOK_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::W_QUEEN];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getQueenAttacks(board.all_pieces, Bitboards::popNextSquare(loop_bitboard)) & white_mobility_area);
                    eval.midgame += QUEEN_MG[move_count];
                    eval.endgame += QUEEN_EG[move_count];
                }
                
                loop_bitboard = board.pieces[Pieces::B_QUEEN];
                while (loop_bitboard){
                    move_count = Bitboards::countBits(Bitboards::getQueenAttacks(board.all_pieces, Bitboards::popNextSquare(loop_bitboard)) & black_mobility_area);
                    eval.midgame -= QUEEN_MG[move_count];
                    eval.endgame -= QUEEN_EG[move_count];
                }

                return eval;
            }
        }

        namespace Eval::KingSafety{

            Evaluation PAWN_ATT = 10;
            Evaluation PAWN_DEF = 20;

            Evaluation KNIGHT_ATT = 30;
            Evaluation KNIGHT_DEF = 40;

            Evaluation BISHOP_ATT = 30;
            Evaluation BISHOP_DEF = 30;

            Evaluation ROOK_ATT   = 30;
            Evaluation ROOK_DEF   = 30;

            Evaluation QUEEN_ATT  = 50;
            Evaluation QUEEN_DEF  = 15;

            EvalByPhase getKingSafetyEval(Board& board){

                // get squares around the king
                Square w_king = board.kingPos<Colours::WHITE>();
                Square b_king = board.kingPos<Colours::BLACK>();

                // get square of kings if they were brought to the centre 6x6 square
                Square w_king_offedge = std::max<Square>(1, std::min<Square>(6, Squares::getX(w_king))); + 8 * std::max<Square>(1, std::min<Square>(6, Squares::getY(w_king)));
                Square b_king_offedge = std::max<Square>(1, std::min<Square>(6, Squares::getX(b_king))); + 8 * std::max<Square>(1, std::min<Square>(6, Squares::getY(b_king)));

                // squares which are dangerous for the king
                Bitboard w_kingsafety_squares = Bitboards::getKingAttacks(w_king_offedge);
                Bitboard b_kingsafety_squares = Bitboards::getKingAttacks(b_king_offedge);

                Bitboard w_blockers = board.colours[Colours::WHITE] | board.pieces[Pieces::B_PAWN];
                Bitboard b_blockers = board.colours[Colours::BLACK] | board.pieces[Pieces::W_PAWN];

                // the squares that kingsafeties can attack if it were a knight
                Bitboard w_kingsafety_knights = Bitboards::getKnightAttacks(w_kingsafety_squares);
                Bitboard b_kingsafety_knights = Bitboards::getKnightAttacks(b_kingsafety_squares);

                // the squares that kingsafeties can attack if it were a bishop
                Bitboard w_kingsafety_bishops = Bitboards::getBishopAttackedSquares(w_blockers, w_kingsafety_squares);
                Bitboard b_kingsafety_bishops = Bitboards::getBishopAttackedSquares(b_blockers, b_kingsafety_squares);
                
                // the squares that kingsafeties can attack if it were a rook
                Bitboard w_kingsafety_rooks = Bitboards::getRookAttackedSquares(w_blockers, w_kingsafety_squares);
                Bitboard b_kingsafety_rooks = Bitboards::getRookAttackedSquares(b_blockers, b_kingsafety_squares);

                int w_pawn_attackers   = Bitboards::countBits(board.pieces[Pieces::W_PAWN]   & b_kingsafety_squares);
                int w_knight_attackers = Bitboards::countBits(board.pieces[Pieces::W_KNIGHT] & b_kingsafety_knights);
                int w_bishop_attackers = Bitboards::countBits(board.pieces[Pieces::W_BISHOP] & b_kingsafety_bishops);
                int w_rook_attackers   = Bitboards::countBits(board.pieces[Pieces::W_ROOK]   & b_kingsafety_rooks);
                int w_queen_attackers  = Bitboards::countBits(board.pieces[Pieces::W_QUEEN]  & (b_kingsafety_bishops | b_kingsafety_rooks));
                
                int w_pawn_defenders   = Bitboards::countBits(board.pieces[Pieces::W_PAWN]   & w_kingsafety_squares);
                int w_knight_defenders = Bitboards::countBits(board.pieces[Pieces::W_KNIGHT] & w_kingsafety_knights);
                int w_bishop_defenders = Bitboards::countBits(board.pieces[Pieces::W_BISHOP] & w_kingsafety_bishops);
                int w_rook_defenders   = Bitboards::countBits(board.pieces[Pieces::W_ROOK]   & w_kingsafety_rooks);
                int w_queen_defenders  = Bitboards::countBits(board.pieces[Pieces::W_QUEEN]  & (w_kingsafety_bishops | w_kingsafety_rooks));
                
                int b_pawn_attackers   = Bitboards::countBits(board.pieces[Pieces::B_PAWN]   & w_kingsafety_squares);
                int b_knight_attackers = Bitboards::countBits(board.pieces[Pieces::B_KNIGHT] & w_kingsafety_knights);
                int b_bishop_attackers = Bitboards::countBits(board.pieces[Pieces::B_BISHOP] & w_kingsafety_bishops);
                int b_rook_attackers   = Bitboards::countBits(board.pieces[Pieces::B_ROOK]   & w_kingsafety_rooks);
                int b_queen_attackers  = Bitboards::countBits(board.pieces[Pieces::B_QUEEN]  & (w_kingsafety_bishops | w_kingsafety_rooks));
                
                int b_pawn_defenders   = Bitboards::countBits(board.pieces[Pieces::B_PAWN]   & b_kingsafety_squares);
                int b_knight_defenders = Bitboards::countBits(board.pieces[Pieces::B_KNIGHT] & b_kingsafety_knights);
                int b_bishop_defenders = Bitboards::countBits(board.pieces[Pieces::B_BISHOP] & b_kingsafety_bishops);
                int b_rook_defenders   = Bitboards::countBits(board.pieces[Pieces::B_ROOK]   & b_kingsafety_rooks);
                int b_queen_defenders  = Bitboards::countBits(board.pieces[Pieces::B_QUEEN]  & (b_kingsafety_bishops | b_kingsafety_rooks));

                Evaluation w_att_total = w_pawn_attackers   * PAWN_ATT   +
                                         w_knight_attackers * KNIGHT_ATT +
                                         w_bishop_attackers * BISHOP_ATT +
                                         w_rook_attackers   * ROOK_ATT   +
                                         w_queen_attackers  * QUEEN_ATT;

                Evaluation w_def_total = w_pawn_defenders   * PAWN_DEF   +
                                         w_knight_attackers * KNIGHT_DEF +
                                         w_bishop_attackers * BISHOP_DEF +
                                         w_rook_attackers   * ROOK_DEF   +
                                         w_queen_attackers  * QUEEN_DEF;

                Evaluation b_att_total = b_pawn_attackers   * PAWN_ATT   +
                                         b_knight_attackers * KNIGHT_ATT +
                                         b_bishop_attackers * BISHOP_ATT +
                                         b_rook_attackers   * ROOK_ATT   +
                                         b_queen_attackers  * QUEEN_ATT;

                Evaluation b_def_total = b_pawn_defenders   * PAWN_DEF   +
                                         b_knight_attackers * KNIGHT_DEF +
                                         b_bishop_attackers * BISHOP_DEF +
                                         b_rook_attackers   * ROOK_DEF   +
                                         b_queen_attackers  * QUEEN_DEF;

                // 0 if king is safe, the larger the more that king is in danger
                Evaluation w_king_danger = std::max<Evaluation>(b_att_total - w_def_total, 0);
                Evaluation b_king_danger = std::max<Evaluation>(w_att_total - b_def_total, 0);

                // bonus for king danger squared so adding a small bit more danger is much worse
                // positive if b_king_danger is greater than w_king_danger
                Evaluation eval = (
                    b_king_danger * b_king_danger -
                    w_king_danger * w_king_danger
                );

                // it matters a lot in the midgame, doesnt matter in the endgame
                return EvalByPhase(eval, 0);
            }
            
        }

        namespace Eval::Pawns{

            /// @brief gets the evaluation of passed pawns in a board
            /// @param board 
            /// @return 
            EvalByPhase getPassedPawnEval(Board& board){
                EvalByPhase eval = EvalByPhase();
    
                // find passed pawns and add evaluation based on how close they are to the other end, middle game gets some eval but less
                // loop through pawns, check above the pawn on the 2/3 adj files
                Bitboard w_pawns = board.pieces[Pieces::W_PAWN];
                Bitboard b_pawns = board.pieces[Pieces::B_PAWN];

                Bitboard w_pawn_stoppers = w_pawns; // squares that white pawns stop
                Bitboard b_pawn_stoppers = b_pawns; // squares that black pawns stop
                for (int i = 0; i < 6; i++) w_pawn_stoppers |= w_pawn_stoppers << 8; // get in front of white pawns
                for (int i = 0; i < 6; i++) b_pawn_stoppers |= b_pawn_stoppers >> 8; // get in front of black pawns

                w_pawn_stoppers |= Bitboards::getPawnAttacks<Colours::WHITE>(w_pawn_stoppers);
                b_pawn_stoppers |= Bitboards::getPawnAttacks<Colours::BLACK>(b_pawn_stoppers);

                w_pawns &= ~b_pawn_stoppers;
                b_pawns &= ~w_pawn_stoppers;

                while (w_pawns){
                    Square rank = Squares::getY(Bitboards::popNextSquare(w_pawns));
                    // add a bonus for the y position of the pawn
                    eval.midgame += rank * 7;
                    eval.endgame += rank * 10 + 30;
                }
                while (b_pawns){
                    Square rank = 7 - Squares::getY(Bitboards::popNextSquare(b_pawns));
                    // add a bonus for the y position of the pawn
                    eval.midgame -= rank * 7;
                    eval.endgame -= rank * 10 + 30;
                }

                return eval;
            }

            /// @brief gets the evaluation of isolated, doubled and doubled isolated pawns
            /// @param board 
            /// @return 
            EvalByPhase getPawnWeaknessesEval(Board& board){
                EvalByPhase eval = EvalByPhase();
    
                Bitboard w_pawns = board.pieces[Pieces::W_PAWN];
                Bitboard b_pawns = board.pieces[Pieces::B_PAWN];

                // files where theres a pawn
                Bitboard w_isolated_files = (
                    (w_pawns & Bitboards::FILEA_BB) ? Bitboards::FILEA_BB : 0 |
                    (w_pawns & Bitboards::FILEB_BB) ? Bitboards::FILEB_BB : 0 |
                    (w_pawns & Bitboards::FILEC_BB) ? Bitboards::FILEC_BB : 0 |
                    (w_pawns & Bitboards::FILED_BB) ? Bitboards::FILED_BB : 0 |
                    (w_pawns & Bitboards::FILEE_BB) ? Bitboards::FILEE_BB : 0 |
                    (w_pawns & Bitboards::FILEF_BB) ? Bitboards::FILEF_BB : 0 |
                    (w_pawns & Bitboards::FILEG_BB) ? Bitboards::FILEG_BB : 0 |
                    (w_pawns & Bitboards::FILEH_BB) ? Bitboards::FILEH_BB : 0
                );

                w_isolated_files &= (
                    Bitboards::shiftBB<Directions::WEST>(w_isolated_files) |
                    Bitboards::shiftBB<Directions::EAST>(w_isolated_files)
                );

                Bitboard b_isolated_files = (
                    (b_pawns & Bitboards::FILEA_BB) ? Bitboards::FILEA_BB : 0 |
                    (b_pawns & Bitboards::FILEB_BB) ? Bitboards::FILEB_BB : 0 |
                    (b_pawns & Bitboards::FILEC_BB) ? Bitboards::FILEC_BB : 0 |
                    (b_pawns & Bitboards::FILED_BB) ? Bitboards::FILED_BB : 0 |
                    (b_pawns & Bitboards::FILEE_BB) ? Bitboards::FILEE_BB : 0 |
                    (b_pawns & Bitboards::FILEF_BB) ? Bitboards::FILEF_BB : 0 |
                    (b_pawns & Bitboards::FILEG_BB) ? Bitboards::FILEG_BB : 0 |
                    (b_pawns & Bitboards::FILEH_BB) ? Bitboards::FILEH_BB : 0
                );

                b_isolated_files &= (
                    Bitboards::shiftBB<Directions::WEST>(b_isolated_files) |
                    Bitboards::shiftBB<Directions::EAST>(b_isolated_files)
                );

                // find isolated pawns
                Bitboard w_isolated = w_pawns & w_isolated_files;
                Bitboard b_isolated = b_pawns & b_isolated_files;

                // find double pawns
                Bitboard w_doubled_files = (
                    (Bitboards::countBits(w_pawns & Bitboards::FILEA_BB) >= 2) ? Bitboards::FILEA_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEB_BB) >= 2) ? Bitboards::FILEB_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEC_BB) >= 2) ? Bitboards::FILEC_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILED_BB) >= 2) ? Bitboards::FILED_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEE_BB) >= 2) ? Bitboards::FILEE_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEF_BB) >= 2) ? Bitboards::FILEF_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEG_BB) >= 2) ? Bitboards::FILEG_BB : 0 |
                    (Bitboards::countBits(w_pawns & Bitboards::FILEH_BB) >= 2) ? Bitboards::FILEH_BB : 0
                );

                Bitboard b_doubled_files = (
                    (Bitboards::countBits(b_pawns & Bitboards::FILEA_BB) >= 2) ? Bitboards::FILEA_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEB_BB) >= 2) ? Bitboards::FILEB_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEC_BB) >= 2) ? Bitboards::FILEC_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILED_BB) >= 2) ? Bitboards::FILED_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEE_BB) >= 2) ? Bitboards::FILEE_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEF_BB) >= 2) ? Bitboards::FILEF_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEG_BB) >= 2) ? Bitboards::FILEG_BB : 0 |
                    (Bitboards::countBits(b_pawns & Bitboards::FILEH_BB) >= 2) ? Bitboards::FILEH_BB : 0
                );

                Bitboard w_doubled = w_pawns & w_doubled_files;
                Bitboard b_doubled = b_pawns & b_doubled_files;


                // find double isolated pawns
                Bitboard w_double_iso = w_doubled & w_isolated;
                Bitboard b_double_iso = b_doubled & b_isolated;


                // add value for isolated pawns (value should be negative)
                eval.midgame += -6 * (Bitboards::countBits(w_isolated) - Bitboards::countBits(b_isolated));
                eval.endgame += -2 * (Bitboards::countBits(w_isolated) - Bitboards::countBits(b_isolated));
            
                // add value for doubled pawns (value should be negative)
                eval.midgame += -5 * (Bitboards::countBits(w_doubled) - Bitboards::countBits(b_doubled));
                eval.endgame += -8 * (Bitboards::countBits(w_doubled) - Bitboards::countBits(b_doubled));

                // add value for doubled isolated pawns (value should be negative, there will still be values for it being doubled and isolated so be careful)
                eval.midgame += -8  * (Bitboards::countBits(w_double_iso) - Bitboards::countBits(b_double_iso));
                eval.endgame += -14 * (Bitboards::countBits(w_double_iso) - Bitboards::countBits(b_double_iso));

                return eval;
            }

        }
    } // namespace Values

    namespace LMR{
        Depth reduction = 2;
        Depth min_depth = 3;
        int moves = 5;

        Depth getLMRDepth(Depth depth){
            return std::max(std::min(min_depth, depth),
                            (0 < 8) ?
                            (depth - reduction) :
                            std::max(8 - reduction, (depth - 2 * reduction)));
        }
    } // namespace LMR

} // namespace Chess::Engine

#endif