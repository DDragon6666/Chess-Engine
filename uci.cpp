

#include <iostream>
#include <sstream>
#include <chrono>
#include <functional>

#include "chess/board.h"
#include "engine/engine.h"
#include "engine/test_positions.h"


void loop(){
    std::string token;
    std::string cmd;
    Chess::Board board;
    Chess::MoveGeneration  move_generator;
    Chess::LegalMovesArray legal_moves;
    Chess::Engine::Engine  engine;

    while (true){
        std::getline(std::cin, cmd);
        if (cmd == "quit"){
            // engine.stop_search = true;
            // t.join();
            return;
        }

        else if (cmd == "uci"){
            std::cout << "id name DDragon_Engine\n";
            std::cout << "id author DDragon\n";
            std::cout << "uciok\n";
        }

        else if (cmd == "isready"){
            std::cout << "readyok\n";
        }

        else if (cmd == "draw"){
            board.printBoard();
        }

        else if (cmd == "debugboard"){
            board.debug(3);
            std::cout << engine.getGamePhase<true>(board) << " - " << engine.getGamePhase<false>(board);
            std::cout << '\n';
            // if (Chess::Engine::TT::table[Chess::Engine::TT::getIndex(board.hash)].equalsHash(board.hash)){
            //     std::cout << "TT eval: " << Chess::Engine::TT::table[Chess::Engine::TT::getIndex(board.hash)].eval << '\n';
            // }
        }

        else if (cmd.substr(0, 8) == "playmove"){
            std::istringstream iss(cmd);
            std::string token;
            iss >> token;
            iss >> token;

            if (token == "undo"){
                board.undoMove();
            }
            else{
                Chess::Move move = Chess::Visuals::stringToMove(token, board);
                if (move_generator.isLegalMove(board, move)){
                    board.playMove(move);
                }
                else{
                    std::cout << "Move: (" << token << ") is illegal\n";
                }
            }
        }

        else if (cmd.substr(0, 8) == "position"){
            std::istringstream iss(cmd);
            std::string token;
            iss >> token;
            iss >> token;
            if (token == "startpos"){
                board.setStartingBoard();
                iss >> token;
            }
            else if (token == "fen"){
                std::string fen;
                while (iss >> token && token != "moves") {
                    fen += token + " ";
                }
                std::cout << "fen: " << fen << "\n";
                board.setFen(fen);
            }
            else{
                std::cout << "invalid command\n";
                continue;
            }

            if (token == "moves") {
                // Apply the moves
                while (iss >> token) {
                    Chess::Move move = Chess::Visuals::stringToMove(token, board);
                    board.playMove(move);
                }
            }
        }

        else if (cmd.substr(0, 2) == "go"){

            Chess::Engine::SearchLimit search_limit = Chess::Engine::Values::NO_LIMIT;
            std::istringstream iss(cmd);
            iss >> token;
            while (iss >> token) {
                // add limits
                if (token == "time"){
                    iss >> search_limit.time;
                }
                else if (token == "depth"){
                    iss >> search_limit.depth;
                }
                else if (token == "wtime"){
                    iss >> search_limit.wtime;
                }
                else if (token == "btime"){
                    iss >> search_limit.btime;
                }
                else if (token == "winc"){
                    iss >> search_limit.winc;
                }
                else if (token == "binc"){
                    iss >> search_limit.binc;
                }
            }

            engine.stop_search = true;
            // t.join();

            if (search_limit == Chess::Engine::Values::NO_LIMIT){
                search_limit = Chess::Engine::Values::DEFAULT_LIMIT;
            }

            engine.go(board, search_limit);

            // t = std::thread(&Chess::Engine::Engine::go, &engine, std::ref(board), search_limit);


        }

        else if (cmd == "moves"){
            Chess::LegalMovesArray legal_moves = move_generator.getLegalMoves(board);
            engine.sortMoves<false>(board, legal_moves, -Chess::Engine::Evaluations::EVAL_INF, Chess::Engine::Evaluations::EVAL_INF);
            std::cout << "Legal moves:\n";
            for (int i = 0; i < legal_moves.count; i++)
            {
                std::cout << Chess::Visuals::moveToString(legal_moves.moves[i]) << "\n";
            }
        }

        else if (cmd == "tacticalmoves"){
            Chess::LegalMovesArray legal_moves = move_generator.getTacticalMoves(board);
            std::cout << "Legal moves:\n";
            for (int i = 0; i < legal_moves.count; i++)
            {
                std::cout << Chess::Visuals::moveToString(legal_moves.moves[i]) << "\n";
            }
        }

        else if (cmd.substr(0, 10) == "enginetest"){
            // enginetest -l short -s time 2000 -c 4 (~0.925)
            // enginetest -l long -s time 2000 -c 4

            std::istringstream iss(cmd);
            iss >> token;

            std::vector<std::string> puzzle_types;
            std::vector<std::string> puzzle_lengths;
            int count = 4;
            int print = 1;
            Chess::Engine::SearchLimit search_limit = Chess::Engine::Values::NO_LIMIT;

            // loop through iss. If the first char is '-' change the vector that gets changed
            int arg_type = 0; // 1 = 
            while (iss >> token){
                if (token[0] == '-'){
                    if (token == "-t" || token == "-types"){
                        arg_type = 1;
                        continue;
                    }
                    else if (token == "-l" || token == "-lengths"){
                        arg_type = 2;
                        continue;
                    }
                    else if (token == "-c" || token == "-count"){
                        arg_type = 3;
                        continue;
                    }
                    else if (token == "-s" || token == "-limit" || token == "-searchlimit"){
                        arg_type = 4;
                        continue;
                    }
                    else if (token == "-p" || token == "-print"){
                        arg_type = 5;
                        continue;
                    }
                }
                if (arg_type == 1){
                    puzzle_types.emplace_back(token);
                }
                if (arg_type == 2){
                    puzzle_lengths.emplace_back(token);
                }
                if (arg_type == 3){
                    count = stoi(token);
                }
                if (arg_type == 4){
                    if (token == "time"){
                        iss >> search_limit.time;
                    }
                    else if (token == "depth"){
                        iss >> search_limit.depth;
                    }
                    else if (token == "wtime"){
                        iss >> search_limit.wtime;
                    }
                    else if (token == "btime"){
                        iss >> search_limit.btime;
                    }
                    else if (token == "winc"){
                        iss >> search_limit.winc;
                    }
                    else if (token == "binc"){
                        iss >> search_limit.binc;
                    }
                }
                if (arg_type == 5){
                    print = stoi(token);
                }
            }

            if (puzzle_types.size() == 0){
                puzzle_types.emplace_back("all");
            }
            if (puzzle_lengths.size() == 0){
                puzzle_lengths.emplace_back("all");
            }

            if (search_limit == Chess::Engine::Values::NO_LIMIT){
                search_limit = Chess::Engine::Values::DEFAULT_LIMIT;
            }

            Chess::Engine::Test::doTest(puzzle_types, puzzle_lengths, count, search_limit, print);
            std::cout << "Done\n";
        }

        else if (cmd == "ucinewgame") {
            board.setStartingBoard();
            Chess::Engine::TT::clear();
        }

        else if (cmd == "ttcap") {
            std::cout << "Filled " << Chess::Engine::TT::filled << '/' << Chess::Engine::TT::table.size() <<
                         " (" << ((double(Chess::Engine::TT::filled))/(double(Chess::Engine::TT::table.size()))) << "%)\n";
        }

        else if (cmd == "eval") {
            engine.printStaticEvaluation(board);
        }

        // else if (cmd == "stop") {
        //     engine.stop_search = true;
        //     t.join();
        // }

        else if (cmd == "setoption name Clear Hash"){
            Chess::Engine::TT::clear();
        }

        else if (cmd.substr(0, 9) == "setoption") {
            std::istringstream iss(cmd);

            std::string name, value;
            while (iss >> token) {
                if (token == "name") {
                    name.clear();
                    while (iss >> token && token != "value") {
                        name += token + " ";
                    }
                    name = name.substr(0, name.length() - 1); // trim trailing space
                }

                if (token == "value") {
                    iss >> value;
                }
            }

            if (name == "Hash") {
                int mb = std::stoi(value);
                Chess::Engine::TT::resize(mb);
            }
        }


        else{
            std::cout << "The command '" << cmd << "' is not recognised\n";
        }
        
        

    }

}





int main(){
    Chess::Zobrist::initialise();
    // Chess::Engine::TT::clear();

    std::cout << sizeof(Chess::Engine::TT::table) << '\n';
    std::cout << "Start\n";

    loop();

}