#ifndef SUDOKUGRAPH_H
#define SUDOKUGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>

// Define constants and types
const int N = 9;
typedef std::pair<int, int> Cell;

// Custom hash function for Cell (pair<int, int>)
struct CellHash {
    std::size_t operator()(const Cell& c) const {
        return std::hash<int>()(c.first) ^ (std::hash<int>()(c.second) << 1);
    }
};

// Struct for commitments (hash + nonce)
struct Commitment {
    std::string hash;
    std::string nonce;

    Commitment() = default;

    Commitment(const std::string& h, const std::string& n)
        : hash(h), nonce(n) {}
};

class SudokuGraph {
public:
    // Data structures with custom CellHash
    std::unordered_map<Cell, std::unordered_set<Cell, CellHash>, 
        CellHash> adjList;
    std::unordered_map<Cell, int, CellHash> coloring;
    std::unordered_map<Cell, Commitment, CellHash> commitments;

    SudokuGraph();

    void colorGraph(const std::vector<std::vector<int>> &solution);
    void commitColors();
    void zeroKnowledgeProof(const std::vector<Cell> &challengeCells);

private:
    void buildGraph();
};

#endif
