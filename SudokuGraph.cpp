#include "SudokuGraph.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <set>
#include <unordered_map>

using namespace std;

SudokuGraph::SudokuGraph() : adjList(), coloring(), commitments() {
    buildGraph();
}

void SudokuGraph::buildGraph() {
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            Cell c1 = {row, col};
            for (int k = 0; k < N; ++k) {
                if (k != col) adjList[c1].insert({row, k});
                if (k != row) adjList[c1].insert({k, col});
            }
            int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
            for (int i = boxRow; i < boxRow + 3; ++i)
                for (int j = boxCol; j < boxCol + 3; ++j)
                    if (i != row || j != col)
                        adjList[c1].insert({i, j});
        }
    }
}

void SudokuGraph::colorGraph(const vector<vector<int>> &solution) {
    for (int row = 0; row < N; ++row)
        for (int col = 0; col < N; ++col)
            coloring[{row, col}] = solution[row][col];
}

void SudokuGraph::commitColors() {
    srand(static_cast<unsigned>(time(0)));
    for (auto &pair : coloring) {
        string nonce = to_string(rand());
        string value = to_string(pair.second) + nonce;
        commitments[pair.first] = { sha256(value), nonce };
    }
}

void SudokuGraph::setStartingBoard(const int board[9][9]) {
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j)
            startingBoard[i][j] = board[i][j];
}

bool SudokuGraph::respondToChallenge(const vector<pair<Cell, Cell>>& challenge, const unordered_map<int, int>& perm,  const unordered_map<Cell, string, CellHash>& roundNonces) const {
    for (const auto& pair : challenge) {
        Cell cellA = pair.first;
        Cell cellB = pair.second;

        int valueA = coloring.at(cellA);
        int valueB = coloring.at(cellB);

        int permA = perm.at(valueA);
        int permB = perm.at(valueB);

        string nonceA = roundNonces.at(cellA);
        string nonceB = roundNonces.at(cellB);

        string recomputedA = hashCommitment(permA, nonceA);
        string recomputedB = hashCommitment(permB, nonceB);

        if (recomputedA != commitments.at(cellA).hash ||
            recomputedB != commitments.at(cellB).hash) {
            cout << "Commitment verification failed for pair: ("
                 << cellA.first << "," << cellA.second << ") and ("
                 << cellB.first << "," << cellB.second << ")\n";
            return false;
        }

        if (permA == permB) {
            cout << "Conflict found: same permuted value in adjacent cells: ("
                 << cellA.first << "," << cellA.second << ") = " << permA
                 << " and (" << cellB.first << "," << cellB.second << ") = " << permB << "\n";
            return false;
        }
    }
    return true;
}


bool SudokuGraph::isValidSudoku(const vector<vector<int>>& board) {
    for (int i = 0; i < 9; ++i) {
        set<int> row, col;
        for (int j = 0; j < 9; ++j) {
            if (board[i][j] != 0 && !row.insert(board[i][j]).second) {
                cerr << "Duplicate in row " << i << " at column " << j << endl;
                return false;
            }
            if (board[j][i] != 0 && !col.insert(board[j][i]).second) {
                cerr << "Duplicate in column " << i << " at row " << j << endl;
                return false;
            }
        }
    }

    for (int boxRow = 0; boxRow < 9; boxRow += 3) {
        for (int boxCol = 0; boxCol < 9; boxCol += 3) {
            set<int> block;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j) {
                    int val = board[boxRow + i][boxCol + j];
                    if (val != 0 && !block.insert(val).second) {
                        cerr << "Duplicate in 3x3 box starting at ("
                             << boxRow << "," << boxCol << ") at cell ("
                             << (boxRow + i) << "," << (boxCol + j) << ")\n";
                        return false;
                    }
                }
        }
    }

    return true;
}

bool SudokuGraph::verifyAndProve(const vector<vector<int>>& startingBoard, const vector<vector<int>>& answer) {
    //Checks if set values from starting puzzle are changed.
    //Still couldn't grasp how to do this without knowing the values
    //or giving away valuable information that reveals info about the
    //solution
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j) {
            this->startingBoard[i][j] = startingBoard[i][j];
            if (startingBoard[i][j] != 0 && startingBoard[i][j] != answer[i][j]) {
                cerr << "Error: Fixed cell at (" << i << "," << j
                     << ") was modified. Expected: " << startingBoard[i][j]
                     << ", Found: " << answer[i][j] << endl;
                return false;
            }
        }

    colorGraph(answer);

    const int numRounds = 100;
    random_device rd;
    mt19937 g(rd());

    // Precompute all edges
    vector<pair<Cell, Cell>> allEdges;
    for (const auto& pair : adjList) {
        Cell from = pair.first;
        for (const Cell& to : pair.second) {
            if (from < to)
                allEdges.push_back({from, to});
        }
    }

    for (int round = 0; round < numRounds; ++round) {
        vector<pair<Cell, Cell>> challenge;
        unordered_set<Cell, CellHash> involvedCells;

        // Structured challenge type: row, col, or box
        int type = round % 3;
        int index = g() % 9;

        if (type == 0) {
            for (int i = 0; i < 9; ++i)
                for (int j = i + 1; j < 9; ++j) {
                    challenge.emplace_back(Cell{index, i}, Cell{index, j});
                    involvedCells.insert({index, i});
                    involvedCells.insert({index, j});
                }
        } else if (type == 1) {
            for (int i = 0; i < 9; ++i)
                for (int j = i + 1; j < 9; ++j) {
                    challenge.emplace_back(Cell{i, index}, Cell{j, index});
                    involvedCells.insert({i, index});
                    involvedCells.insert({j, index});
                }
        } else {
            int boxRow = (index / 3) * 3;
            int boxCol = (index % 3) * 3;
            vector<Cell> cells;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    cells.emplace_back(Cell{boxRow + i, boxCol + j});
            for (size_t i = 0; i < cells.size(); ++i)
                for (size_t j = i + 1; j < cells.size(); ++j) {
                    challenge.emplace_back(cells[i], cells[j]);
                    involvedCells.insert(cells[i]);
                    involvedCells.insert(cells[j]);
                }
        }

        shuffle(allEdges.begin(), allEdges.end(), g);
        for (int i = 0; i < 10 && i < allEdges.size(); ++i) {
            challenge.push_back(allEdges[i]);
            involvedCells.insert(allEdges[i].first);
            involvedCells.insert(allEdges[i].second);
        }

        // Generate fresh permutation and per-round commitments
        vector<int> permVec = {1,2,3,4,5,6,7,8,9};
        shuffle(permVec.begin(), permVec.end(), g);
        unordered_map<int, int> permutation;
        for (int i = 1; i <= 9; ++i)
            permutation[i] = permVec[i - 1];

        unordered_map<Cell, string, CellHash> roundNonces;
        for (const Cell& c : involvedCells) {
            string nonce = to_string(g());
            int permuted = permutation[coloring[c]];
            string hashVal = hashCommitment(permuted, nonce);
            commitments[c] = { hashVal, nonce };
            roundNonces[c] = nonce;
        }

        if (!respondToChallenge(challenge, permutation, roundNonces)) {
            cerr << "ZKP challenge failed at round " << round + 1 << ".\n";
            return false;
        }
    }

    return true;
}
