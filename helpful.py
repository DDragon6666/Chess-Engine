import chess.pgn
import io


uci = input('uci: ')


board = chess.Board()

uci_moves = uci.split()

for move in uci_moves:
    board.push(chess.Move.from_uci(move))

game = chess.pgn.Game()
game.setup(board.root())

node = game

# Replay the moves in PGN
for move in board.move_stack:
    node = node.add_variation(move)

# Save PGN to a string
pgn_io = io.StringIO()
print(game, file=pgn_io)
pgn_string = pgn_io.getvalue()

print(pgn_string)  # Print the PGN