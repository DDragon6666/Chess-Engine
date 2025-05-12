


#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <iostream>
#include <array>

#include "types.h"
#include "board.h"


namespace Chess{

    namespace Iterator{
        // Iterator class to iterate through the valid moves
        struct Iterator {
            Move* ptr;

            Iterator(Move* p) : ptr(p) {}

            // Overload the dereference operator
            Move& operator*() {
                return *ptr;
            }

            // Overload the increment operator (++)
            Iterator& operator++() {
                ++ptr;
                return *this;
            }

            // Overload the equality comparison operator
            bool operator!=(const Iterator& other) const {
                return ptr != other.ptr;
            }
        };
    } // namespace Iterator

    #pragma pack(push, 2)
    struct LegalMovesArray{
        Move moves[216];
        uint16_t count;

        LegalMovesArray(){
            count = 0;
        }

        // begin() returns an iterator to the first valid move
        Iterator::Iterator begin() {
            return Iterator::Iterator(moves);
        }

        // end() returns an iterator to one past the last valid move
        Iterator::Iterator end() {
            return Iterator::Iterator(moves + count);
        }

        void operator+=(Move m){
            moves[count++] = m;
        }

        Move operator[](int i){
            return moves[i];
        }

    };
    #pragma pack(pop)
    
    class MoveGeneration{
        public:

            LegalMovesArray moves;


            template<Colour turn>
            LegalMovesArray generateLegalMoves(Board& board){
                moves.count = 0;

                using namespace Bitboards;
                using namespace Colours;
                using namespace Pieces;

                const Square king_square = board.kingPos<turn>();

                // get pins and checkers
                const Bitboard king_rook_attacks   = Bitboards::getRookAttacks  (board.all_pieces, king_square);
                const Bitboard king_bishop_attacks = Bitboards::getBishopAttacks(board.all_pieces, king_square);

                Bitboard not_us = ~board.colours[turn];

                Bitboard attacked_squares = board.attackedBitboardNoKing<!turn>();

                // get checkers
                Bitboard checkers = (
                                     (king_rook_attacks   & (board.pieces[(turn * 8) ^ Pieces::B_ROOK]   | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                     (king_bishop_attacks & (board.pieces[(turn * 8) ^ Pieces::B_BISHOP] | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                     (getKnightAttacks(king_square) & board.pieces[(turn * 8) ^ Pieces::B_KNIGHT]) |
                                     (getPawnAttacks<turn>(board.pieces[(turn * 8) | Pieces::W_KING]) & board.pieces[(turn * 8) ^ Pieces::B_PAWN])
                                    ) & board.colours[!turn];

                Bitboard stop_checks;
                
                // add squares which stop checks
                switch(countBits(checkers)){
                    case 0:
                        stop_checks = FULL_BITBOARD;
                        break;
                    case 1:
                        stop_checks = bitboardBetween(getNextSquare(checkers), king_square) | checkers; // | checkers to add knights
                        break;
                    default:
                        // just get king moves
                        Bitboard legal_moves = Bitboards::getKingAttacks(board.pieces[(turn * 8) | Pieces::W_KING]) & not_us & ~attacked_squares;

                        while (legal_moves){
                            moves += Moves::makeMove(king_square, popNextSquare(legal_moves));
                        }

                        return moves;
                }

                const Bitboard ep_bb = getBit(board.ep) & ~1;

                const Bitboard pawn_stop_checks = stop_checks | ((((turn == Colours::WHITE) ? (stop_checks << 8) : (stop_checks >> 8))) & ep_bb);

                const Bitboard potential_pins = (king_rook_attacks | king_bishop_attacks) & board.colours[turn];

                Bitboard pinners        = ((getRookAttacks  (board.all_pieces ^ potential_pins, king_square) & (board.pieces[(turn * 8) ^ Pieces::B_ROOK]   | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                           (getBishopAttacks(board.all_pieces ^ potential_pins, king_square) & (board.pieces[(turn * 8) ^ Pieces::B_BISHOP] | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])));

                Bitboard pinned_pieces  = 0;

                while (pinners){
                    pinned_pieces |= Bitboards::bitboardBetween(Bitboards::popNextSquare(pinners), king_square);
                }

                pinned_pieces &= potential_pins;

                
                const Bitboard ew_to_king    = Bitboards::bitboardThrough(king_square & 0b111000, (king_square & 0b111000) | 1);
                const Bitboard ns_to_king    = Bitboards::bitboardThrough(king_square & 0b000111, (king_square & 0b000111) | 8);
                const Bitboard nw_se_to_king = Bitboards::bitboardThrough(Bitboards::NW_SE_DIAGONAL_SQUARES[king_square * 2], Bitboards::NW_SE_DIAGONAL_SQUARES[king_square * 2 + 1]); // ugly. needs making better probably
                const Bitboard ne_sw_to_king = Bitboards::bitboardThrough(Bitboards::NE_SW_DIAGONAL_SQUARES[king_square * 2], Bitboards::NE_SW_DIAGONAL_SQUARES[king_square * 2 + 1]); // ugly. needs making better probably
                
                // generate pawn legal moves - to remove unmoveable pinned pieces & ~(pinned_pieces) | (pinned_pieces & bitboard through Sk and Sk + NW)
                Bitboard legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NW : Directions::SE>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | nw_se_to_king))
                                    & (board.colours[!turn] | ep_bb) & pawn_stop_checks;

                // add moves. If black add promotion moves first
                Square move_square;
                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                    }
                }
                
                legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NE : Directions::SW>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | ne_sw_to_king))
                                    & (board.colours[!turn] | ep_bb) & pawn_stop_checks;
                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                    }
                }

                legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | ns_to_king))
                                    & ~board.all_pieces;
                Bitboard pieces = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(legal_moves & (turn == Colours::WHITE ? Bitboards::RANK3_BB : Bitboards::RANK6_BB)) & ~board.all_pieces;
                legal_moves &= stop_checks;
                pieces &= stop_checks;

                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                    }
                }

                while (pieces){
                    move_square = Bitboards::popNextSquare(pieces);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NORTH2) : (move_square - Directions::SOUTH2), move_square);
                }

                // knight moves
                pieces = board.pieces[(turn * 8) | Pieces::W_KNIGHT] & ~pinned_pieces;
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getKnightAttacks(move_square) & not_us & stop_checks;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // bishop and queen diagonal moves - not pinned
                pieces = (board.pieces[(turn * 8) | Pieces::W_BISHOP] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & ~pinned_pieces;
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getBishopAttacks(board.all_pieces, move_square) & not_us & stop_checks;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }
                // pinned bishops and queens - can ignore if in check as they cant stop the check
                pieces = (board.pieces[(turn * 8) | Pieces::W_BISHOP] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & pinned_pieces & (nw_se_to_king | ne_sw_to_king) & (checkers ? 0 : Bitboards::FULL_BITBOARD);
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getBishopAttacks(board.all_pieces, move_square) & not_us & (ne_sw_to_king | nw_se_to_king) & stop_checks;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // rook and queen straight moves - not pinned
                pieces = (board.pieces[(turn * 8) | Pieces::W_ROOK] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & ~pinned_pieces;

                
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getRookAttacks(board.all_pieces, move_square) & not_us & stop_checks;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }
                // pinned rooks and queens
                pieces = (board.pieces[(turn * 8) | Pieces::W_ROOK] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & pinned_pieces & (ns_to_king | ew_to_king) & (checkers ? 0 : Bitboards::FULL_BITBOARD);
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);

                    pieces &= pieces - 1;
                    
                    legal_moves = Bitboards::getRookAttacks(board.all_pieces, move_square) & not_us & (ns_to_king | ew_to_king) & stop_checks;
                    
                    
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // king moves
                legal_moves = Bitboards::getKingAttacks(board.pieces[(turn * 8) | Pieces::W_KING]) & not_us & ~attacked_squares;


                while (legal_moves){
                    moves += Moves::makeMove(king_square, __builtin_ctzll(legal_moves));
                    legal_moves &= legal_moves - 1;
                }

                // castling (queenside)
                if ((board.castling_rights[turn] & Castling::QUEENSIDE) &&
                   !((board.all_pieces & (turn == WHITE ? Bitboards::B1C1D1_BB : Bitboards::B8C8D8_BB)) ||
                     (attacked_squares & (turn == WHITE ? Bitboards::C1D1_BB   : Bitboards::C8D8_BB))) &&
                    (checkers == 0)){
                    moves += Moves::makeMove<0, 1>((turn == BLACK) * 56 + Squares::E1, (turn == BLACK) * 56 + Squares::C1);
                }
                // (kingside)
                if ((board.castling_rights[turn] & Castling::KINGSIDE) &&
                   !((board.all_pieces & (turn == WHITE ? Bitboards::F1G1_BB : Bitboards::F8G8_BB)) ||
                     (attacked_squares & (turn == WHITE ? Bitboards::F1G1_BB : Bitboards::F8G8_BB))) &&
                    (checkers == 0)){
                    moves += Moves::makeMove<1>((turn == BLACK) * 56 + Squares::E1, (turn == BLACK) * 56 + Squares::G1);
                }

                return moves;
            }


            LegalMovesArray getLegalMoves(Board& board){
                if (board.turn == Colours::WHITE){
                    generateLegalMoves<Colours::WHITE>(board);
                }
                else{
                    generateLegalMoves<Colours::BLACK>(board);
                }

                return moves;
            }


            template<Colour turn>
            LegalMovesArray generateTacticalMoves(Board& board){
                moves.count = 0;

                using namespace Bitboards;
                using namespace Colours;
                using namespace Pieces;

                const Square king_square = board.kingPos<turn>();

                // get pins and checkers
                const Bitboard king_rook_attacks   = Bitboards::getRookAttacks  (board.all_pieces, king_square);
                const Bitboard king_bishop_attacks = Bitboards::getBishopAttacks(board.all_pieces, king_square);

                Bitboard not_us = ~board.colours[turn];

                Bitboard attacked_squares = board.attackedBitboardNoKing<!turn>();

                // get checkers
                Bitboard checkers = (
                                     (king_rook_attacks   & (board.pieces[(turn * 8) ^ Pieces::B_ROOK]   | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                     (king_bishop_attacks & (board.pieces[(turn * 8) ^ Pieces::B_BISHOP] | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                     (getKnightAttacks(king_square) & board.pieces[(turn * 8) ^ Pieces::B_KNIGHT]) |
                                     (getPawnAttacks<turn>(board.pieces[(turn * 8) | Pieces::W_KING]) & board.pieces[(turn * 8) ^ Pieces::B_PAWN])
                                    ) & board.colours[!turn];

                Bitboard stop_checks;
                
                // add squares which stop checks
                switch(countBits(checkers)){
                    case 0:
                        stop_checks = FULL_BITBOARD;
                        break;
                    case 1:
                        stop_checks = bitboardBetween(getNextSquare(checkers), king_square) | checkers; // | checkers to add knights
                        break;
                    default:
                        // just get king moves
                        Bitboard legal_moves = Bitboards::getKingAttacks(board.pieces[(turn * 8) | Pieces::W_KING]) & not_us & ~attacked_squares;

                        while (legal_moves){
                            moves += Moves::makeMove(king_square, popNextSquare(legal_moves));
                        }

                        return moves;
                }

                Bitboard defended_by_pawn   = Bitboards::getPawnAttacks<!turn>(board.pieces[Pieces::gen(Pieces::W_PAWN, !turn)]);
                Bitboard defended_by_knight = Bitboards::getKnightAttacks(board.pieces[Pieces::gen(Pieces::W_KNIGHT, !turn)]);
                Bitboard defended_by_bishop = Bitboards::getBishopAttackedSquares(board.all_pieces, board.pieces[Pieces::gen(Pieces::W_BISHOP, !turn)]);
                Bitboard defended_by_rook   = Bitboards::getRookAttackedSquares  (board.all_pieces, board.pieces[Pieces::gen(Pieces::W_ROOK,   !turn)]);
                Bitboard defended_by_queen  = Bitboards::getBishopAttackedSquares(board.all_pieces, board.pieces[Pieces::gen(Pieces::W_QUEEN,  !turn)]) | Bitboards::getRookAttackedSquares  (board.all_pieces, board.pieces[Pieces::gen(Pieces::W_QUEEN,  !turn)]);
                Bitboard defended_by_king   = Bitboards::getKingAttacks(board.pieces[Pieces::gen(Pieces::W_KING, !turn)]);

                Bitboard enemy_non_pawns = board.all_pieces & ~board.pieces[Pieces::W_PAWN] & ~board.pieces[Pieces::B_PAWN]; // their non pawn pieces

                Bitboard pawn_tactical_moves   = (board.all_pieces | Bitboards::PROMOTION_RANKS) |
                                                 (shiftBB<turn == WHITE ? Directions::SW : Directions::NW>(enemy_non_pawns) & shiftBB<turn == WHITE ? Directions::SE : Directions::NE>(enemy_non_pawns)); // pawn forks
                Bitboard knight_tactical_moves = (board.all_pieces & ~defended_by_pawn) | enemy_non_pawns |
                                                 (Bitboards::getKnightAttacks(board.pieces[Pieces::gen(Pieces::W_QUEEN, !turn)]) & Bitboards::getKnightAttacks(board.pieces[Pieces::gen(Pieces::W_KING, !turn)]));
                Bitboard bishop_tactical_moves = (board.all_pieces & ~defended_by_pawn) | enemy_non_pawns; // board.all_pieces; // (board.all_pieces & ~defended_by_pawn) | enemy_non_pawns;
                Bitboard rook_tactical_moves   = (board.all_pieces & ~defended_by_pawn) | enemy_non_pawns; // board.all_pieces; // (board.all_pieces & ~defended_by_pawn) | enemy_non_pawns;
                Bitboard queen_tactical_moves  = (board.all_pieces & ~(defended_by_pawn | defended_by_bishop | defended_by_knight | defended_by_rook)) | board.pieces[gen(W_QUEEN, !turn)];
                Bitboard king_tactical_moves   = (board.all_pieces);

                const Bitboard ep_bb = getBit(board.ep) & ~1;

                const Bitboard pawn_stop_checks = stop_checks | ((((turn == Colours::WHITE) ? (stop_checks << 8) : (stop_checks >> 8))) & ep_bb);

                const Bitboard potential_pins = (king_rook_attacks | king_bishop_attacks) & board.colours[turn];

                Bitboard pinners        = ((getRookAttacks  (board.all_pieces ^ potential_pins, king_square) & (board.pieces[(turn * 8) ^ Pieces::B_ROOK]   | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])) |
                                           (getBishopAttacks(board.all_pieces ^ potential_pins, king_square) & (board.pieces[(turn * 8) ^ Pieces::B_BISHOP] | board.pieces[(turn * 8) ^ Pieces::B_QUEEN])));

                Bitboard pinned_pieces  = 0;

                while (pinners){
                    pinned_pieces |= Bitboards::bitboardBetween(Bitboards::popNextSquare(pinners), king_square);
                }

                pinned_pieces &= potential_pins;

                
                const Bitboard ew_to_king    = Bitboards::bitboardThrough(king_square & 0b111000, (king_square & 0b111000) | 1);
                const Bitboard ns_to_king    = Bitboards::bitboardThrough(king_square & 0b000111, (king_square & 0b000111) | 8);
                const Bitboard nw_se_to_king = Bitboards::bitboardThrough(Bitboards::NW_SE_DIAGONAL_SQUARES[king_square * 2], Bitboards::NW_SE_DIAGONAL_SQUARES[king_square * 2 + 1]); // ugly. needs making better probably
                const Bitboard ne_sw_to_king = Bitboards::bitboardThrough(Bitboards::NE_SW_DIAGONAL_SQUARES[king_square * 2], Bitboards::NE_SW_DIAGONAL_SQUARES[king_square * 2 + 1]); // ugly. needs making better probably
                
                // generate pawn legal moves - to remove unmoveable pinned pieces & ~(pinned_pieces) | (pinned_pieces & bitboard through Sk and Sk + NW)
                Bitboard legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NW : Directions::SE>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | nw_se_to_king))
                                    & (board.colours[!turn] | ep_bb) & pawn_stop_checks & pawn_tactical_moves;

                // add moves. If black add promotion moves first (cause they are the first bits)
                Square move_square;
                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NW) : (move_square - Directions::SE), move_square);
                    }
                }
                
                legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NE : Directions::SW>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | ne_sw_to_king))
                                    & (board.colours[!turn] | ep_bb) & pawn_stop_checks & pawn_tactical_moves;
                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NE) : (move_square - Directions::SW), move_square);
                    }
                }

                legal_moves = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(board.pieces[(turn * 8) | Pieces::W_PAWN] & (~pinned_pieces | ns_to_king))
                                    & ~board.all_pieces;
                Bitboard pieces = Bitboards::shiftBB<(turn == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(legal_moves & (turn == Colours::WHITE ? Bitboards::RANK3_BB : Bitboards::RANK6_BB)) & ~board.all_pieces;
                legal_moves &= stop_checks & pawn_tactical_moves;
                pieces &= stop_checks & pawn_tactical_moves;

                if (turn == Colours::BLACK){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                    }
                }
                while (legal_moves & Bitboards::NOT_PROMOTION_RANKS){
                    move_square = Bitboards::popNextSquare(legal_moves);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                }
                if (turn == Colours::WHITE){
                    while (legal_moves & Bitboards::PROMOTION_RANKS){
                        move_square = Bitboards::popNextSquare(legal_moves);
                        moves += Moves::makeMove<0, 0, Pieces::W_KNIGHT>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_BISHOP>((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_ROOK>  ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                        moves += Moves::makeMove<0, 0, Pieces::W_QUEEN> ((turn == WHITE) ? (move_square - Directions::NORTH) : (move_square - Directions::SOUTH), move_square);
                    }
                }

                while (pieces){
                    move_square = Bitboards::popNextSquare(pieces);
                    moves += Moves::makeMove((turn == WHITE) ? (move_square - Directions::NORTH2) : (move_square - Directions::SOUTH2), move_square);
                }

                // knight moves
                pieces = board.pieces[(turn * 8) | Pieces::W_KNIGHT] & ~pinned_pieces;
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getKnightAttacks(move_square) & not_us & stop_checks & knight_tactical_moves;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // bishop and queen diagonal moves - not pinned
                pieces = (board.pieces[(turn * 8) | Pieces::W_BISHOP] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & ~pinned_pieces;
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getBishopAttacks(board.all_pieces, move_square) & not_us & stop_checks & bishop_tactical_moves;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }
                // pinned bishops and queens - can ignore if in check as they cant stop the check
                pieces = (board.pieces[(turn * 8) | Pieces::W_BISHOP] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & pinned_pieces & (nw_se_to_king | ne_sw_to_king) & (checkers ? 0 : Bitboards::FULL_BITBOARD);
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getBishopAttacks(board.all_pieces, move_square) & not_us & (ne_sw_to_king | nw_se_to_king) & stop_checks & bishop_tactical_moves;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // rook and queen straight moves - not pinned
                pieces = (board.pieces[(turn * 8) | Pieces::W_ROOK] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & ~pinned_pieces;

                
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);
                    
                    pieces &= pieces - 1;

                    legal_moves = Bitboards::getRookAttacks(board.all_pieces, move_square) & not_us & stop_checks & rook_tactical_moves;
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }
                // pinned rooks and queens
                pieces = (board.pieces[(turn * 8) | Pieces::W_ROOK] | board.pieces[(turn * 8) | Pieces::W_QUEEN]) & pinned_pieces & (ns_to_king | ew_to_king) & (checkers ? 0 : Bitboards::FULL_BITBOARD);
                while (pieces){
                    move_square = Bitboards::getNextSquare(pieces);

                    pieces &= pieces - 1;
                    
                    legal_moves = Bitboards::getRookAttacks(board.all_pieces, move_square) & not_us & (ns_to_king | ew_to_king) & stop_checks & rook_tactical_moves;
                    
                    
                    while (legal_moves){
                        moves += Moves::makeMove(move_square, __builtin_ctzll(legal_moves));
                        legal_moves &= legal_moves - 1;
                    }
                }

                // king moves
                legal_moves = Bitboards::getKingAttacks(board.pieces[(turn * 8) | Pieces::W_KING]) & not_us & ~attacked_squares & king_tactical_moves;


                while (legal_moves){
                    moves += Moves::makeMove(king_square, __builtin_ctzll(legal_moves));
                    legal_moves &= legal_moves - 1;
                }

                // castling (queenside)
                if ((board.castling_rights[turn] & Castling::QUEENSIDE) &&
                   !((board.all_pieces & (turn == WHITE ? Bitboards::B1C1D1_BB : Bitboards::B8C8D8_BB)) ||
                     (attacked_squares & (turn == WHITE ? Bitboards::C1D1_BB   : Bitboards::C8D8_BB))) &&
                    (checkers == 0)){
                    moves += Moves::makeMove<0, 1>((turn == BLACK) * 56 + Squares::E1, (turn == BLACK) * 56 + Squares::C1);
                }
                // (kingside)
                if ((board.castling_rights[turn] & Castling::KINGSIDE) &&
                   !((board.all_pieces & (turn == WHITE ? Bitboards::F1G1_BB : Bitboards::F8G8_BB)) ||
                     (attacked_squares & (turn == WHITE ? Bitboards::F1G1_BB : Bitboards::F8G8_BB))) &&
                    (checkers == 0)){
                    moves += Moves::makeMove<1>((turn == BLACK) * 56 + Squares::E1, (turn == BLACK) * 56 + Squares::G1);
                }

                return moves;
            }


            LegalMovesArray getTacticalMoves(Board& board){
                if (board.turn == Colours::WHITE){
                    generateTacticalMoves<Colours::WHITE>(board);
                }
                else{
                    generateTacticalMoves<Colours::BLACK>(board);
                }

                return moves;
            }


            bool isLegalMove(Board& board, Move move){
                if (board.turn == Colours::WHITE){
                    generateLegalMoves<Colours::WHITE>(board);
                }
                else{
                    generateLegalMoves<Colours::BLACK>(board);
                }

                for (Move move2 : moves){
                    if (move == move2) return true;
                }

                return false;
            }
    };

    namespace Visuals{
        void printLegalMoves(LegalMovesArray& moves, bool debug = false){
            if (debug){
                for (Move move : moves){
                    std::cout << (int)move << " - " << Visuals::moveToString(move) << '\n';
                }
            }
            else{
                for (Move move : moves){
                    std::cout<< Visuals::moveToString(move) << '\n';
                }
            }
        }
    } // namespace Visuals
 
} // namespace Chess

#endif
