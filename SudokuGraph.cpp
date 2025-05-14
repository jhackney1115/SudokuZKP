#include "SudokuGraph.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <set>

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

bool SudokuGraph::respondToChallenge(const vector<pair<Cell, Cell>>& challenge) const {
    for (const auto& pair : challenge) {
        Cell cellA = pair.first;
        Cell cellB = pair.second;

        int valueA = coloring.at(cellA);
        int valueB = coloring.at(cellB);

        cout << cellA.first << " " << cellA.second << endl;
        cout << cellB.first << " " << cellB.second << endl;
        cout << valueA << " " << valueB << endl;
        cout << endl;

        string nonceA = commitments.at(cellA).nonce;
        string nonceB = commitments.at(cellB).nonce;

        string recomputedCommitmentA = hashCommitment(valueA, nonceA);
        string recomputedCommitmentB = hashCommitment(valueB, nonceB);

        if (recomputedCommitmentA != commitments.at(cellA).hash ||
            recomputedCommitmentB != commitments.at(cellB).hash) {
            cout << "Commitment verification failed for pair: ("
                 << cellA.first+1 << "," << cellA.second+1 << ") and ("
                 << cellB.first+1 << "," << cellB.second+1 << ")\n";
            return false;
        }

        if (valueA == valueB) {
            cout << "Conflict found: same value in adjacent cells: ("
                 << cellA.first+1 << "," << cellA.second+1 << ") = "
                 << valueA << " and ("
                 << cellB.first+1 << "," << cellB.second+1 << ") = "
                 << valueB << "\n";
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

bool SudokuGraph::verifyAndProve(const vector<vector<int>>&startingBoard, const vector<vector<int>>& answer) {
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

    /*if (!isValidSudoku(answer)) {
        cerr << "Sudoku rule violation detected.\n";
        return false;
    }*/

    colorGraph(answer);
    commitColors();
    
    // Loop multiple times for stronger ZKP simulation
    const int numRounds = 100;
    random_device rd;
    mt19937 g(rd());

    //Generate all edges
    vector<pair<Cell, Cell>> allEdges;
    for (const auto& pair : adjList) {
        Cell from = pair.first;
        for (const Cell& to : pair.second) {
            if (from < to)
                allEdges.push_back({from, to});
        }
    }

    /*Cell Test1={0,1};
    Cell Test2={0,2};
    vector<pair<Cell, Cell>> Check;
    Check.push_back({Test1, Test2});
    if (!respondToChallenge(Check)) {
            cerr << "ZKP challenge failed.\n";
            return false;
    }*/

     for (int round = 0; round < numRounds; ++round) {
        vector<pair<Cell, Cell>> challenge;

        // Structured challenge type: row, col, or box
        int type = round % 3; // 0 = row, 1 = col, 2 = box
        int index = g() % 9;

        if (type == 0) { // row
            for (int i = 0; i < 9; ++i)
                for (int j = i + 1; j < 9; ++j)
                    challenge.emplace_back(Cell{index, i}, Cell{index, j});
        } else if (type == 1) { // column
            for (int i = 0; i < 9; ++i)
                for (int j = i + 1; j < 9; ++j)
                    challenge.emplace_back(Cell{i, index}, Cell{j, index});
        } else { // box
            int boxRow = (index / 3) * 3;
            int boxCol = (index % 3) * 3;
            vector<Cell> cells;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    cells.emplace_back(Cell{boxRow + i, boxCol + j});
            for (size_t i = 0; i < cells.size(); ++i)
                for (size_t j = i + 1; j < cells.size(); ++j)
                    challenge.emplace_back(cells[i], cells[j]);
        }

        // Add 10 random pairs from full adjacency list as extra
        //random checks
        shuffle(allEdges.begin(), allEdges.end(), g);
        for (int i = 0; i < 10 && i < allEdges.size(); ++i) {
            challenge.push_back(allEdges[i]);
        }

        shuffle(challenge.begin(), challenge.end(), g);
        if (challenge.size() > 20)
            challenge.resize(20);

        if (!respondToChallenge(challenge)) {
            cerr << "ZKP challenge failed at round " << round + 1 << ".\n";
            return false;
        }
    }

    
    return true;
}
