#ifndef SUDOKUGRAPH_H
#define SUDOKUGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <utility>

const int N = 9;
typedef std::pair<int, int> Cell;

struct CellHash {
    std::size_t operator()(const Cell& c) const {
        return std::hash<int>()(c.first) ^ (std::hash<int>()(c.second) << 1);
    }
};

struct Commitment {
    std::string hash;
    std::string nonce;

    Commitment() = default;
    Commitment(const std::string& h, const std::string& n)
        : hash(h), nonce(n) {}
};

class SudokuGraph {
public:
    std::unordered_map<Cell, std::unordered_set<Cell, CellHash>, CellHash> adjList;
    std::unordered_map<Cell, int, CellHash> coloring;
    std::unordered_map<Cell, Commitment, CellHash> commitments;

    int startingBoard[9][9];

    SudokuGraph();

    void colorGraph(const std::vector<std::vector<int>> &answer);
    void commitColors();
    void setStartingBoard(const int board[9][9]);
    bool respondToChallenge(const std::vector<std::pair<Cell, Cell>>& challenge, const std::unordered_map<int, int>& perm,  const std::unordered_map<Cell, std::string, CellHash>& roundNonces) const;
    bool verifyAndProve(const std::vector<std::vector<int>>& startingBoard,
                        const std::vector<std::vector<int>>& answer);
    bool isValidSudoku(const std::vector<std::vector<int>>& board);

private:
    void buildGraph();
};

#endif
