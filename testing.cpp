
#include <fstream>

#include "chess/board.h"
#include "chess/move_generator.h"


Chess::MoveGeneration move_gen;

template<bool root, Chess::Colour turn>
uint64_t perft(Chess::Board& board, int depth){
    Chess::Zobrist::Hash hash = board.hash;
    board.setHash();
    if (board.hash != hash){
        std::cout << "Incorrect hash :(\n";
    }

    std::string info = std::to_string(hash) + ": " + std::to_string(board.colours[Chess::Colours::WHITE]) + " " + std::to_string(board.colours[Chess::Colours::BLACK]) + "\n";

    std::ofstream save_file("engine-logs\\logs.txt", std::ios::app);
    save_file << info;
    save_file.close();

    if (depth <= 0){
        return 1;
    }

    if (board.insufficientMaterial()) return 0;

    Chess::LegalMovesArray legal_moves = move_gen.generateLegalMoves<turn>(board);

    // if (depth == 1 && (!root)){
    //     return legal_moves.count;
    // }

    if (!root){
        uint64_t count = 0;
        for (Chess::Move move : legal_moves)
        {
            board.playMove(move);
            count += perft<false, !turn>(board, depth - 1);
            board.undoMove();
        }

        bool in_check = board.inCheck<turn>();
        { // null moves
            if (!in_check){
                Chess::Square ep = board.playNull();
                count += perft<false, !turn>(board, depth - 1);
                board.undoNull(ep);
            }
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
    Chess::Zobrist::initialise();

    Chess::Board board("8/8/R7/1p5k/1P1K3P/8/2n5/8 w - - 1 55");


    if (board.turn == Chess::Colours::WHITE) perft<true, Chess::Colours::WHITE>(board, 4);
    if (board.turn == Chess::Colours::BLACK) perft<true, Chess::Colours::BLACK>(board, 4);

    std::cout << "Correct :)\n";


}