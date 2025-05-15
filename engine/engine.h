
#ifndef ENGINE_H
#define ENGINE_H

#include <algorithm>
#include <array>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

#include "..\logger.h"
#include "..\chess\types.h"
#include "engine_types.h"
#include "..\transposition_table\tt.h"
#include "..\chess\board.h"
#include "..\chess\move_generator.h"

namespace Chess{

    namespace Engine{

        class Engine{

            private:
                MoveGeneration move_gen;
                uint64_t nodes_searched;
                uint64_t ab_prune_count;
                uint64_t tt_use_count;

            public:
                std::atomic_bool stop_search;
                
            public:
                void go(Board& board, SearchLimit search_limits){

                    // if theres only 1 legal move, return that legal move (and is playing a game)
                    Move best_move = 0;
                    if (search_limits.wtime || search_limits.btime || search_limits.winc || search_limits.binc){
                        if (move_gen.getLegalMoves(board).count == 1) best_move = move_gen.getLegalMoves(board)[0];
                    }
                    if (best_move == 0) best_move = getBestMove(board, search_limits, true);


                    Logger::log("bestmove " + Chess::Visuals::moveToString(best_move) + "\n");
                    std::cout << "bestmove " << Chess::Visuals::moveToString(best_move) << '\n';
                }

                Move getBestMove(Board& board, SearchLimit search_limits, bool print = false){

                    if (board.turn == Colours::WHITE){
                        return getBestMove<Colours::WHITE>(board, search_limits, print);
                    }
                    return getBestMove<Colours::BLACK>(board, search_limits, print);                    
                }

                template<Colour turn>
                Move getBestMove(Board& board, SearchLimit search_limits, bool print = false){

                    stop_search = false;

                    // start the stop search timer
                    std::thread timer_thread(&Engine::startSearchTimer, this, getSearchTime<turn>(search_limits));

                    auto start = std::chrono::high_resolution_clock::now();

                    nodes_searched = 0;
                    ab_prune_count = 0;
                    tt_use_count   = 0;

                    bool for_game = search_limits.wtime || search_limits.btime || search_limits.winc || search_limits.binc;
                    

                    LegalMovesArray moves = move_gen.generateLegalMoves<turn>(board);
                    sortMoves<true>(board, moves);
                    std::vector<MoveOrderingStruct> move_ordering;
                    move_ordering.resize(moves.count);
                    // add moves to move ordering
                    for (int i = 0; i < moves.count; i++) move_ordering[i].move = moves[i];

                    Evaluation alpha = -Evaluations::EVAL_MATE;
                    Evaluation beta  =  Evaluations::EVAL_MATE;
                    Evaluation b     =  Evaluations::EVAL_MATE;

                    Move best_move = moves[0];
                    Move best_move_this_iter = moves[0];

                    Depth max_depth = search_limits.depth ? search_limits.depth : 99;
                    Depth search_depth;

                    // loop through each move and negamax them
                    Move move;
                    bool is_capture;
                    bool in_ab_window;

                    // iterative deepening
                    for (Depth depth = 1; depth <= max_depth; depth++){

                        in_ab_window = false;

                        for (int i = 0; i < moves.count; i++){
                            move = moves[i];
                            // move_ordering[i].move = move;

                            is_capture = board.all_pieces & Bitboards::getBit(Moves::to(move));
                            
                            board.playMove(move);
    
                            // late move reduction
                            search_depth = (i < LMR::moves ||
                                            board.inCheck() ||
                                            is_capture)
                                            ? depth 
                                            : LMR::getLMRDepth(depth);
    
                            // PVS
                            // Evaluation eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);;
                            Evaluation eval;
                            if (i == 0){
                                // search with normal window
                                eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);
                            }
                            else{
                                // search with alpha - 1 to alpha window
                                eval = -negamax<!turn>(board, search_depth - 1, -alpha - 1, -alpha);
        
                                if (eval > alpha && beta - alpha > 1){
                                    eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);
                                }
                            }


                            board.undoMove();
    
                            // if stop search or is mate, return the best move
                            if (stop_search || Evaluations::is_win(eval)){
                                if (Evaluations::is_win(eval) && !stop_search){
                                    auto end = std::chrono::high_resolution_clock::now();
                                    std::chrono::duration<double> duration = end - start;
                                    double time_taken = duration.count();
                                    alpha     = eval;
                                    best_move = move;
                                    std::string info_str = std::string("info") +
                                                           " depth "    + std::to_string(depth) +
                                                           " nodes "    + std::to_string(nodes_searched) +
                                                           " time "     + std::to_string((int)(time_taken * 1000)) +
                                                           " nps "      + std::to_string((int)(nodes_searched / time_taken)) +
                                                           " score cp " + std::to_string((int)Evaluations::normalise(alpha, turn)) +
                                                           " pv "       + getPrincipalVariation(board, best_move) +
                                                           '\n';
                                    Logger::log(info_str);
                                    if (print){
                                        std::cout << info_str;
                                    }
                                }
                                stopSearch(timer_thread);
                                return best_move;
                            }

                            // update alpha and best move
                            if (eval > alpha){
                                in_ab_window        = true;
                                alpha               = eval;
                                best_move_this_iter = move;
                                move_ordering[i].importance = eval;
                                move_ordering[i].raised_alpha = true;
                            }
                            else{
                                move_ordering[i].importance = eval;
                                move_ordering[i].raised_alpha = false;
                            }

                            // if (print){ (debugging only)
                            //     // print the search info for this depth
                            //     std::cout << "info";
                            //     std::cout << " depth "    << depth;
                            //     std::cout << " nodes "    << nodes_searched;
                            //     std::cout << " score cp " << (int)alpha;
                            //     std::cout << " pv "       << getPrincipalVariation(board, move);
                            //     std::cout << '\n';
                            //     // std::cout << "extra info";
                            //     // std::cout << " ab prune " << ab_prune_count;
                            //     // std::cout << " tt usage " << tt_use_count << '\n';
                            // }
                            
                            b = alpha + 1;
                        }
    
                        // sort moves
                        std::sort(move_ordering.begin(), move_ordering.end(), [](const MoveOrderingStruct& a, const MoveOrderingStruct& b) {return ((a.raised_alpha != b.raised_alpha) && a.raised_alpha) || (a.importance > b.importance);});
                        
                        // // // change the moves array
                        for (int i = 0; i < moves.count; i++){
                            moves.moves[i] = move_ordering[i].move;
                        }
                        
                        best_move = best_move_this_iter; // update best move
                        
                        TT::save(board.hash, alpha, depth, best_move, Values::NodeTypes::EXACT);

                        auto end = std::chrono::high_resolution_clock::now();
                        
                        std::chrono::duration<double> duration = end - start;
                        double time_taken = duration.count();

                        std::string info_str = std::string("info") +
                                               " depth "    + std::to_string(depth) +
                                               " nodes "    + std::to_string(nodes_searched) +
                                               " time "     + std::to_string((int)(time_taken * 1000)) +
                                               " nps "      + std::to_string((int)(nodes_searched / time_taken)) +
                                               " score cp " + std::to_string((int)Evaluations::normalise(alpha, turn)) +
                                               " pv "       + getPrincipalVariation(board, best_move) +
                                               '\n';
                        Logger::log(info_str);

                        if (print){
                            // print the search info for this depth
                            std::cout << info_str;
                        }

                        // decrease alpha for the next depth
                        if (in_ab_window && false){
                            alpha -= 40;
                        }
                        else{
                            alpha -= 1000;
                        }
                        
                        // if the best move is much better than other moves at a reasonable depth and for_game is true, return the best move
                        if (move_ordering[0].importance - 500 > move_ordering[1].importance && depth > 8 && for_game){
                            stopSearch(timer_thread);
                            return best_move;
                        }


                        
                    }

                    stopSearch(timer_thread);
                    return best_move;

                }

                template<Colour turn>
                Evaluation negamax(Board& board, Depth depth, Evaluation alpha, Evaluation beta){
                    if (stop_search){
                        return 0;
                    }

                    if (board.insufficientMaterial()){
                        return 0;
                    }

                    if (board.repeated()){
                        return 0;
                    }

                    
                    if (depth <= 0){
                        Evaluation eval = qsearch<turn>(board, alpha, beta);
                        // TT::save(board.hash, alpha, depth, 0);
                        return eval;
                    }

                    LegalMovesArray moves = move_gen.generateLegalMoves<turn>(board);
                    // check transposition table
                    {
                        TT::TTData entry = TT::getEntry(board.hash);
                        if (entry.hash == board.hash && entry.depth >= depth){
                            bool legal = false;
                            if (entry.move){
                                // check if the saved move is legal
                                for (Move move : moves){
                                    if (entry.move == move) { legal = true; break; }
                                }
                            }
                            if (legal){
                                Evaluation adjusted_eval = entry.eval;
                                if (Evaluations::is_win (adjusted_eval)) adjusted_eval = std::max(adjusted_eval - depth, Evaluations::EVAL_MATE_IN_MAX_PLY);
                                if (Evaluations::is_loss(adjusted_eval)) adjusted_eval = std::min(adjusted_eval + depth, Evaluations::EVAL_MATED_IN_MAX_PLY);
                                if (entry.node_type == Values::NodeTypes::EXACT) {
                                    return adjusted_eval;
                                }
                                else if (entry.node_type == Values::NodeTypes::FAIL_LOW && adjusted_eval <= alpha) {
                                    return adjusted_eval;
                                }
                                else if (entry.node_type == Values::NodeTypes::FAIL_HIGH && adjusted_eval >= beta) {
                                    return adjusted_eval;
                                }
                            }
                        }
                    }

                    bool in_check = board.inCheck<turn>();
                    { // null move pruning
                        if (!in_check){
                            Square ep = board.playNull();
                            Evaluation eval = -negamax<!turn>(board, depth - Values::Negamax::NULL_MOVE_PRUNE_REDUCTION, -beta, -(beta - 1));
                            board.undoNull(ep);
                            if (eval >= beta){
                                return eval;
                            }
                        }
                    }

                    if (moves.count == 0){ // mate and stalemate cases
                        if (board.inCheck()){
                            return -Evaluations::getMateEval(depth);
                        }
                        return 0;
                    }

                    { // sort moves
                        if (depth > 3){
                            sortMoves<false>(board, moves, alpha, beta);
                        }
                        else{
                            sortMoves<true> (board, moves, alpha, beta);
                        }
                    }

                    // loop through each move and negamax them
                    Evaluation start_alpha = alpha;
                    Depth search_depth;
                    Move move;
                    Move best_move = 0;
                    bool is_capture;
                    for (int i = 0; i < moves.count; i++){
                        move = moves[i];

                        is_capture = board.all_pieces & Bitboards::getBit(Moves::to(move));

                        // play move
                        board.playMove(move);

                        // late move reduction - if one of the first 4 moves, is in check, captures a piece search at the normal depth
                        {
                            search_depth = (i < LMR::moves ||
                                            in_check ||
                                            board.inCheck() ||
                                            is_capture)
                                            ? depth 
                                            : LMR::getLMRDepth(depth);
                        }

                        // PVS
                        // Evaluation eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);
                        Evaluation eval;
                        if (i == 0){
                            // search with normal window
                            eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);
                        }
                        else{
                            // search with alpha - 1 to alpha window
                            eval = -negamax<!turn>(board, search_depth - 1, -alpha - 1, -alpha);
    
                            if (eval > alpha && beta - alpha > 1){
                                eval = -negamax<!turn>(board, search_depth - 1, -beta, -alpha);
                            }
                        }

                        // undo move
                        board.undoMove();
                        
                        if (eval > alpha){best_move = move;}
                        // update alpha
                        alpha = std::max<Evaluation>(eval, alpha);

                        if (alpha >= beta){
                            ab_prune_count++;
                            TT::save(board.hash, alpha, depth, best_move, Values::NodeTypes::FAIL_HIGH);
                            return alpha; // alpha-beta pruning
                        }
                    }

                    if (alpha >= beta) {
                        TT::save(board.hash, alpha, depth, best_move, Values::NodeTypes::FAIL_HIGH);
                    } else if (alpha > start_alpha) {
                        TT::save(board.hash, alpha, depth, best_move, Values::NodeTypes::EXACT);
                    } else {
                        TT::save(board.hash, alpha, depth, moves[0], Values::NodeTypes::FAIL_LOW);
                    }

                    return alpha;
                }


                template<Colour turn>
                Evaluation qsearch(Board& board, Evaluation alpha, Evaluation beta){
                    if (stop_search){
                        return 0;
                    }

                    if (board.insufficientMaterial()){
                        return 0;
                    }

                    if (board.repeated()){
                        return 0;
                    }

                    if (board.isCheckmate<turn>()){
                        return Evaluations::EVAL_MATE_IN_MAX_PLY;
                    }

                    Evaluation standing_pat = staticEvaluate<turn>(board);

                    if (standing_pat >= beta){
                        return beta;
                    }

                    alpha = std::max<Evaluation>(alpha, standing_pat);

                    LegalMovesArray moves = move_gen.generateTacticalMoves<turn>(board); // gets capture moves
                    sortMoves<true>(board, moves, alpha, beta);

                    // loop through each move and negamax them
                    bool first_move = true;
                    for (Move move : moves){
                        board.playMove(move);

                        // negamax
                        // Evaluation eval = -qsearch<!turn>(board, -beta, -alpha);
                        // PVS
                        Evaluation eval;
                        if (first_move){
                            // search with normal window
                            eval = -qsearch<!turn>(board, -beta, -alpha);
                        }
                        else{
                            // search with alpha - 1 to alpha window
                            eval = -qsearch<!turn>(board, -alpha - 1, -alpha);
    
                            if (eval > alpha && beta - alpha > 1){
                                eval = -qsearch<!turn>(board, -beta, -alpha);
                            }
                        }

                        board.undoMove();

                        alpha = std::max<Evaluation>(eval, alpha);

                        if (alpha >= beta){
                            ab_prune_count++;
                            return beta; // alpha-beta pruning
                        }

                        first_move = false;
                    }

                    return alpha;
                }


                // Evaluation see(Board& board, Square square){
                //     Bitboard below_square = Bitboards::getBit(square) - 1;

                //     // get pieces on both sides which are attacking or xraying the square
                //     Bitboard w_pawns   = board.pieces[Pieces::W_PAWN]   &  Bitboards::getPawnAttacks<Colours::BLACK>(Bitboards::getBit(square));
                //     Bitboard b_pawns   = board.pieces[Pieces::B_PAWN]   &  Bitboards::getPawnAttacks<Colours::WHITE>(Bitboards::getBit(square));

                //     // get pieces that arent truly blockers
                //     Bitboard diag_non_blockers = board.pieces[Pieces::W_BISHOP] | board.pieces[Pieces::B_BISHOP] | board.pieces[Pieces::W_QUEEN] | board.pieces[Pieces::B_QUEEN] | w_pawns | b_pawns;
                //     Bitboard hori_non_blockers = board.pieces[Pieces::W_ROOK]   | board.pieces[Pieces::B_ROOK]   | board.pieces[Pieces::W_QUEEN] | board.pieces[Pieces::B_QUEEN];

                //     Bitboard bishop_attacks = Bitboards::getBishopAttacks(board.all_pieces & ~diag_non_blockers, square);
                //     Bitboard rook_attacks   = Bitboards::getRookAttacks  (board.all_pieces & ~hori_non_blockers, square);


                //     // ignore pieces that are already on the square
                //     Bitboard w_knights = board.pieces[Pieces::W_KNIGHT] &  Bitboards::getKnightAttacks(square);
                //     Bitboard w_bishops = board.pieces[Pieces::W_BISHOP] &  bishop_attacks;
                //     Bitboard w_rooks   = board.pieces[Pieces::W_ROOK]   &  rook_attacks;
                //     Bitboard w_queens  = board.pieces[Pieces::W_QUEEN]  & (bishop_attacks | rook_attacks);
                //     Bitboard w_kings   = board.pieces[Pieces::W_KING]   &  Bitboards::getKingAttacks(square);


                //     Bitboard b_knights = board.pieces[Pieces::B_KNIGHT] &  Bitboards::getKnightAttacks(square);
                //     Bitboard b_bishops = board.pieces[Pieces::B_BISHOP] &  bishop_attacks;
                //     Bitboard b_rooks   = board.pieces[Pieces::B_ROOK]   &  rook_attacks;
                //     Bitboard b_queens  = board.pieces[Pieces::B_QUEEN]  & (bishop_attacks | rook_attacks);
                //     Bitboard b_kings   = board.pieces[Pieces::B_KING]   &  Bitboards::getKingAttacks(square);

                //     Evaluation eval = 0;

                //     // calculate the see eval
                //     Colour turn = board.turn;
                //     Piece smallest_attacker = 0;
                //     while (true){
                //         if (turn == Colours::WHITE){
                //             // get the smallest attacker
                //             if (w_pawns){
                //                 smallest_attacker = Pieces::W_PAWN;
                //                 w_pawns &= w_pawns - 1;
                //             }
                //             if (smallest_attacker == Pieces::EMPTY && w_knights){
                //                 smallest_attacker = Pieces::W_KNIGHT;
                //                 w_knights &= w_knights - 1;
                //             }
                //             if (smallest_attacker == Pieces::EMPTY && w_bishops){
                //                 // make sure theres no blockers between this and the square (our queen, enemy_bishops, enemy_queen)
                //                 Bitboard below_bishop = Bitboards::getNextBit(w_bishops) - 1;

                //                 // fast check
                //                 Bitboard queens_between;
                //                 if (Bitboards::getNextSquare(w_rooks) > square){
                //                     queens_between = below_bishop & ~below_square & bishop_attacks & w_queens;
                //                 }
                //                 if (Bitboards::getNextSquare(w_rooks) < square){
                //                     queens_between = below_square & ~below_bishop & bishop_attacks & w_queens;
                //                 }
                //                 // make sure its not a queen on the diagonal the bishop isnt on
                //                 if (queens_between){
                //                     queens_between &= Bitboards::bitboardThrough(square, Bitboards::getNextSquare(w_bishops));
                //                 }

                //                 if (!queens_between){
                //                     smallest_attacker = Pieces::W_BISHOP;
                //                     w_bishops &= w_bishops - 1;
                //                 }
                //             }
                //             if (smallest_attacker == Pieces::EMPTY && w_rooks){
                //                 // make sure theres no queen between this and the square
                //                 Bitboard below_rook = Bitboards::getNextBit(w_rooks) - 1;
                //                 // if the square is below the rook

                //                 // fast check
                //                 Bitboard queens_between;
                //                 if (Bitboards::getNextSquare(w_rooks) > square){
                //                     queens_between = below_rook & ~below_square & rook_attacks & w_queens;
                //                 }
                //                 if (Bitboards::getNextSquare(w_rooks) < square){
                //                     queens_between = below_square & ~below_rook & rook_attacks & w_queens;
                //                 }
                //                 // make sure its not a queen on the horizontal/verticle the rook isnt on
                //                 if (queens_between){
                //                     queens_between &= Bitboards::bitboardThrough(square, Bitboards::getNextSquare(w_rooks));
                //                 }

                //                 if (!queens_between){
                //                     smallest_attacker = Pieces::W_ROOK;
                //                     w_rooks &= w_rooks - 1;
                //                 }
                //             }
                //             if (smallest_attacker == Pieces::EMPTY && w_queens){
                //                 // should be possible as rooks bishops and pawns (only blockers) arent possible
                //                 smallest_attacker = Pieces::W_QUEEN;
                //                 w_queens &= w_queens - 1;
                //             }
                //             if (smallest_attacker == Pieces::EMPTY && w_kings){
                //                 // check if its actually legal. if not ignore this
                //                 smallest_attacker = Pieces::W_KING;
                //                 w_kings &= w_kings - 1;
                //             }
                //         }
                //         else{

                //         }
                //     }
                // }


                /// @brief returns the evaluation of the board without playing any moves
                /// @param board 
                /// @return 
                template<Colour turn>
                Evaluation staticEvaluate(Board& board){
                    nodes_searched++;

                    // EvalByPhase eval = EvalByPhase();

                    EvalByPhase eval = Values::Eval::getMaterialEval(board);

                    eval += Values::Eval::PSQT::addPieceSquareTables(board);

                    eval += Values::Eval::Mobility::getMobilityEval(board) * 0.3;

                    eval += Values::Eval::Pawns::getPassedPawnEval(board);

                    eval += Values::Eval::Pawns::getPawnWeaknessesEval(board);

                    eval += Values::Eval::KingSafety::getKingSafetyEval(board);

                    // slight bonus for whos turn it is
                    if (turn == Colours::WHITE) eval += EvalByPhase(Values::Eval::MG_TURN_BONUS, Values::Eval::EG_TURN_BONUS);
                    else                        eval -= EvalByPhase(Values::Eval::MG_TURN_BONUS, Values::Eval::EG_TURN_BONUS);
                    
                    // bonus for castling rights in middle game
                    eval.midgame += Values::Eval::MG_CASTLING_BONUS * (__builtin_popcount(board.castling_rights[Colours::WHITE]) - __builtin_popcount(board.castling_rights[Colours::BLACK]));

                    
                    Evaluation evaluation = eval.getEval(getGamePhase<true>(board));
                        
                    if constexpr (turn == Colours::BLACK) evaluation = -evaluation;
                    return evaluation;
                }

                /// @brief returns the evaluation of the board without playing any moves or calculating many moves each side can make
                /// @param board 
                /// @return 
                template<Colour turn>
                Evaluation fastStaticEvaluate(Board& board){
                    EvalByPhase eval = Values::Eval::getMaterialEval(board);

                    eval += Values::Eval::PSQT::addPieceSquareTables(board);

                    eval += Values::Eval::Pawns::getPassedPawnEval(board);

                    eval += Values::Eval::Pawns::getPawnWeaknessesEval(board);

                    // slight bonus for whos turn it is
                    if (turn == Colours::WHITE) eval += EvalByPhase(Values::Eval::MG_TURN_BONUS, Values::Eval::EG_TURN_BONUS);
                    else                        eval -= EvalByPhase(Values::Eval::MG_TURN_BONUS, Values::Eval::EG_TURN_BONUS);
                    
                    // bonus for castling rights in middle game
                    eval.midgame += Values::Eval::MG_CASTLING_BONUS * (__builtin_popcount(board.castling_rights[Colours::WHITE]) - __builtin_popcount(board.castling_rights[Colours::BLACK]));

                    
                    Evaluation evaluation = eval.getEval(getGamePhase<true>(board));
                        
                    if constexpr (turn == Colours::BLACK) evaluation = -evaluation;
                    return evaluation;
                }

                template<bool fast = true>
                Evaluation getMoveImportance(Board& board, Move move, Evaluation alpha, Evaluation beta, Bitboard defended_squares){
                    if constexpr (fast){
                        board.playMove(move);

                        Evaluation importance = (board.turn == Colours::BLACK) ?
                                                 fastStaticEvaluate<Colours::WHITE>(board) :
                                                 fastStaticEvaluate<Colours::BLACK>(board);
                        
                        board.undoMove();

                        if (Bitboards::getBit(Moves::to(move)) & defended_squares){
                            double phase = getGamePhase<true>(board);
                            Piece p = board.getPieceAt(Moves::to(move));
                            importance -= (Values::Eval::PIECE_MG_VALUES[p] - Values::Eval::PIECE_EG_VALUES[p]) * phase + Values::Eval::PIECE_EG_VALUES[p];
                        }

                        return importance;
                    }
                    else{
                        board.playMove(move);

                        // check transposition table
                        {
                            TT::TTData entry = TT::getEntry(board.hash);
                            if (entry.hash == board.hash){
                                board.undoMove();
                                return entry.eval;
                            }
                        }

                        Evaluation eval;
                        if (board.turn == Colours::WHITE) { eval = -qsearch<Colours::WHITE>(board, -beta - 50, -alpha + 50); }
                        else                              { eval = -qsearch<Colours::BLACK>(board, -beta - 50, -alpha + 50); }
                        board.undoMove();
                        return eval;
                    }
                }

                template<bool fast = true>
                void sortMoves(Board& board, LegalMovesArray& moves, Evaluation alpha = 0, Evaluation beta = 0){

                    Bitboard defended_squares = board.turn == Colours::WHITE ? board.attackedBitboardNoKing<Colours::BLACK>() : board.attackedBitboardNoKing<Colours::WHITE>();
                    std::vector<MoveOrderingStruct> move_ordering;
                    move_ordering.reserve(moves.count);

                    Move best_move = 0;
                    if constexpr (!fast){
                        // check transposition table
                        TT::TTData entry = TT::getEntry(board.hash);
                        if (entry.hash == board.hash){
                            best_move = entry.move;
                        }
                    }

                    for (int i = 0; i < moves.count; i++){
                        if constexpr (!fast){
                            if (best_move == moves[i]){move_ordering.emplace_back(moves[i], Evaluations::EVAL_INF); continue;}
                        }
                        move_ordering.emplace_back(moves[i], getMoveImportance<fast>(board, moves[i], alpha, beta, defended_squares));
                    }
                    std::sort(move_ordering.begin(), move_ordering.end(), [](const MoveOrderingStruct& a, const MoveOrderingStruct& b) {return a.importance > b.importance;});
                
                    // change the moves array
                    for (int i = 0; i < moves.count; i++){
                        moves.moves[i] = move_ordering[i].move;
                    }
                }
        
                template<bool normalised>
                double getGamePhase(Board& board){
                    double piece_count = std::min(24, 1 * Bitboards::countBits(board.pieces[Pieces::W_KNIGHT] | board.pieces[Pieces::B_KNIGHT] | board.pieces[Pieces::W_BISHOP] | board.pieces[Pieces::B_BISHOP]) +
                                                      2 * Bitboards::countBits(board.pieces[Pieces::W_ROOK]   | board.pieces[Pieces::B_ROOK]) +
                                                      4 * Bitboards::countBits(board.pieces[Pieces::W_QUEEN]  | board.pieces[Pieces::B_QUEEN]));
                    if constexpr (normalised){
                        return piece_count / 24;
                    }
                    return piece_count;
                }


                std::string getPrincipalVariation(Board& board, Move move){
                    std::string r = "";

                    int i = 0;
                    while (i < 20){
                        r += Visuals::moveToString(move);
                        r += " ";
                        i++;
                        board.playMove(move);
                        TT::TTData entry = TT::getEntry(board.hash);
                        if (entry.hash == board.hash){
                            move = entry.move;
                        }
                        if (move_gen.isLegalMove(board, move)){
                            continue;
                        }
                        break;
                    }

                    for (int j = 0; j < i; j++){
                        board.undoMove();
                    }

                    return r;
                }


                /// @brief waits some time then sets stop search to true
                /// @param time 
                void startSearchTimer(int milliseconds){
                    // not a good way of doing it but should work
                    auto start = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(milliseconds);
                    while (!stop_search){
                        auto end = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> d = end - start;
                        if (d.count() >= 0){
                            break;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                    stop_search = true;
                }

                /// @brief stops the current search
                void stopSearch(std::thread& timer_thread){
                    stop_search = true;
                    if (timer_thread.joinable()) timer_thread.join();
                }

                /// @brief gets the time (milliseconds) that the search should go on for
                /// @param s 
                /// @return 
                template<Colour c>
                int getSearchTime(SearchLimit s){
                    int t = 0;
                    if (s.time) t = s.time;
                    // if using wtime, winc / btime, binc
                    // the time used is 1/20 of the time left + the inc it has
                    // if the above is less than the time it has left, use the time it has left

                    // subtract lichess ping from the time it has and the time it uses
                    // subtracting from the time it has simulates the time it takes to reach this point
                    // subtracting from the time it uses simulates the time it takes to get to lichess/output
                    else if (c == Colours::WHITE) t = getSearchTime(s.wtime, s.winc);
                    else if (c == Colours::BLACK) t = getSearchTime(s.btime, s.binc);
                    // if theres either a s.wtime s.btime s.winc or s.binc then make it at least 1 ms
                    if (s.wtime || s.btime || s.winc || s.binc) t = std::max(1, t);
                    if (!t) return 3600000; // if no time is set, cap it to an hour
                    return t;
                }

                int getSearchTime(int time, int inc){

                    // simulate the time it takes to reach this point from when lichess changed turns
                    time -= Lichess::ping;

                    int t = 0;

                    if (time < 10000){
                        t = inc + 100; // if below 10 seconds, make it use ~100ms a turn
                    }
                    else{
                        t = inc + time / 20; // if above 10s, make it use 1/20th of its time remaining
                    }

                    // cap the time used to time so it doesnt flag
                    t = std::min(time, t);

                    // simulate the time it will take for it to go from here to lichess
                    t -= Lichess::ping;

                    // make the time at least 0s
                    t = std::max(0, t);

                    return t;
                }

            };

    } // namespace Engine

} // namespace Chess



#endif