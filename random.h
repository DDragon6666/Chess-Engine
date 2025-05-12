

#ifndef RANDOM_H
#define RANDOM_H


#include <random>


template<class Type = int>
Type random(Type min, Type max, int seed = 0){
    static std::mt19937 rng(seed);
    if constexpr (std::is_integral<Type>::value){
        std::uniform_int_distribution<Type> dist(min, max - 1);
        return dist(rng);
    }
    else{
        std::uniform_real_distribution<Type> dist(min, max);
        return dist(rng);
    }
}



// taken from stockfish (misc.h file)
class PRNG {

    private:
    uint64_t s;

    uint64_t rand64() {

        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

    public:
    PRNG(uint64_t seed) :
        s(seed) {
    }

    template<typename T>
    T rand() {
        return T(rand64());
    }

    // Special generator used to fast init magic numbers.
    // Output values only have 1/8th of their bits set on average.
    template<typename T>
    T sparse_rand() {
        return T(rand64() & rand64() & rand64());
    }
};


#endif
