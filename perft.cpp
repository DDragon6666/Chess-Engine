

#include <iostream>
#include <thread>
#include <chrono>

#include "chess/board.h"
#include "chess/move_generator.h"

Chess::MoveGeneration move_gen;

template<bool root, Chess::Colour turn>
uint64_t perft(Chess::Board& board, int depth){
    if (depth <= 0){
        return 1;
    }

    Chess::LegalMovesArray legal_moves = move_gen.generateLegalMoves<turn>(board);

    if (depth == 1 && (!root)){
        return legal_moves.count;
    }

    if (!root){
        uint64_t count = 0;
        for (Chess::Move move : legal_moves)
        {
            board.playMove(move);
            count += perft<false, !turn>(board, depth - 1);
            board.undoMove();
        }
        return count;
    }

    else{
        uint64_t count = 0;
        uint64_t inc = 0;
        for (Chess::Move move : legal_moves){  
            board.playMove(move);
            inc = perft<false, !turn>(board, depth - 1);
            count += inc;
            std::cout << /*(int)move << " : " << */Chess::Visuals::moveToString(move) << ": ";
            // board.debug();
            // std::cout << " : ";
            std::cout << inc << "\n";
            board.undoMove();
        }
        return count;
    }


    return 0;
}


int main(){
    Chess::Board board;


    std::cout << "Fen:\n";
    std::string fen; std::getline(std::cin, fen);
    if (fen.substr(0, 5) != "start"){
        board.setFen(fen);
    }
    
    board.printBoard();
    // board.debug();


    int max_depth;
    std::cout << "Max Depth: ";
    std::cin  >> max_depth;

    uint64_t leaf_node_count;

    for (int depth = max_depth; depth <= max_depth; depth++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // wait a second

        auto start = std::chrono::high_resolution_clock::now();

        if (board.turn == Chess::Colours::WHITE){
            leaf_node_count = perft<true, Chess::Colours::WHITE>(board, depth);
        }
        else{
            leaf_node_count = perft<true, Chess::Colours::BLACK>(board, depth);
        }
            

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> duration = end - start;

        double time_taken = duration.count();

        std::cout << "Depth: " << depth << "  \tLeaf node count: " << leaf_node_count << "  \tExecution time:  " << time_taken << "s  \tPerf test speed: " << (int)(((double)leaf_node_count) / time_taken) << "n/s\n";
    }


    std::cout << "Done\n";
    int nothing;
    std::cin >> nothing;
    
    
}

