
#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <array>
#include <vector>

#include "types.h"
#include "..\transposition_table\zobrist.h"


namespace Chess{
    struct UndoMoveInfo{
        // move
        Move move;
    
        // castling rights
        uint8_t w_castling_rights;
        uint8_t b_castling_rights;
    
        // ep square
        Square ep;
    
        // captured piece
        Piece captured;
    
        // promoted
        Piece moved;

        // hash for undoing change in hash and checking if a position has repeated
        Zobrist::Hash hash;

        UndoMoveInfo(Move move_, uint8_t w_castling_rights_, uint8_t b_castling_rights_, Square ep_, Piece moved_, Piece captured_, Zobrist::Hash hash_){
            move = move_;
            w_castling_rights = w_castling_rights_;
            b_castling_rights = b_castling_rights_;
            ep = ep_;
            moved = moved_;
            captured = captured_;
            hash = hash_;
        }
    
    };

    class Board{
        public:
            Bitboard pieces[16];
            Bitboard colours[2];
            Bitboard all_pieces;
            Piece piece_arr[64];

            uint8_t castling_rights[2]; // first 2 bits are kingside, queenside

            Colour turn;

            Square ep;

            Zobrist::Hash hash;

            std::vector<UndoMoveInfo> move_stack;

        public:

            Piece getPieceAt(Square square){
                return piece_arr[square];
            }


            /// @brief removes the piece at square. If theress no piece there it will not change anything
            /// @param square 
            void removePieceAt(Square square){
                Piece p = getPieceAt(square);
                Bitboard nb = ~Bitboards::getBit(square);
                pieces[p] &= nb;
                colours[Pieces::getColour(p)] &= nb;
                all_pieces &= nb;
                piece_arr[square] = Pieces::EMPTY;
            }
            
            /// @brief sets the piece at square to piece. Doesn't remove the current piece at square
            /// @param square 
            /// @param piece 
            void setPieceAt(Square square, Piece piece){
                piece_arr[square] = piece;
                Bitboard b = Bitboards::getBit(square);
                pieces [piece] |= b;
                colours[Pieces::getColour(piece)] |= b;
                all_pieces |= b;
            }
            
            /// @brief replaces at piece at square with piece. Removes the current piece there then places the new one
            /// @param square the square to replace the piece
            /// @param piece the piece it will be replaced with
            void replacePieceAt(Square square, Piece piece){
                Bitboard b = Bitboards::getBit(square);
                // remove old piece
                Piece p = getPieceAt(square);
                pieces[p] &= ~b;
                colours[Pieces::getColour(p)] &= ~b;
                all_pieces &= ~b;

                // place new piece
                piece_arr[square] = piece;
                pieces[piece] |= ((bool)piece) * b;
                colours[Pieces::getColour(piece)] |= ((bool)piece) * b;
                all_pieces |= ((bool)piece) * b;

            }

            void printBoard(bool print_hash = false){
                // loop through pieces and print the piece
                for (int y = 0; y < 8; y++){
                    std::cout << Visuals::Y_COORDS[7 - y];
                    for (int x = 0; x < 8; x++){
                        std::cout << ' ' << Visuals::PIECE_CHARS[getPieceAt((7 - y) * 8 + x)];
                    }
                    std::cout << '\n';
                }
                std::cout << ' ';
                for (int x = 0; x < 8; x++){
                    std::cout << ' ' << Visuals::X_COORDS[x];
                }
                std::cout << '\n';

                if (print_hash){
                    std::cout << "Hash: " << hash << '\n';
                }
            }

            /// @brief  clears the board of all pieces, castling rights, ep, turn and move_stack
            void empty(){
                // clear board
                for (Square s = 0; s <= Squares::H8; s++){
                    piece_arr[s] = Pieces::EMPTY;
                }
                
                all_pieces = 0;
                for (Piece p = 0; p < 16; p++){
                    pieces[p] = 0;
                }
                colours[0] = 0;
                colours[1] = 0;

                ep = 0;

                turn = Colours::WHITE;

                castling_rights[0] = Castling::NO_RIGHTS;
                castling_rights[1] = Castling::NO_RIGHTS;

                move_stack.clear();

                setHash();
            }

            void setStartingBoard(){
                empty();

                // place pieces
                setPieceAt(Squares::A1, Pieces::W_ROOK);
                setPieceAt(Squares::B1, Pieces::W_KNIGHT);
                setPieceAt(Squares::C1, Pieces::W_BISHOP);
                setPieceAt(Squares::D1, Pieces::W_QUEEN);
                setPieceAt(Squares::E1, Pieces::W_KING);
                setPieceAt(Squares::F1, Pieces::W_BISHOP);
                setPieceAt(Squares::G1, Pieces::W_KNIGHT);
                setPieceAt(Squares::H1, Pieces::W_ROOK);

                setPieceAt(Squares::A2, Pieces::W_PAWN);
                setPieceAt(Squares::B2, Pieces::W_PAWN);
                setPieceAt(Squares::C2, Pieces::W_PAWN);
                setPieceAt(Squares::D2, Pieces::W_PAWN);
                setPieceAt(Squares::E2, Pieces::W_PAWN);
                setPieceAt(Squares::F2, Pieces::W_PAWN);
                setPieceAt(Squares::G2, Pieces::W_PAWN);
                setPieceAt(Squares::H2, Pieces::W_PAWN);


                setPieceAt(Squares::A8, Pieces::B_ROOK);
                setPieceAt(Squares::B8, Pieces::B_KNIGHT);
                setPieceAt(Squares::C8, Pieces::B_BISHOP);
                setPieceAt(Squares::D8, Pieces::B_QUEEN);
                setPieceAt(Squares::E8, Pieces::B_KING);
                setPieceAt(Squares::F8, Pieces::B_BISHOP);
                setPieceAt(Squares::G8, Pieces::B_KNIGHT);
                setPieceAt(Squares::H8, Pieces::B_ROOK);

                setPieceAt(Squares::A7, Pieces::B_PAWN);
                setPieceAt(Squares::B7, Pieces::B_PAWN);
                setPieceAt(Squares::C7, Pieces::B_PAWN);
                setPieceAt(Squares::D7, Pieces::B_PAWN);
                setPieceAt(Squares::E7, Pieces::B_PAWN);
                setPieceAt(Squares::F7, Pieces::B_PAWN);
                setPieceAt(Squares::G7, Pieces::B_PAWN);
                setPieceAt(Squares::H7, Pieces::B_PAWN);
                
                
                // set castling rights
                castling_rights[Colours::WHITE] = Castling::ALL_RIGHTS;
                castling_rights[Colours::BLACK] = Castling::ALL_RIGHTS;

                // set turn
                turn = Colours::WHITE;
                
                // set ep
                ep = 0;

                setHash();
            }


            /// @brief returns whether an en passant is possible due to a rook pinning it horzontaly. Ignores bishops pinning it because bishops cant pin it if it was played and not set up
            /// @return 
            bool isEpPinned(){
                const Square k_square = Bitboards::getNextSquare(pieces[Pieces::gen(Pieces::W_KING, turn)]);
                // check if the king is at rank 3 or rank 6
                if (!(((turn) ? Bitboards::RANK4_BB : Bitboards::RANK5_BB) & pieces[Pieces::gen(Pieces::W_KING, turn)])){
                    // king is not on the same rank as ep
                    return false;
                }

                Bitboard kbb = pieces[Pieces::gen(Pieces::W_KING, turn)];
                const Bitboard enemy_horizontals = pieces[Pieces::gen(Pieces::W_ROOK, !turn)] | pieces[Pieces::gen(Pieces::W_QUEEN, !turn)];

                // loop from the king through the one above/below ep square
                // 2 cases: ep.x > king.x or ep.x < king.x
                if (Squares::getX(ep) < Squares::getX(k_square)){
                    // loop from the king to file A (0)
                    for (int x = 0; x < Squares::getX(k_square); x++){
                        // shift bb
                        kbb |= kbb >> 1;

                        if (enemy_horizontals & kbb){
                            // found a rook or a queen. If there are 4 pieces between them then its pinned (4 pieces = king, our pawn, their pawn, rook/queen)
                            return Bitboards::countBits(all_pieces & kbb) == 4;
                        }
                    }
                    // didnt find a rook or queen. return false
                    return false;
                }
                else{
                    // loop from the king to file A (0)
                    for (int x = 0; x < 7 - Squares::getX(k_square); x++){
                        // shift bb
                        kbb |= kbb << 1;

                        if (enemy_horizontals & kbb){
                            // found a rook or a queen. If there are 4 pieces between them then its pinned (4 pieces = king, our pawn, their pawn, rook/queen)
                            return Bitboards::countBits(all_pieces & kbb) == 4;
                        }
                    }
                    // didnt find a rook or queen. return false
                    return false;
                }

                // ep is bad. it is at the kings x so the king is where the pawn should be. return false
                return false;
            }


            void setFen(std::string fen){
                empty();

                Square sq = Squares::A8;
                char   fen_char;
                int    i = 0;


                while (i < fen.length()){
                    fen_char = fen[i];
                    i++;
                    if (isdigit(fen_char)){
                        sq += Directions::EAST * (fen_char - '0');
                    }
                    else if (fen_char == '/'){
                        sq += Directions::SOUTH * 2;
                    }
                    else if (fen_char == ' '){
                        break;
                    }
                    else{
                        setPieceAt(sq,
                                (fen_char == 'p') ? Pieces::B_PAWN   :
                                (fen_char == 'P') ? Pieces::W_PAWN   :
                                (fen_char == 'n') ? Pieces::B_KNIGHT :
                                (fen_char == 'N') ? Pieces::W_KNIGHT :
                                (fen_char == 'b') ? Pieces::B_BISHOP :
                                (fen_char == 'B') ? Pieces::W_BISHOP :
                                (fen_char == 'r') ? Pieces::B_ROOK   :
                                (fen_char == 'R') ? Pieces::W_ROOK   :
                                (fen_char == 'q') ? Pieces::B_QUEEN  :
                                (fen_char == 'Q') ? Pieces::W_QUEEN  :
                                (fen_char == 'k') ? Pieces::B_KING   :
                                (fen_char == 'K') ? Pieces::W_KING   :
                                Pieces::EMPTY
                                );
                        sq += Directions::EAST;
                    }
                }

                if (fen[i] == ' '){
                    i++;
                }

                turn = (fen[i] == 'w') ? Colours::WHITE : Colours::BLACK;


                i++;


                while (i < fen.length()){
                    i++;
                    if (fen[i] == ' '){break;}
                    else if (fen[i] == 'K'){castling_rights[Colours::WHITE] |= Castling::KINGSIDE;}
                    else if (fen[i] == 'Q'){castling_rights[Colours::WHITE] |= Castling::QUEENSIDE;}
                    else if (fen[i] == 'k'){castling_rights[Colours::BLACK] |= Castling::KINGSIDE;}
                    else if (fen[i] == 'q'){castling_rights[Colours::BLACK] |= Castling::QUEENSIDE;}
                }

                i += 1;

                if (fen[i] != '-'){
                    ep = fen[i] - 'a' + ((turn == Colours::WHITE) ? Squares::A6 : Squares::A3);
                }

                if (isEpPinned()){
                    ep = 0;
                }

                setHash();

            }

            /// @brief prints the values of each bitboard, ep, castling rights, turn, and the previous past_moves moves
            void debug(int past_moves = 0){
                std::cout << "all pieces " << all_pieces << '\n';
                std::cout << "colours w  " << colours[Colours::WHITE] << '\n';
                std::cout << "colours b  " << colours[Colours::BLACK] << '\n';
                for (Piece p = 0; p < 16; p++){
                    std::cout << "pieces " << Visuals::PIECE_CHARS[p] << " " << pieces[p] << '\n';
                }
                std::cout << "castling w" << (int)castling_rights[Colours::WHITE] << '\n';
                std::cout << "castling b" << (int)castling_rights[Colours::BLACK] << '\n';
                std::cout << "turn (0 = w) " << turn << '\n';
                std::cout << "ep " << (int)ep << '\n';

                std::cout << "Check:     " << inCheck() << '\n';
                std::cout << "Checkmate: " << isCheckmate() << '\n';

                for (int i = 0; i < std::min(past_moves, (int)move_stack.size()); i++){
                    std::cout << "Past move " << i << ":\t" << Visuals::moveToString(move_stack[move_stack.size() - i - 1].move) <<
                                 " \n\t\tcaptured: " << (int)move_stack[move_stack.size() - i - 1].captured <<
                                 " \n\t\tmoved: "    << (int)move_stack[move_stack.size() - i - 1].moved <<
                                 " \n\t\tep: "       << (int)move_stack[move_stack.size() - i - 1].ep <<
                                 " \n\t\tw castl: "  << (int)move_stack[move_stack.size() - i - 1].w_castling_rights <<
                                 " \n\t\tb castl: "  << (int)move_stack[move_stack.size() - i - 1].b_castling_rights << '\n';
                }
                // print a pgn
                for (int i = 0; i < move_stack.size(); i++){
                    std::cout << Visuals::moveToString(move_stack[i].move) << ' ';
                }
                std::cout << '\n';


                std::cout << "Hash: " << hash << '\n';
            }


            void playMove(Move move){
                
                Square to   = Moves::to  (move);
                Square from = Moves::from(move);

                Piece moved_piece    = getPieceAt(from);
                Piece captured_piece = getPieceAt(to);

                bool pawn_moved = moved_piece == (Pieces::W_PAWN | 8 * turn);
                bool is_ep      = pawn_moved && to == ep && ep;
                
                // add the move info to the move stack
                move_stack.emplace_back(move, castling_rights[Colours::WHITE], castling_rights[Colours::BLACK], ep, moved_piece, is_ep ? Pieces::UNDO_EP_PIECE : captured_piece, hash);

                // replace the piece at the new square. If promoting replace with the piece its promoting to
                replacePieceAt(to, Moves::isPromotion(move) ? (Moves::promotionPiece(move) | 8 * turn) : moved_piece);
                hash ^= Zobrist::PIECE_KEYS[to][captured_piece];
                hash ^= Zobrist::PIECE_KEYS[to][Moves::isPromotion(move) ? (Moves::promotionPiece(move) | 8 * turn) : moved_piece];
                
                // remove the piece
                removePieceAt(from);
                hash ^= Zobrist::PIECE_KEYS[from][moved_piece];
                
                // check if en passant
                if (is_ep){
                    // remove piece 1 below one where the pawn moved to
                    removePieceAt(to - (turn == Colours::WHITE ? Directions::NORTH : Directions::SOUTH));
                    hash ^= Zobrist::PIECE_KEYS[to - (turn == Colours::WHITE ? Directions::NORTH : Directions::SOUTH)][Pieces::oppColour(moved_piece)];
                }
                
                
                if (ep){
                    // remove ep from hash
                    hash ^= Zobrist::EP_KEYS[Squares::getX(ep)];
                }
                ep = (pawn_moved && (turn == Colours::WHITE ? (to - from == Directions::NORTH2) : (to - from == Directions::SOUTH2))) ? ((turn == Colours::WHITE) ? (to + Directions::SOUTH) : (to + Directions::NORTH)) : 0;
                ep = isEpPinned() ? 0 : ep; // remove pinned eps
                ep = (((turn == Colours::WHITE) ?
                        Bitboards::getPawnAttacks<Colours::WHITE>(Bitboards::getBit(ep)) :
                        Bitboards::getPawnAttacks<Colours::BLACK>(Bitboards::getBit(ep)))
                     & pieces[Pieces::gen(Pieces::W_PAWN, !turn)]) ? ep : 0; // if a pawn can take then its ep otherwise its not
                if (ep){
                    // add new ep from hash
                    hash ^= Zobrist::EP_KEYS[Squares::getX(ep)];
                }

                // if castling move the rook
                if (Moves::castling<true>(move)){
                    // remove rook
                    removePieceAt(to + 1);
                    // place rook
                    setPieceAt(to - 1, moved_piece - 2);

                    // hashing update
                    hash ^= Zobrist::PIECE_KEYS[to + 1][moved_piece - 2];
                    hash ^= Zobrist::PIECE_KEYS[to - 1][moved_piece - 2];
                }
                else if (Moves::castling<false>(move)){
                    // std::cout << "queenside castle\n";
                    // remove rook
                    removePieceAt(to - 2);
                    // place rook
                    setPieceAt(to + 1, moved_piece - 2);

                    // hashing update
                    hash ^= Zobrist::PIECE_KEYS[to + 1][moved_piece - 2];
                    hash ^= Zobrist::PIECE_KEYS[to - 2][moved_piece - 2];
                }

                uint8_t white_queenside = (from == Squares::A1 || to == Squares::A1) * Castling::QUEENSIDE;
                uint8_t white_kingside  = (from == Squares::H1 || to == Squares::H1);
                uint8_t black_queenside = (from == Squares::A8 || to == Squares::A8) * Castling::QUEENSIDE;
                uint8_t black_kingside  = (from == Squares::H8 || to == Squares::H8);


                uint8_t w_castling_rights_before = castling_rights[Colours::WHITE]; // save castling rights for hashing update
                uint8_t b_castling_rights_before = castling_rights[Colours::BLACK]; // save castling rights for hashing update
                
                // remove castling rights
                castling_rights[Colours::WHITE] &= ~(white_queenside | white_kingside);
                castling_rights[Colours::BLACK] &= ~(black_queenside | black_kingside);
                castling_rights[turn] &= ((moved_piece & 7) != Pieces::W_KING) * Castling::ALL_RIGHTS;

                hash ^= Zobrist::W_CASTLE[w_castling_rights_before ^ castling_rights[Colours::WHITE]];
                hash ^= Zobrist::B_CASTLE[b_castling_rights_before ^ castling_rights[Colours::BLACK]];


                // change the turn
                turn = !turn;
                hash ^= Zobrist::TURN_KEY;
            }


            void undoMove(){
                if (move_stack.empty()){
                    return;
                }

                UndoMoveInfo move_info = move_stack[move_stack.size() - 1];
                move_stack.pop_back();

                turn = !turn;

                Square to   = Moves::to  (move_info.move);
                Square from = Moves::from(move_info.move);
                Piece moved = move_info.moved;
                Move move   = move_info.move;

                if (move_info.captured == Pieces::UNDO_EP_PIECE){
                    setPieceAt(to + turn * Directions::NORTH + (!turn) * Directions::SOUTH, Pieces::W_PAWN | 8 * (!turn));
                    // std::cout << "undo ep: " << (int)(to + turn * Directions::NORTH + (!turn) * Directions::SOUTH) << " " << (int)(Pieces::W_PAWN | 8 * (!turn)) << '\n';
                    replacePieceAt(to,   Pieces::EMPTY);
                }
                else{
                    replacePieceAt(to,   move_info.captured);
                }
                setPieceAt(from, moved);

                ep = move_info.ep;

                castling_rights[Colours::WHITE] = move_info.w_castling_rights;
                castling_rights[Colours::BLACK] = move_info.b_castling_rights;

                // undo castling moving rooks
                if (Moves::castling<true>(move)){
                    // remove the rook at to - 1
                    removePieceAt(Moves::to(move) - 1);
                    // place the rook at to + 1
                    setPieceAt(Moves::to(move) + 1, moved - 2);
                }
                else if (Moves::castling<false>(move)){
                    // remove the rook at to + 1
                    removePieceAt(Moves::to(move) + 1);
                    // place the rook at to - 2
                    setPieceAt(Moves::to(move) - 2, moved - 2);
                }

                hash = move_info.hash;
            }

            
            /// @brief returns the ep square of the current board
            /// @return 
            Square playNull(){
                // change turn, undo ep
                Square saved_ep = ep;
                ep = 0;
                turn = !turn;

                // change hash
                if (saved_ep){
                    hash ^= Zobrist::EP_KEYS[Squares::getX(saved_ep)];
                }
                hash ^= Zobrist::TURN_KEY;
                
                return saved_ep;
            }
            
            void undoNull(Square ep_){
                // change turn, set ep
                turn = !turn;
                ep = ep_;
                
                // change hash
                if (ep){
                    hash ^= Zobrist::EP_KEYS[Squares::getX(ep)];
                }
                hash ^= Zobrist::TURN_KEY;
            }


            void setHash(){
                // very slow, only use when setting a certain fen onto the board
                hash = 0;
        
                for (Square square = Squares::A1; square <= Squares::H8; square++){
                    Piece piece = getPieceAt(square);
        

                    if (piece == Pieces::EMPTY) continue;
                    hash ^= Zobrist::PIECE_KEYS[square][piece];
        
                }
        
                hash ^= turn ? Zobrist::TURN_KEY : 0;
        
                hash ^= Zobrist::W_CASTLE[castling_rights[Colours::WHITE]];
                hash ^= Zobrist::B_CASTLE[castling_rights[Colours::BLACK]];
        
                if (ep){
                    hash ^= Zobrist::EP_KEYS[Squares::getX(ep)];
                }
            }


            bool inCheck(){
                if (turn == Colours::WHITE){
                    return inCheck<Colours::WHITE>();
                }
                return inCheck<Colours::BLACK>();
            }

            template<Colour template_turn>
            bool inCheck(){
                Square king_square = kingPos<template_turn>();
                Bitboard checkers = (
                    (Bitboards::getRookAttacks  (all_pieces, king_square) & (pieces[(template_turn * 8) ^ Pieces::B_ROOK]   | pieces[(template_turn * 8) ^ Pieces::B_QUEEN])) |
                    (Bitboards::getBishopAttacks(all_pieces, king_square) & (pieces[(template_turn * 8) ^ Pieces::B_BISHOP] | pieces[(template_turn * 8) ^ Pieces::B_QUEEN])) |
                    (Bitboards::getKnightAttacks(king_square) & pieces[(turn * 8) ^ Pieces::B_KNIGHT]) |
                    (Bitboards::getPawnAttacks<template_turn>(pieces[(template_turn * 8) | Pieces::W_KING]) & pieces[(template_turn * 8) ^ Pieces::B_PAWN])
                   ) & colours[!template_turn];

                return checkers;
            }

            template <Colour turn_>
            Bitboard getPins(){
                Square king_square = kingPos<turn_>();
                Bitboard potential_pins = (Bitboards::getRookAttacks(all_pieces, king_square) | Bitboards::getBishopAttacks(all_pieces, king_square)) & colours[turn_];

                Bitboard pinners        = ((Bitboards::getRookAttacks  (all_pieces ^ potential_pins, king_square) & (pieces[(turn_ * 8) ^ Pieces::B_ROOK]   | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])) |
                                           (Bitboards::getBishopAttacks(all_pieces ^ potential_pins, king_square) & (pieces[(turn_ * 8) ^ Pieces::B_BISHOP] | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])));

                Bitboard pinned_pieces  = 0;

                while (pinners){
                    pinned_pieces |= Bitboards::bitboardBetween(Bitboards::popNextSquare(pinners), king_square);
                }

                pinned_pieces &= potential_pins;

                return pinned_pieces;
            }


            Board(){
                setStartingBoard();
            }

            Board(std::string fen){
                setFen(fen);
            }

            template<Colour c>
            Square kingPos(){
                if (c == Colours::WHITE){
                    return Bitboards::getNextSquare(pieces[Pieces::W_KING]);
                }
                return Bitboards::getNextSquare(pieces[Pieces::B_KING]);
            }


            bool repeated(){
                // loop back through past hashes. if it equals the current hash, return true
                for (int i = move_stack.size() - 1; i >= 0; i--){
                    if (move_stack[i].hash == hash){
                        return true;
                    }
                    // if there was a change in castling rights, captured piece or moved a pawn, return false
                    if ((Pieces::typeOf(move_stack[i].moved) == Pieces::W_PAWN) ||
                        (move_stack[i].captured != Pieces::EMPTY) ||
                        (move_stack[i].w_castling_rights != castling_rights[Colours::WHITE]) ||
                        (move_stack[i].b_castling_rights != castling_rights[Colours::BLACK])){
                            return false;
                        }
                }

                return false;
            }

            bool insufficientMaterial(){
                // get count of bishops and knights on each side
                Bitboard kings = pieces[Pieces::W_KING] | pieces[Pieces::B_KING];
                
                if (!(all_pieces & ~kings)) return true; // theres no pieces left except kings, return true
                Bitboard pawns  = pieces[Pieces::W_PAWN ] | pieces[Pieces::B_PAWN ];
                Bitboard rooks  = pieces[Pieces::W_ROOK ] | pieces[Pieces::B_ROOK ];
                Bitboard queens = pieces[Pieces::W_QUEEN] | pieces[Pieces::B_QUEEN];
                if (pawns || rooks || queens) return false; // theres a pawn, rook or queen, return false

                int w_knights = Bitboards::countBits(pieces[Pieces::W_KNIGHT]);
                int w_bishops = Bitboards::countBits(pieces[Pieces::W_BISHOP]);
                int b_knights = Bitboards::countBits(pieces[Pieces::B_KNIGHT]);
                int b_bishops = Bitboards::countBits(pieces[Pieces::B_BISHOP]);

                if (w_bishops && w_knights) return false; // 1 bishop 1 knight is sufficient so any non 0 amount of knights with any non 0 amount of bishops is sufficient
                if (b_bishops && b_knights) return false;

                // there has to be 3 or more knights for a non draw
                if (w_knights >= 3 || b_knights >= 3) return false;

                // if theres bishops on both coloured squares return false
                if ((pieces[Pieces::W_BISHOP] & Bitboards::LIGHT_SQUARES) && (pieces[Pieces::W_BISHOP] & ~Bitboards::LIGHT_SQUARES)) return false;
                if ((pieces[Pieces::B_BISHOP] & Bitboards::LIGHT_SQUARES) && (pieces[Pieces::B_BISHOP] & ~Bitboards::LIGHT_SQUARES)) return false;

                return true;

            }

            /// @brief returns the bitboard of attacked squares if the king of !attacker wasnt there
            /// @tparam attacker 
            /// @return 
            template<Colour attacker>
            Bitboard attackedBitboardNoKing(){
                using namespace Bitboards;
                // returns a bitboard of where pieces of attacker attacks if there werent a king of not attacker on the board
                Bitboard all_pieces_ = all_pieces ^ pieces[Pieces::gen(Pieces::W_KING, !attacker)];

                Bitboard attacked = Bitboards::getPawnAttacks<attacker>(pieces[Pieces::gen(Pieces::W_PAWN, attacker)]);

                attacked |= Bitboards::getKnightAttacks(pieces[Pieces::gen(Pieces::W_KNIGHT, attacker)]);
                
                attacked |= Bitboards::getBishopAttackedSquares(all_pieces_, pieces[Pieces::gen(Pieces::W_BISHOP, attacker)] | pieces[Pieces::gen(Pieces::W_QUEEN, attacker)]);

                attacked |= Bitboards::getRookAttackedSquares  (all_pieces_, pieces[Pieces::gen(Pieces::W_ROOK,   attacker)] | pieces[Pieces::gen(Pieces::W_QUEEN, attacker)]);
                
                attacked |= Bitboards::getKingAttacks(pieces[Pieces::gen(Pieces::W_KING, attacker)]);

                return attacked;
            }

            /// @brief returns if the board is in checkmate
            /// @tparam turn_ the player whose turn it is
            /// @return 
            template<Colour turn_>
            bool isCheckmate(){
                // find checkers. See if the king can move, other pieces can take the checker or block the check
                // need to find the side to plays pinned pieces
                // need to find the opponents sides squares attacked
                // need to find checkers and lines between the checkers and king
                Square king_square = kingPos<turn_>();

                // get pins and checkers
                Bitboard king_rook_attacks   = Bitboards::getRookAttacks  (all_pieces, king_square);
                Bitboard king_bishop_attacks = Bitboards::getBishopAttacks(all_pieces, king_square);

                
                // get checkers
                Bitboard checkers = (
                                     (king_rook_attacks   & (pieces[(turn_ * 8) ^ Pieces::B_ROOK]   | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])) |
                                     (king_bishop_attacks & (pieces[(turn_ * 8) ^ Pieces::B_BISHOP] | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])) |
                                     (Bitboards::getKnightAttacks(king_square) & pieces[(turn_ * 8) ^ Pieces::B_KNIGHT]) |
                                     (Bitboards::getPawnAttacks<turn_>(pieces[(turn_ * 8) | Pieces::W_KING]) & pieces[(turn_ * 8) ^ Pieces::B_PAWN])
                                    ) & colours[!turn_];

                                    
                if (Bitboards::countBits(checkers) == 0) return false;
                Bitboard not_us = ~colours[turn_];
                Bitboard attacked_squares = attackedBitboardNoKing<!turn_>();
                if (Bitboards::countBits(checkers) >= 2) return !(Bitboards::getKingAttacks(pieces[(turn_ * 8) | Pieces::W_KING]) & not_us & ~attacked_squares); // return if theres any legal king moves
                
                Bitboard stop_checks = (Bitboards::bitboardBetween(Bitboards::getNextSquare(checkers), king_square) | checkers) & ~Bitboards::getBit(king_square); // | checkers to add knights
                
                // single check, if the king has moves, return false
                if (Bitboards::getKingAttacks(pieces[(turn_ * 8) | Pieces::W_KING]) & not_us & ~attacked_squares){
                    return false;
                }

                // return true if theres nothing able to block the check
                Bitboard potential_pins = (king_rook_attacks | king_bishop_attacks) & colours[turn_];

                Bitboard pinners        = ((Bitboards::getRookAttacks  (all_pieces ^ potential_pins, king_square) & (pieces[(turn_ * 8) ^ Pieces::B_ROOK]   | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])) |
                                           (Bitboards::getBishopAttacks(all_pieces ^ potential_pins, king_square) & (pieces[(turn_ * 8) ^ Pieces::B_BISHOP] | pieces[(turn_ * 8) ^ Pieces::B_QUEEN])));

                Bitboard pinned  = 0;

                while (pinners){
                    pinned |= Bitboards::bitboardBetween(Bitboards::popNextSquare(pinners), king_square);
                }

                pinned &= potential_pins;

                // check pawns
                Bitboard ep_bb = Bitboards::getBit(ep) & ~1;
                Bitboard legal_eps = (stop_checks & checkers) | ((((turn == Colours::WHITE) ? (stop_checks << 8) : (stop_checks >> 8))) & ep_bb);
                Bitboard pawns_forward = Bitboards::shiftBB<(turn_ == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH> (pieces[Pieces::gen(Pieces::W_PAWN, turn_)] & ~pinned) & ~all_pieces;
                if (
                    Bitboards::getPawnAttacks<turn_>(pieces[Pieces::gen(Pieces::W_PAWN, turn_)] & ~pinned) & legal_eps || // if a pawn can captures the attacker
                    pawns_forward & stop_checks || // pawns moving forwards
                    Bitboards::shiftBB<(turn_ == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(pawns_forward & ((turn_ == Colours::WHITE) ? Bitboards::RANK3_BB : Bitboards::RANK6_BB)) & stop_checks & ~all_pieces // pawn moving forwards twice
                ){
                    // std::cout << "Ca: " << (Bitboards::getPawnAttacks<turn_>(pieces[Pieces::gen(Pieces::W_PAWN, turn_)] & ~pinned) & legal_eps) << '\n';
                    // std::cout << "F1: " << (pawns_forward & stop_checks) << '\n';
                    // std::cout << "F2: " << (Bitboards::shiftBB<(turn_ == Colours::WHITE) ? Directions::NORTH : Directions::SOUTH>(pawns_forward & ((turn_ == Colours::WHITE) ? Bitboards::RANK3_BB : Bitboards::RANK6_BB)) & stop_checks & ~all_pieces) << '\n';
                    // std::cout << "Pawn stops\n";
                    return false;
                }
                // check knights
                if (Bitboards::getKnightAttacks(pieces[Pieces::gen(Pieces::W_KNIGHT, turn_)] & ~pinned) & stop_checks){
                    // std::cout << "Knight stops\n";
                    return false;
                }
                // check bishops and queens
                if (Bitboards::getBishopAttackedSquares(all_pieces, (pieces[Pieces::gen(Pieces::W_BISHOP, turn_)] | pieces[Pieces::gen(Pieces::W_QUEEN, turn_)]) & ~pinned) & stop_checks){
                    // std::cout << "Bishop/queen stops\n";
                    return false;
                }
                // check rooks and queens
                if (Bitboards::getRookAttackedSquares  (all_pieces, (pieces[Pieces::gen(Pieces::W_ROOK,   turn_)] | pieces[Pieces::gen(Pieces::W_QUEEN, turn_)]) & ~pinned) & stop_checks){
                    // std::cout << "Rook/queen stops\n";
                    return false;
                }

                return true;
            }


            /// @brief returns true if the board is in checkmate
            /// @return 
            bool isCheckmate(){
                if (turn == Colours::WHITE){
                    return isCheckmate<Colours::WHITE>();
                }
                return isCheckmate<Colours::BLACK>();
            }
    };


    namespace Visuals{
        
        Move stringToMove(std::string move_str, Board& board){
            Square from = stringToSquare(move_str.substr(0, 2));
            Square to   = stringToSquare(move_str.substr(2, 2));
            if (move_str.length() == 5){
                switch(move_str[4]){
                    case 'n': return Moves::makeMove<0, 0, Pieces::W_KNIGHT>(from, to);
                    case 'b': return Moves::makeMove<0, 0, Pieces::W_BISHOP>(from, to);
                    case 'r': return Moves::makeMove<0, 0, Pieces::W_ROOK>  (from, to);
                    case 'q': return Moves::makeMove<0, 0, Pieces::W_QUEEN> (from, to);
                }
            }

            if ((move_str == "e1g1") && board.getPieceAt(from) == Pieces::W_KING) return Moves::makeMove<1, 0>(from, to);
            if ((move_str == "e1c1") && board.getPieceAt(from) == Pieces::W_KING) return Moves::makeMove<0, 1>(from, to);
            if ((move_str == "e8g8") && board.getPieceAt(from) == Pieces::B_KING) return Moves::makeMove<1, 0>(from, to);
            if ((move_str == "e8c8") && board.getPieceAt(from) == Pieces::B_KING) return Moves::makeMove<0, 1>(from, to);

            return Moves::makeMove(from, to);
        }
    } // namespace Visuals
    
} // namespace Chess

#endif
