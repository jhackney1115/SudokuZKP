#include "SudokuGraph.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

vector<vector<int>> loadBoardFromFile(const string& filename) {
    ifstream file(filename);
    vector<vector<int>> board(9, vector<int>(9));
    if (!file) {
        cerr << "Error: Cannot open " << filename << endl;
        return board;
        //returns a blank board of 0's
    }
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j)
            file >> board[i][j];
    return board;
}

void printBoard(const vector<vector<int>>& board) {
    for (const auto& row : board) {
        for (int val : row) cout << val << " ";
        cout << endl;
    }
}

int main() {
    // Choose input method
    bool useFileInput = true;  // Set to false to use hardcoded boards

    vector<vector<int>> startingBoard, solution;

    if (useFileInput) {
        startingBoard = loadBoardFromFile("start.txt");
        solution = loadBoardFromFile("solution.txt");
    } else {
        startingBoard = {
            {5,3,0,0,7,0,0,0,0},
            {6,0,0,1,9,5,0,0,0},
            {0,9,8,0,0,0,0,6,0},
            {8,0,0,0,6,0,0,0,3},
            {4,0,0,8,0,3,0,0,1},
            {7,0,0,0,2,0,0,0,6},
            {0,6,0,0,0,0,2,8,0},
            {0,0,0,4,1,9,0,0,5},
            {0,0,0,0,8,0,0,7,9}
        };
        solution = {
            {5,3,4,6,7,8,9,1,2},
            {6,7,2,1,9,5,3,4,8},
            {1,9,8,3,4,2,5,6,7},
            {8,5,9,7,6,1,4,2,3},
            {4,2,6,8,5,3,7,9,1},
            {7,1,3,9,2,4,8,5,6},
            {9,6,1,5,3,7,2,8,4},
            {2,8,7,4,1,9,6,3,5},
            {3,4,5,2,8,6,1,7,9}
        };
    }

    cout << "Starting Board:" << endl;
    printBoard(startingBoard);

    cout << "\nUser Solution:" << endl;
    printBoard(solution);

    SudokuGraph sg;
    bool success = sg.verifyAndProve(startingBoard, solution);

    cout << "\nVerification Result: " << 
    (success ? "VALID solution." : "INVALID solution.") << endl;
    if (!success) {
        cout <<
        "Please check the above errors and correct your solution." 
        << endl;
    }

    return 0;
}
