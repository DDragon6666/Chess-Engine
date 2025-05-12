
#ifndef CHESS_POSITIONS_H
#define CHESS_POSITIONS_H

#include <cstdint>
#include <array>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "..\chess\types.h"
#include "..\chess\board.h"
#include "..\chess\move_generator.h"
#include "..\random.h"
#include "engine.h"


namespace Chess{

    namespace Engine::Test{

        std::vector<std::string> loadFromFile(std::string file_name, int count){
            std::ifstream load_file(file_name);
            std::vector<std::string> puzzles;

            if (!load_file) {
                load_file.close();
                // std::cerr << "Error: Could not open file " << file_name << std::endl;
                return puzzles;
            }
            
            int line_count = 0;
            std::string line;
            
            while (std::getline(load_file, line)) {
                line_count++;
            }

            // go to the beginning of the file
            load_file.clear();
            load_file.seekg(0);

            count = std::min(line_count, count);

            std::vector<int> puzzle_lines;
            for (int i = 0; i < count; i++){
                puzzle_lines.emplace_back(random<int>(0, line_count - count + 1));
            }
            std::sort(puzzle_lines.begin(), puzzle_lines.end());
            for (int i = 0; i < count; i++){
                puzzle_lines[i] += i;
            }


            int puzzle_number = 0;
            int puzzle_count = 0;

            bool found = false;

            while (std::getline(load_file, line) && (puzzle_lines[puzzle_lines.size() - 1] < line_count)){
                if (puzzle_number == puzzle_lines[puzzle_count]){
                    puzzle_count++;
                    puzzles.emplace_back(line);
                    if (puzzle_count == puzzle_lines.size()){
                        found = true;
                        break;
                    }
                }
                puzzle_number++;
            }

            if (!found){
                std::cout << "end search" << line_count << " - " << puzzle_lines[0] << "\n";
            }

            load_file.close();

            return puzzles;
        }

        /// @brief puzzle types are all the puzzle types that it will do puzzles from
        /// @param puzzle_types are the strings of puzzle types it will do. `all` does count from all types.
        /// @param lengths are the strings of lengths it will do (oneMove, short, long, veryLong, noLength). `all` will choose from all
        /// @param count is the amount of puzzles to do from each puzzle_type and length
        /// @param search_limit is the limit that the engine will search to
        /// @param print is whether it will print the results of each puzzle
        void doTest(std::vector<std::string>& puzzle_types, std::vector<std::string>& puzzle_lengths, int count, SearchLimit search_limit, int print){
            Board board;
            Engine engine;

            std::string puzzle_types_str = "";
            for (std::string t : puzzle_types){
                puzzle_types_str += t;
                puzzle_types_str += ' ';
            }

            std::string puzzle_lengths_str = "";
            for (std::string t : puzzle_lengths){
                puzzle_lengths_str += t;
                puzzle_lengths_str += ' ';
            }

            std::cout << "Types: " << puzzle_types_str << "| Lengths: " << puzzle_lengths_str << "| Count: " << count << " | SearchLimit: " << search_limit << '\n';

            // if its correct:
            //     leave a gap before it prints (makes it easier to see wrong positions)
            //     best_move == sf move
            //     fen

            // if its incorrect:
            //     print X
            //     best_move != sf move
            //     fen

            for (std::string t : puzzle_types){
                if (t == "all"){
                    puzzle_types = {
                        "deflection",
                        "capturingDefender",
                        "pin",
                        "advancedPawn",
                        "mate",
                        "exposedKing",
                        "attackingF2F7",
                        "crushing",
                        "queenEndgame",
                        "knightEndgame",
                        "interference",
                        "endgame",
                        "opening",
                        "equality",
                        "rookEndgame",
                        "sacrifice",
                        "queensideAttack",
                        "hangingPiece",
                        "fork",
                        "bishopEndgame",
                        "kingsideAttack",
                        "zugzwang",
                        "enPassant",
                        "middlegame",
                        "underPromotion",
                        "xRayAttack",
                        "intermezzo",
                        "pawnEndgame",
                        "attraction",
                        "quietMove",
                        "doubleCheck",
                        "skewer",
                        "trappedPiece",
                        "defensiveMove",
                        "queenRookEndgame",
                        "clearance",
                        "castling",
                        "discoveredAttack",
                        "advantage",
                        "promotion"
                    };
                    break;
                }
            }

            
            for (std::string t : puzzle_lengths){
                if (t == "all"){
                    puzzle_lengths = {
                        "oneMove",
                        "short",
                        "long",
                        "veryLong",
                        "noLength"
                    };
                    break;
                }
            }

            
            std::vector<std::string> puzzles;
            std::vector<std::string> puzzles_files;
            for (std::string puzzle_length : puzzle_lengths){
                for (std::string puzzle_type : puzzle_types){
                    std::vector<std::string> new_puzzles = loadFromFile("engine\\test_positions\\" + puzzle_length + "_" + puzzle_type + ".txt", count);
                    for (std::string p : new_puzzles){
                        puzzles.emplace_back(p);
                        puzzles_files.emplace_back(puzzle_length + "_" + puzzle_type);
                    }
                }
            }
            
            int hit_count = 0;
            int cur_hit_count = 0;
            int cur_hit_total = 0;
            std::string cur_puzzle_type = "";
            
            for (int i = 0; i < puzzles.size(); i++){
                if (cur_puzzle_type != puzzles_files[i] && print){
                    if (cur_hit_total){
                        std::string spacing = " ";
                        for (int k = 0; k < std::max(1, 27 - (int)cur_puzzle_type.length()); k++){
                            spacing += " ";
                        }
                        std::cout << "Hit rate: " << cur_puzzle_type << ":" << spacing << ((double)cur_hit_count)/((double)cur_hit_total) << '\n';
                    }
                    cur_puzzle_type = puzzles_files[i];
                    cur_hit_count = 0;
                    cur_hit_total = 0;
                }
                std::string puzzle = puzzles[i];
                std::string fen;
                std::string move_str;
                for (int j = 0; j < puzzle.length(); j++){
                    if (puzzle[j] == ','){
                        fen = puzzle.substr(0, j);
                        move_str = puzzle.substr(j + 1, puzzle.length());
                    }
                }

                board.setFen(fen);
                Move puzzle_move = Visuals::stringToMove(move_str, board);
                Move best_move = engine.getBestMove(board, search_limit, print == 3);
                
                std::string spacing = " ";
                for (int k = 0; k < std::max(1, 27 - (int)puzzles_files[i].length()); k++){
                    spacing += " ";
                }
                
                cur_hit_total++;
                if (best_move == puzzle_move){
                    // correct move
                    hit_count++;
                    cur_hit_count++;
                }
                if (print >= 2){
                    if (best_move == puzzle_move){
                        // correct move
                        std::cout << "    " << "Types: " << puzzles_files[i] << spacing << Visuals::moveToString(best_move) << " == " << Visuals::moveToString(puzzle_move) << "    " << fen << '\n';
                    }
                    else{
                        // incorrect move
                        std::cout << " X  " << "Types: " << puzzles_files[i] << spacing << Visuals::moveToString(best_move) << " != " << Visuals::moveToString(puzzle_move) << "    " << fen << '\n';
                    }
                }
            }

            std::cout << "Hit rate: " << ((double)hit_count)/((double)puzzles.size()) << '\n';
        }

    }

} // namespace Chess::Engine::Test


#endif
