#include "SudokuGraph.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Constructor with explicit member initialization list
SudokuGraph::SudokuGraph() : adjList(), coloring(), commitments() {
    buildGraph();
}

// Build graph with constraints for Sudoku adjacency
void SudokuGraph::buildGraph() {
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            Cell c1 = {row, col};
            for (int k = 0; k < N; ++k) {
                if (k != col) adjList[c1].insert({row, k});
                if (k != row) adjList[c1].insert({k, col});
            }
            int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
            for (int i = boxRow; i < boxRow + 3; ++i) {
                for (int j = boxCol; j < boxCol + 3; ++j) {
                    if (i != row || j != col)
                        adjList[c1].insert({i, j});
                }
            }
        }
    }
}

// Apply colors (numbers) from Sudoku solution to graph nodes
void SudokuGraph::colorGraph(const vector<vector<int>> &solution) {
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            coloring[{row, col}] = solution[row][col];
        }
    }
}

// Commit colors using SHA-256 hash + random nonce
void SudokuGraph::commitColors() {
    srand(static_cast<unsigned>(time(0)));
    for (auto &pair : coloring) {
        string nonce = to_string(rand());
        string value = to_string(pair.second) + nonce;
        commitments[pair.first] = { sha256(value), nonce };
    }
}

// Respond to a zero-knowledge challenge by revealing colors and nonces for specific cells
void SudokuGraph::zeroKnowledgeProof(const vector<Cell> &challengeCells) {
    cout << "Zero-Knowledge Proof Response:" << endl;
    for (const auto &cell : challengeCells) {
        int color = coloring[cell];
        string nonce = commitments[cell].nonce;
        cout << "Cell (" << cell.first << "," << cell.second << ") → Color: "
             << color << ", Nonce: " << nonce << endl;

        string checkValue = to_string(color) + nonce;
        string checkHash = sha256(checkValue);
        cout << "Hash: " << checkHash << " | Commitment: "
             << commitments[cell].hash << endl;

        if (checkHash != commitments[cell].hash)
            cout << "Mismatch detected!" << endl;
    }
}
