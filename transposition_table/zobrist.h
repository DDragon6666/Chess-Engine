



// zobrist.h
#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>


#include "..\random.h"
#include "..\chess\types.h"


namespace Chess::Zobrist{

    using Hash = uint64_t;

    Hash PIECE_KEYS[64][16];
    Hash EP_KEYS   [8];
    Hash W_CASTLE  [4];
    Hash B_CASTLE  [4];
    Hash TURN_KEY;

    bool initialise(){
        PRNG rng(1070372); // number taken from stockfish (position.cpp)

        for (int square = Chess::Squares::A1; square <= Chess::Squares::H8; square++){
            for (int i = 0; i < 16; i++){
                PIECE_KEYS[square][i] = 0;
            }
        }

        for (int square = Chess::Squares::A1; square <= Chess::Squares::H8; square++){
            for (Piece p : Pieces::PIECES){
                PIECE_KEYS[square][p] = rng.rand<Hash>();
            }
        }

        // set the enpassant keys
        for (int enpassant_number = 0; enpassant_number < 8; enpassant_number++){
            EP_KEYS[enpassant_number] = rng.rand<Hash>();
            // std::cout << "ep keys: " << EP_KEYS[enpassant_number] << "\n";
        }

        // set the turn key
        TURN_KEY = rng.rand<Hash>();
        // std::cout << "Turn key: " << TURN_KEY << '\n';

        // add 2 as athe first value will be 0 and last will be 1^2
        W_CASTLE[0] = 0;
        W_CASTLE[1] = rng.rand<Hash>();
        W_CASTLE[2] = rng.rand<Hash>();
        W_CASTLE[3] = W_CASTLE[1] ^ W_CASTLE[2];
        // std::cout << "wcastle keys: " << W_CASTLE[1] << "\n";
        // std::cout << "wcastle keys: " << W_CASTLE[2] << "\n";


        B_CASTLE[0] = 0;
        B_CASTLE[1] = rng.rand<Hash>();
        B_CASTLE[2] = rng.rand<Hash>();
        B_CASTLE[3] = B_CASTLE[1] ^ B_CASTLE[2];
        // std::cout << "bcastle keys: " << B_CASTLE[1] << "\n";
        // std::cout << "bcastle keys: " << B_CASTLE[2] << "\n";

        return true;
    }

}





#endif



