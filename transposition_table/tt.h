
#ifndef TT_H
#define TT_H

#include <array>

#include "zobrist.h"
#include "../engine/engine_types.h"


namespace Chess::Engine::TT{

    #pragma pack(push, 1)
    struct TTData{
        Zobrist::Hash hash;
        Evaluation eval;
        Depth depth;
        Move move;
        int node_type;

        TTData(Zobrist::Hash hash_, Evaluation eval_, Depth depth_, Move move_, int node_type_) : hash(hash_), eval(eval_), depth(depth_), move(move_), node_type(node_type_) {}
        TTData() : hash(0), eval(0), depth(0), move(0), node_type(0) {}
    };
    #pragma pack(pop)

    constexpr int count = 50000000;
    int filled = 0;

    TTData table[count];

    int getIndex(Zobrist::Hash hash){
        return hash % count;
    }

    void save(Zobrist::Hash hash, Evaluation eval, Depth depth, Move move, int node_type){
        int index = getIndex(hash);
        if (table[index].hash == 0 && table[index].eval == 0) filled++;
        table[index] = TTData(hash, eval, depth, move, node_type);
    }

    void clear(){
        for (int i = 0; i < count; i++){
            table[i] = TTData();
        }
        filled = 0;
    }

} // namespace Chess::Engine::TT


#endif