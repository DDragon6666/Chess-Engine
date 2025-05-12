
# exit()

import csv
import chess

csv_file_path = ""

all_puzzle_types = (
    "dovetailMate",
    "deflection",
    "arabianMate",
    "capturingDefender",
    "pin",
    "hookMate",
    "advancedPawn",
    "masterVsMaster",
    "mate",
    "mateIn2",
    "mateIn1",
    "oneMove",
    "vukovicMate",
    "long",
    "exposedKing",
    "attackingF2F7",
    "crushing",
    "superGM",
    "queenEndgame",
    "knightEndgame",
    "interference",
    "endgame",
    "opening",
    "equality",
    "rookEndgame",
    "bodenMate",
    "sacrifice",
    "queensideAttack",
    "hangingPiece",
    "backRankMate",
    "fork",
    "bishopEndgame",
    "kingsideAttack",
    "zugzwang",
    "enPassant",
    "middlegame",
    "killBoxMate",
    "underPromotion",
    "xRayAttack",
    "doubleBishopMate",
    "anastasiaMate",
    "intermezzo",
    "pawnEndgame",
    "attraction",
    "quietMove",
    "master",
    "doubleCheck",
    "short",
    "skewer",
    "mateIn4",
    "trappedPiece",
    "veryLong",
    "mateIn3",
    "defensiveMove",
    "queenRookEndgame",
    "mateIn5",
    "smotheredMate",
    "clearance",
    "castling",
    "discoveredAttack",
    "advantage",
    "promotion"
)


"""
    Lengths: (1 per puzzle already done by lichess (some have 0 so will get nolength length))
        oneMove
        short
        long
        veryLong
        nolength
"""

"""
    Save puzzle types wont have puzzle lengths (will be added on later)
    Who played the game doesnt matter
    Different types of mates doesnt matter (depth of mate does matter)
"""

puzzle_lengths = (
    "oneMove",
    "short",
    "long",
    "veryLong",
    "noLength"
)

save_puzzle_types = (
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
)

save_puzzle_types_set = set(save_puzzle_types)

print(len(save_puzzle_types))

added = 0

def save_puzzles(file_name: str, puzzles):
    global added
    file_name = file_name.lower()
    print("not saving files")
    # with open(file_name, "a") as f:
    #     for puzzle in puzzles:
    #         added += 1
    #         f.write(puzzle + "\n")

board = chess.Board()
with open(csv_file_path, newline="") as csvfile:
    
    spamreader = csv.reader(csvfile, delimiter=",", quotechar="|")
    i = 0
    expected_added = 0

    puzzles = {} # stores the 

    for row in spamreader:
        if (not i):
            i += 1
            continue
        # if (i > 1000000):
        #     break
        i += 1

        if (i % 25000 == 1):
            print("Puzzle", i - 1)
        row_list = (str(row)).split("', '")
        fen = row_list[1]
        moves = row_list[2].split()
        puzzle_info = row_list[7].split()
        board.set_fen(fen)
        board.push(chess.Move.from_uci(moves[0]))
        fen = board.fen()
        move = moves[1]

        puzzle_length = 4
        for ptype in puzzle_info:
            if   (ptype == "oneMove" ):
                puzzle_length = 0
                break
            elif (ptype == "short"   ):
                puzzle_length = 1
                break
            elif (ptype == "long"    ):
                puzzle_length = 2
                break
            elif (ptype == "veryLong"):
                puzzle_length = 3
                break

        # for each puzzle type, open that file and add the new fen and best move in
        for ptype in puzzle_info:
            if (ptype in save_puzzle_types_set):
                expected_added += 1
                puzzle_type = puzzle_lengths[puzzle_length] + "_" + ptype + ".txt"
                if (puzzles.get(puzzle_type) is None):
                    puzzles[puzzle_type] = []
                puzzles.get(puzzle_type).append(fen + "," + move)
                if (len(puzzles.get(puzzle_type)) == 1000):
                    save_puzzles("engine\\test_positions\\" + puzzle_type, puzzles.get(puzzle_type))
                    puzzles[puzzle_type] = []

    for puzzle in puzzles.keys():
        save_puzzles("engine\\test_positions\\" + puzzle, puzzles.get(puzzle))

    print("E:", expected_added, " A:", added)