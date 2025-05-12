
#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <cstdint>
#include <array>
#include <iostream>


namespace Chess{

    namespace Castling{
        constexpr uint8_t NO_RIGHTS  = 0;
        constexpr uint8_t KINGSIDE   = 1;
        constexpr uint8_t QUEENSIDE  = 2;
        constexpr uint8_t ALL_RIGHTS = 3;
    } // namespace Castling

    using Direction = int8_t;
    namespace Directions{

        constexpr Direction NORTH = 8;
        constexpr Direction SOUTH = -8;
        constexpr Direction EAST  = 1;
        constexpr Direction WEST  = -1;
        
        constexpr Direction NORTH2 = 16;
        constexpr Direction SOUTH2 = -16;
        
        constexpr Direction NW = NORTH + WEST;
        constexpr Direction NE = NORTH + EAST;
        constexpr Direction SW = SOUTH + WEST;
        constexpr Direction SE = SOUTH + EAST;

        constexpr Direction NWW = NORTH + WEST  + WEST;
        constexpr Direction NNW = NORTH + NORTH + WEST;
        constexpr Direction NNE = NORTH + NORTH + EAST;
        constexpr Direction NEE = NORTH + EAST  + EAST;
        constexpr Direction SEE = SOUTH + EAST  + EAST;
        constexpr Direction SSE = SOUTH + SOUTH + EAST;
        constexpr Direction SSW = SOUTH + SOUTH + WEST;
        constexpr Direction SWW = SOUTH + WEST  + WEST;

    } // namespace Directions

    using Square = uint8_t;
    namespace Squares{

        constexpr Square A1 =  0;
        constexpr Square B1 =  1;
        constexpr Square C1 =  2;
        constexpr Square D1 =  3;
        constexpr Square E1 =  4;
        constexpr Square F1 =  5;
        constexpr Square G1 =  6;
        constexpr Square H1 =  7;
        constexpr Square A2 =  8;
        constexpr Square B2 =  9;
        constexpr Square C2 = 10;
        constexpr Square D2 = 11;
        constexpr Square E2 = 12;
        constexpr Square F2 = 13;
        constexpr Square G2 = 14;
        constexpr Square H2 = 15;
        constexpr Square A3 = 16;
        constexpr Square B3 = 17;
        constexpr Square C3 = 18;
        constexpr Square D3 = 19;
        constexpr Square E3 = 20;
        constexpr Square F3 = 21;
        constexpr Square G3 = 22;
        constexpr Square H3 = 23;
        constexpr Square A4 = 24;
        constexpr Square B4 = 25;
        constexpr Square C4 = 26;
        constexpr Square D4 = 27;
        constexpr Square E4 = 28;
        constexpr Square F4 = 29;
        constexpr Square G4 = 30;
        constexpr Square H4 = 31;
        constexpr Square A5 = 32;
        constexpr Square B5 = 33;
        constexpr Square C5 = 34;
        constexpr Square D5 = 35;
        constexpr Square E5 = 36;
        constexpr Square F5 = 37;
        constexpr Square G5 = 38;
        constexpr Square H5 = 39;
        constexpr Square A6 = 40;
        constexpr Square B6 = 41;
        constexpr Square C6 = 42;
        constexpr Square D6 = 43;
        constexpr Square E6 = 44;
        constexpr Square F6 = 45;
        constexpr Square G6 = 46;
        constexpr Square H6 = 47;
        constexpr Square A7 = 48;
        constexpr Square B7 = 49;
        constexpr Square C7 = 50;
        constexpr Square D7 = 51;
        constexpr Square E7 = 52;
        constexpr Square F7 = 53;
        constexpr Square G7 = 54;
        constexpr Square H7 = 55;
        constexpr Square A8 = 56;
        constexpr Square B8 = 57;
        constexpr Square C8 = 58;
        constexpr Square D8 = 59;
        constexpr Square E8 = 60;
        constexpr Square F8 = 61;
        constexpr Square G8 = 62;
        constexpr Square H8 = 63;

        inline Square getX(Square s){return s & 7;}

        inline Square getY(Square s){return (s >> 3) & 7;}

    } // namespace Square

    using Colour = bool;
    namespace Colours{
        constexpr Colour WHITE = 0;
        constexpr Colour BLACK = 1;
    } // namespace Colours

    using Piece = uint8_t;
    namespace Pieces{

        constexpr Piece EMPTY    = 0;
        constexpr Piece W_PAWN   = 1;
        constexpr Piece W_KNIGHT = 2;
        constexpr Piece W_BISHOP = 3;
        constexpr Piece W_ROOK   = 4;
        constexpr Piece W_QUEEN  = 5;
        constexpr Piece W_KING   = 6;

        constexpr Piece B_PAWN   = 9;
        constexpr Piece B_KNIGHT = 10;
        constexpr Piece B_BISHOP = 11;
        constexpr Piece B_ROOK   = 12;
        constexpr Piece B_QUEEN  = 13;
        constexpr Piece B_KING   = 14;

        constexpr Piece PIECES[12] = {
            W_PAWN,
            W_KNIGHT,
            W_BISHOP,
            W_ROOK,
            W_QUEEN,
            W_KING,
            B_PAWN,
            B_KNIGHT,
            B_BISHOP,
            B_ROOK,
            B_QUEEN,
            B_KING
        };

        constexpr Piece UNDO_EP_PIECE = 15; // used to store that a move was ep

        inline Piece oppColour(Piece p){
            return p ^ 8;
        }

        inline Colour getColour(Piece p){
            return p & 8;
        }

        inline Piece typeOf(Piece p){
            return p & 7;
        }

        inline Piece gen(Piece type, Colour col){
            return type | (col * 8);
        }

    } // namespace Square

    using Move = uint16_t;
    namespace Moves{

        /*
        (inclusive bits where the first bit is 1)
        from is 1-6 bits
        to   is 7-12 bits
        flag is 13 - 14 bits
            1 << 13 is kingside castling
            2 << 13 is queenside castling
            3 << 13 is promoting
        promotion piece is the remaing 2 bits
        */

        inline Square from(Move m){
            return m & 63;
        }

        inline Square to  (Move m){
            return (m >> 6) & 63;
        }

        /// @brief returns the piece type of a promotion
        /// @param m 
        /// @return returns the piece type of the promotion
        inline Piece promotionPiece(Move m){
            return ((m >> 14) & 3) + Pieces::W_KNIGHT;
        }

        inline bool isPromotion(Move m){
            return ((m >> 12) & 3) == 3;
        }

        template<bool kingside>
        inline bool castling(Move m){
            if (kingside){
                return ((m >> 12) & 3) == 1;
            }
            return ((m >> 12) & 3) == 2;
        }

        template<bool castle_kingside = 0, bool castle_queenside = 0, Piece promotion_piece = 0>
        inline Move makeMove(Square from, Square to){
            return (from) |
                   (to << 6) |
                   (castle_kingside  ? 4096 : 0) |
                   (castle_queenside ? 8192 : 0) |
                   (promotion_piece  ? 12288 : 0) |
                   ((std::max<Piece>(Pieces::typeOf(promotion_piece), Pieces::W_KNIGHT) - 2) << 14);
        }
        

    } // namespace Square

    using Bitboard = uint64_t;
    namespace Bitboards{
        
        constexpr Bitboard FILEA_BB = 72340172838076673ULL;
        constexpr Bitboard FILEB_BB = FILEA_BB << 1;
        constexpr Bitboard FILEC_BB = FILEA_BB << 2;
        constexpr Bitboard FILED_BB = FILEA_BB << 3;
        constexpr Bitboard FILEE_BB = FILEA_BB << 4;
        constexpr Bitboard FILEF_BB = FILEA_BB << 5;
        constexpr Bitboard FILEG_BB = FILEA_BB << 6;
        constexpr Bitboard FILEH_BB = FILEA_BB << 7;

        constexpr Bitboard RANK1_BB = 255ULL;
        constexpr Bitboard RANK2_BB = RANK1_BB << 8;
        constexpr Bitboard RANK3_BB = RANK1_BB << 16;
        constexpr Bitboard RANK4_BB = RANK1_BB << 24;
        constexpr Bitboard RANK5_BB = RANK1_BB << 32;
        constexpr Bitboard RANK6_BB = RANK1_BB << 40;
        constexpr Bitboard RANK7_BB = RANK1_BB << 48;
        constexpr Bitboard RANK8_BB = RANK1_BB << 56;

        constexpr Bitboard PROMOTION_RANKS = RANK1_BB | RANK8_BB;
        constexpr Bitboard NOT_PROMOTION_RANKS = ~PROMOTION_RANKS;

        constexpr Bitboard FULL_BITBOARD = 0xffffffffffffffffULL;

        constexpr Bitboard B1C1D1_BB = 14;
        constexpr Bitboard C1D1_BB   = 12;

        constexpr Bitboard F1G1_BB   = 96;

        constexpr Bitboard B8C8D8_BB = 1008806316530991104ULL;
        constexpr Bitboard C8D8_BB   = 864691128455135232ULL;

        constexpr Bitboard F8G8_BB   = 6917529027641081856ULL;


        inline Bitboard getBit(Square square){
            return 1ULL << square;
        }

        template<Direction D>
        constexpr Bitboard shiftBB(Bitboard b){
            if (D == Directions::NORTH){
                return b << 8;
            }
            if (D == Directions::SOUTH){
                return b >> 8;
            }
            if (D == Directions::EAST){
                return (b << 1) & ~FILEA_BB;
            }
            if (D == Directions::WEST){
                return (b >> 1) & ~FILEH_BB;
            }
            if (D == Directions::NE){
                return (b << 9) & ~FILEA_BB;
            }
            if (D == Directions::NW){
                return (b << 7) & ~FILEH_BB;
            }
            if (D == Directions::SE){
                return (b >> 7) & ~FILEA_BB;
            }
            if (D == Directions::SW){
                return (b >> 9) & ~FILEH_BB;
            }
            if (D == Directions::NORTH2){
                return b << 16;
            }
            if (D == Directions::SOUTH2){
                return b >> 16;
            }
            if (D == Directions::NNW){
                return (b << 15) & ~FILEH_BB;
            }
            if (D == Directions::NNE){
                return (b << 17) & ~FILEA_BB;
            }
            if (D == Directions::NWW){
                return (b << 6) & ~(FILEG_BB | FILEH_BB);
            }
            if (D == Directions::NEE){
                return (b << 10) & ~(FILEA_BB | FILEB_BB);
            }
            if (D == Directions::SSW){
                return (b >> 17) & ~FILEH_BB;
            }
            if (D == Directions::SSE){
                return (b >> 15) & ~FILEA_BB;
            }
            if (D == Directions::SWW){
                return (b >> 10) & ~(FILEG_BB | FILEH_BB);
            }
            if (D == Directions::SEE){
                return (b >> 6) & ~(FILEA_BB | FILEB_BB);
            }
            
            return b;
        }


        template<Direction D>
        constexpr void shiftThisBB(Bitboard& b){
            if (D == Directions::NORTH){
                b <<= 8;
            }
            if (D == Directions::SOUTH){
                b >>= 8;
            }
            if (D == Directions::EAST){
                b = (b << 1) & ~FILEA_BB;
            }
            if (D == Directions::WEST){
                b = (b >> 1) & ~FILEH_BB;
            }
            if (D == Directions::NE){
                b = (b << 9) & ~FILEA_BB;
            }
            if (D == Directions::NW){
                b = (b << 7) & ~FILEH_BB;
            }
            if (D == Directions::SE){
                b = (b >> 7) & ~FILEA_BB;
            }
            if (D == Directions::SW){
                b = (b >> 9) & ~FILEH_BB;
            }
            if (D == Directions::NORTH2){
                b <<= 16;
            }
            if (D == Directions::SOUTH2){
                b >>= 16;
            }
            if (D == Directions::NNW){
                b = (b << 15) & ~FILEH_BB;
            }
            if (D == Directions::NNE){
                b = (b << 17) & ~FILEA_BB;
            }
            if (D == Directions::NWW){
                b = (b << 6) & ~(FILEG_BB | FILEH_BB);
            }
            if (D == Directions::NEE){
                b = (b << 10) & ~(FILEA_BB | FILEB_BB);
            }
            if (D == Directions::SSW){
                b = (b >> 17) & ~FILEH_BB;
            }
            if (D == Directions::SSE){
                b = (b >> 15) & ~FILEA_BB;
            }
            if (D == Directions::SWW){
                b = (b >> 10) & ~(FILEG_BB | FILEH_BB);
            }
            if (D == Directions::SEE){
                b = (b >> 6) & ~(FILEA_BB | FILEB_BB);
            }   
        }

        Bitboard getBishopAttacks(Bitboard blockers, const Square square){

            Bitboard attacks = 0;
        
            const int x = Squares::getX(square);
            const int y = Squares::getY(square);
        
            const int up_left    = std::min<int>(x    , 7 - y);
            const int up_right   = std::min<int>(7 - x, 7 - y);
            const int down_left  = std::min<int>(    x,     y);
            const int down_right = std::min<int>(7 - x,     y);
        
            const Bitboard s = Bitboards::getBit(square);
            // up left, nw
            Bitboard square_bb = s;
            for (int i = 0; i < up_left; i++){
                square_bb <<= Directions::NW;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
        
            // up right, ne
            square_bb = s;
            for (int i = 0; i < up_right; i++){
                square_bb <<= Directions::NE;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
            
            // down left, sw
            square_bb = s;
            for (int i = 0; i < down_left; i++){
                square_bb >>= -Directions::SW;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
            // down right, se
            square_bb = s;
            for (int i = 0; i < down_right; i++){
                square_bb >>= -Directions::SE;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
        
            return attacks;
        }


        Bitboard getBishopAttackedSquares(Bitboard blockers, const Bitboard bishops){

            const Bitboard not_blockers = ~blockers;

            Bitboard attacks = 0;
        
            Bitboard loop_bb = bishops;

            // nw
            while (loop_bb){
                loop_bb = shiftBB<Directions::NW>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }

            // ne
            loop_bb = bishops;
            while (loop_bb){
                loop_bb = shiftBB<Directions::NE>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
            
            // sw
            loop_bb = bishops;
            while (loop_bb){
                loop_bb = shiftBB<Directions::SW>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }

            // se
            loop_bb = bishops;
            while (loop_bb){
                loop_bb = shiftBB<Directions::SE>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
        
            return attacks;
        }


        Bitboard getRookAttacks(Bitboard blockers, const Square square){

            Bitboard attacks = 0;

            const Bitboard not_blockers = ~blockers;
        
            const int x = Squares::getX(square);
            const int y = Squares::getY(square);
            const int ox = 7 - Squares::getX(square);
            const int oy = 7 - Squares::getY(square);
        
            const Bitboard s = Bitboards::getBit(square);

            Bitboard square_bb = s;

            // up, north
            square_bb = s;
            for (int i = 0; i < oy; i++){
                square_bb <<= Directions::NORTH;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
        
            // down, south
            square_bb = s;
            for (int i = 0; i < y; i++){
                square_bb >>= -Directions::SOUTH;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
            
            // left, west
            square_bb = s;
            for (int i = 0; i < x; i++){
                square_bb >>= -Directions::WEST;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
            // right, east
            square_bb = s;
            for (int i = 0; i < ox; i++){
                square_bb <<= Directions::EAST;
                attacks |= square_bb;
                if (blockers & square_bb){
                    break;
                }
            }
        
            return attacks;
        }


        Bitboard getRookAttackedSquares(Bitboard blockers, const Bitboard rooks){

            const Bitboard not_blockers = ~blockers;

            Bitboard attacks = 0;
        
            Bitboard loop_bb = rooks;

            // up, north
            while (loop_bb){
                loop_bb = shiftBB<Directions::NORTH>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
            
            // down, south
            loop_bb = rooks;
            while (loop_bb){
                loop_bb = shiftBB<Directions::SOUTH>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
            
            // left, west
            loop_bb = rooks;
            while (loop_bb){
                loop_bb = shiftBB<Directions::WEST>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
            
            // right, east
            loop_bb = rooks;
            while (loop_bb){
                loop_bb = shiftBB<Directions::EAST>(loop_bb);
                attacks |= loop_bb;
                loop_bb &= not_blockers;
            }
        
            return attacks;
        }


        Bitboard getQueenAttacks(Bitboard blockers, Square square){
            return getBishopAttacks(blockers, square) | getRookAttacks(blockers, square);
        }


        inline Bitboard getKnightAttacks(const Bitboard b){
            return shiftBB<Directions::NEE>(b) | shiftBB<Directions::NNE>(b) | shiftBB<Directions::NNW>(b) | shiftBB<Directions::NWW>(b) |
                   shiftBB<Directions::SEE>(b) | shiftBB<Directions::SSE>(b) | shiftBB<Directions::SSW>(b) | shiftBB<Directions::SWW>(b);
        }


        inline Bitboard getKnightAttacks(const Square square){
            Bitboard b = getBit(square);
            return shiftBB<Directions::NEE>(b) | shiftBB<Directions::NNE>(b) | shiftBB<Directions::NNW>(b) | shiftBB<Directions::NWW>(b) |
                   shiftBB<Directions::SEE>(b) | shiftBB<Directions::SSE>(b) | shiftBB<Directions::SSW>(b) | shiftBB<Directions::SWW>(b);
        }


        template<Colour c>
        inline Bitboard getPawnAttacks(const Bitboard pawns){
            if (c == Colours::WHITE){
                return shiftBB<Directions::NW>(pawns) | shiftBB<Directions::NE>(pawns);
            }
            return shiftBB<Directions::SW>(pawns) | shiftBB<Directions::SE>(pawns);
        }

        inline Bitboard getKingAttacks(const Bitboard king){
            return Bitboards::shiftBB<Directions::NORTH>(king) |
                   Bitboards::shiftBB<Directions::SOUTH>(king) |
                   Bitboards::shiftBB<Directions::EAST> (king) |
                   Bitboards::shiftBB<Directions::WEST> (king) |
                   Bitboards::shiftBB<Directions::NE>   (king) |
                   Bitboards::shiftBB<Directions::NW>   (king) |
                   Bitboards::shiftBB<Directions::SE>   (king) |
                   Bitboards::shiftBB<Directions::SW>   (king);
        }

        inline Bitboard getKingAttacks(const Square king){
            return getKingAttacks(getBit(king));
        }

        inline Square getNextSquare(Bitboard& bb){
            return (Square)__builtin_ctzll(bb);
        }
        
        inline Square popNextSquare(Bitboard& bb){
            Square s = (Square)__builtin_ctzll(bb);
            bb &= bb - 1;
            return s;
        }
        
        inline Bitboard getNextBit(Bitboard& bb){
            return bb & -bb;
        }
        
        inline Bitboard popNextBit(Bitboard& bb){
            Bitboard rb = bb & -bb;
            bb ^= rb;
            return rb;
        }
        
        inline int countBits(Bitboard number){
            return __builtin_popcountll(number);
        }

        std::array<Bitboard, 4096> initialiseBetweenBitboards(){
            Direction direction = 0;
            std::array<Bitboard, 4096> bitboards = {};
            for (Square s1 = Squares::A1; s1 <= Squares::H8; s1++){
                for (Square s2 = Squares::A1; s2 <= Squares::H8; s2++){
                    if (s1 == s2){
                        bitboards[s1 * 64 + s2] = 0;
                    }
                    else if (s1 >= s2){
                        bitboards[s1 * 64 + s2] = bitboards[s2 * 64 + s1];
                    }
                    else if (Squares::getY(s1) == Squares::getY(s2)){
                        // same rank so set the bitboard between them
                        // s2 is larger than s1
                        direction = Directions::EAST;
                        for (int i = 0; i <= Squares::getX(s2) - Squares::getX(s1); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1);
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s2);
                    }
                    else if (Squares::getX(s1) == Squares::getX(s2)){
                        direction = Directions::NORTH;
                        
                        for (int i = 0; i <= Squares::getY(s2) - Squares::getY(s1); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1);
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s2);
                    }
                    
                    else if (Squares::getY(s1) - Squares::getY(s2) == Squares::getX(s1) - Squares::getX(s2)){
                        // up right diagonal
                        direction = Directions::NE;
                        for (int i = 0; i <= Squares::getY(s2) - Squares::getY(s1); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1);
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s2);
                    }
                    
                    else if (Squares::getY(s1) - Squares::getY(s2) == Squares::getX(s2) - Squares::getX(s1)){
                        // up left diagonal
                        direction = Directions::NW;
                        for (int i = 0; i <= Squares::getY(s2) - Squares::getY(s1); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1);
                        bitboards[s1 * 64 + s2] |= Bitboards::getBit(s2);
                    }
                }
            }
            return bitboards;
        }
        
        
        std::array<Bitboard, 4096> initialiseThroughBitboards(){
            Direction direction = 0;
            std::array<Bitboard, 4096> bitboards = {};
            for (Square s1 = Squares::A1; s1 <= Squares::H8; s1++){
                for (Square s2 = Squares::A1; s2 <= Squares::H8; s2++){
                    if (s1 == s2){
                        bitboards[s1 * 64 + s2] = 0;
                    }
                    else if (s1 >= s2){
                        bitboards[s1 * 64 + s2] = bitboards[s2 * 64 + s1];
                    }
                    else if (Squares::getY(s1) == Squares::getY(s2)){
                        // same rank so set the bitboard between them
                        // s2 is larger than s1
                        bitboards[s1 * 64 + s2] = RANK1_BB << 8 * Squares::getY(s1);
                    }
                    else if (Squares::getX(s1) == Squares::getX(s2)){
        
                        bitboards[s1 * 64 + s2] = FILEA_BB << Squares::getX(s1);
                    }
        
                    else if (Squares::getY(s1) - Squares::getY(s2) == Squares::getX(s1) - Squares::getX(s2)){
                        // up left diagonal
                        direction = Directions::NE;
                        for (int i = -std::min(Squares::getY(s1), Squares::getX(s1)); i <= std::min(7 - Squares::getY(s1), 7 - Squares::getX(s1)); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                    }
        
                    else if (Squares::getY(s1) - Squares::getY(s2) == Squares::getX(s2) - Squares::getX(s1)){
                        // up right diagonal
                        direction = Directions::NW;
                        for (int i = -std::min<uint8_t>(Squares::getY(s1), 7 - Squares::getX(s1)); i <= std::min<uint8_t>(7 - Squares::getY(s1), Squares::getX(s1)); i++){
                            bitboards[s1 * 64 + s2] |= Bitboards::getBit(s1 + i * direction);
                        }
                    }
                }
            }
            return bitboards;
        }
        
        
        const std::array<Bitboard, 4096> BETWEEN_SQUARES = initialiseBetweenBitboards();
        const std::array<Bitboard, 4096> THROUGH_SQUARES = initialiseThroughBitboards();
        

        inline Bitboard bitboardBetween(Square square1, Square square2){
            return BETWEEN_SQUARES[square1 * 64 + square2];
        }
        
        inline Bitboard bitboardThrough(Square square1, Square square2){
            return THROUGH_SQUARES[square1 * 64 + square2];
        }

        constexpr Square NE_SW_DIAGONAL_SQUARES[128] = {
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::D1, Squares::H5,
            Squares::E1, Squares::H4,
            Squares::F1, Squares::H3,
            Squares::G1, Squares::H2,
            Squares::H1, Squares::H1,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::D1, Squares::H5,
            Squares::E1, Squares::H4,
            Squares::F1, Squares::H3,
            Squares::G1, Squares::H2,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::D1, Squares::H5,
            Squares::E1, Squares::H4,
            Squares::F1, Squares::H3,
            Squares::A4, Squares::E8,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::D1, Squares::H5,
            Squares::E1, Squares::H4,
            Squares::A5, Squares::D8,
            Squares::A4, Squares::E8,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::D1, Squares::H5,
            Squares::A6, Squares::C8,
            Squares::A5, Squares::D8,
            Squares::A4, Squares::E8,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::C1, Squares::H6,
            Squares::A7, Squares::B8,
            Squares::A6, Squares::C8,
            Squares::A5, Squares::D8,
            Squares::A4, Squares::E8,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8,
            Squares::B1, Squares::H7,
            Squares::A8, Squares::A8,
            Squares::A7, Squares::B8,
            Squares::A6, Squares::C8,
            Squares::A5, Squares::D8,
            Squares::A4, Squares::E8,
            Squares::A3, Squares::F8,
            Squares::A2, Squares::G8,
            Squares::A1, Squares::H8
        };

        constexpr Square NW_SE_DIAGONAL_SQUARES[128] = {
            Squares::A1, Squares::A1,
            Squares::B1, Squares::A2,
            Squares::C1, Squares::A3,
            Squares::D1, Squares::A4,
            Squares::E1, Squares::A5,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::B1, Squares::A2,
            Squares::C1, Squares::A3,
            Squares::D1, Squares::A4,
            Squares::E1, Squares::A5,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::C1, Squares::A3,
            Squares::D1, Squares::A4,
            Squares::E1, Squares::A5,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::D1, Squares::A4,
            Squares::E1, Squares::A5,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::H4, Squares::D8,
            Squares::E1, Squares::A5,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::H4, Squares::D8,
            Squares::H5, Squares::E8,
            Squares::F1, Squares::A6,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::H4, Squares::D8,
            Squares::H5, Squares::E8,
            Squares::H6, Squares::F8,
            Squares::G1, Squares::A7,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::H4, Squares::D8,
            Squares::H5, Squares::E8,
            Squares::H6, Squares::F8,
            Squares::H7, Squares::G8,
            Squares::H1, Squares::A8,
            Squares::H2, Squares::B8,
            Squares::H3, Squares::C8,
            Squares::H4, Squares::D8,
            Squares::H5, Squares::E8,
            Squares::H6, Squares::F8,
            Squares::H7, Squares::G8,
            Squares::H8, Squares::H8
        };

    } // namespace bitboard

    namespace Visuals{

        const char PIECE_CHARS[16] = {' ', 'P', 'N', 'B', 'R', 'Q', 'K', '1', '2', 'p', 'n', 'b', 'r', 'q', 'k', '3'};

        const char X_COORDS[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
        const char Y_COORDS[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};

        Square stringToSquare(std::string square_str){
            return square_str[0] - 'a' + (square_str[1] - '1') * 8;
        }

        std::string moveToString(Move move){
            if (Moves::isPromotion(move)){
                return std::string(1, X_COORDS[Squares::getX(Moves::from(move))] - 'A' + 'a') +
                       std::string(1, Y_COORDS[Squares::getY(Moves::from(move))]) +
                       std::string(1, X_COORDS[Squares::getX(Moves::to  (move))] - 'A' + 'a') +
                       std::string(1, Y_COORDS[Squares::getY(Moves::to  (move))]) +
                       std::string(1, PIECE_CHARS[Moves::promotionPiece(move) | 8]);
            }
            return std::string(1, X_COORDS[Squares::getX(Moves::from(move))] - 'A' + 'a') +
                   std::string(1, Y_COORDS[Squares::getY(Moves::from(move))]) +
                   std::string(1, X_COORDS[Squares::getX(Moves::to  (move))] - 'A' + 'a') +
                   std::string(1, Y_COORDS[Squares::getY(Moves::to  (move))]);
        }

    } // namespace Visuals

} // namespace Chess

#endif
