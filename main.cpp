#include <iostream>
#include <vector>
#include <limits>
#include <unordered_map>
#include <thread>
#include <functional>
using namespace std;
//phenylalanine
const int ROWS = 6, COLS = 7;
const int PLAYER = 2, AI = 1;
const int MAX_DEPTH = 10;
int num_round = 0;

typedef vector<vector<int>> Board;
vector<int> moves = {3, 2, 4, 1, 5, 0, 6}; // move order
unordered_map<string, int> tpost; //transposition table
unordered_map<string, int> book = {
    {"", 3},
    {"33", 3},
    {"3333", 3},
    {"333333", 2},
    {"32", 5}, {"36", 3}, {"35", 5}, {"34", 1},  {"31", 1},  {"30", 3}
}; //book
string movestring = "";
int find_book(string& moves){
    if (book.count(moves)) return book[moves];
    return -1;
}
string make_key(Board& board) {
    string key = "";
    for (auto& row : board)
        for (auto cell : row)
            key += to_string(cell);
    return key;
}
bool is_valid(Board& board, int col) {
    return board[0][col] == 0;
}
void display(Board& board){
    cout << "You are X, CPU is O\n0123456\n";
    for (int i = 0; i < ROWS; i++){
        for (int ii = 0; ii < COLS; ii++){
            switch (board[i][ii]) {
                case 0:
                    cout << '.';
                    break;
                case 1:
                    cout << 'X';
                    break;
                case 2:
                    cout << 'O';
                    break;
                default:
                    cout << '?';
                    break;
            }
        }
        cout << endl;
    }
}
void move(Board& board, int col, int player) {
    for (int row = ROWS-1; row >= 0; --row) {
        if (board[row][col] == 0) {
            board[row][col] = player;
            break;
        }
    }
}
void undo(Board& board, int col) {
    for (int row = 0; row < ROWS; ++row) {
        if (board[row][col] != 0) {
            board[row][col] = 0;
            break;
        }
    }
}
void plr_win(Board& board, int plr, int& result){
//    if (plr == 1){cout << "t1!\n";}
//    else cout << "t2!\n";
    // horizontal
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS-3; ++c)
            if (board[r][c]==plr && board[r][c+1]==plr && board[r][c+2]==plr && board[r][c+3]==plr){
                result = plr;return;}
    // vertical
    for (int r = 0; r < ROWS-3; ++r)
        for (int c = 0; c < COLS; ++c)
            if (board[r][c]==plr && board[r+1][c]==plr && board[r+2][c]==plr && board[r+3][c]==plr){
                result = plr;return;}
    // reverse solidus (\)
    for (int r = 0; r < ROWS-3; ++r)
        for (int c = 0; c < COLS-3; ++c)
            if (board[r][c]==plr && board[r+1][c+1]==plr && board[r+2][c+2]==plr && board[r+3][c+3]==plr){
                result = plr;return;}
    // regular solidus (/)
    for (int r = 3; r < ROWS; ++r)
        for (int c = 0; c < COLS-3; ++c)
            if (board[r][c]==plr && board[r-1][c+1]==plr && board[r-2][c+2]==plr && board[r-3][c+3]==plr){
                result = plr;return;}
    return;
}
int is_winning(Board& board) {
    int r = 0;
    thread t2(plr_win, ref(board), 2, ref(r));
    thread t1(plr_win, ref(board), 1, ref(r));
    t1.join();
    t2.join();
//    cout << r << endl;
    return r;
}
int evaluate(Board& board) {
    int r = is_winning(board);
    if (r == AI) return 50;
    if (r == PLAYER) return -50;
    return 0;
}
bool full(Board& board) {
    for (int c = 0; c < COLS; ++c)
        if (board[0][c] == 0) // top row empty = moves still possible
            return false;
    return true;
}

int minimax(Board& board, int depth, int alpha, int beta, bool maximizing_player) {
    int score = evaluate(board);
    if (depth == 0 || abs(score) == 1000 || full(board)) {
        if (full(board) && score == 0)
            return 0;
        if (score > 0)
            return score + depth;
        else if (score < 0)
            return score - depth;
        return score;
    }

    
    if (maximizing_player) {
        int max_eval = numeric_limits<int>::min();
        for (int col : moves) {
            if (is_valid(board, col)) {
                move(board, col, AI);
                int eval = minimax(board, depth-1, alpha, beta, false);    // full window search
                undo(board, col);
                max_eval = max(max_eval, eval);
                alpha = max(alpha, eval);
                if (beta <= alpha)
                    break; // prune
            }
        }
        return max_eval;
    } else {
        int min_eval = numeric_limits<int>::max();
        for (int col : moves) {
            if (is_valid(board, col)) {
                move(board, col, PLAYER);
                int eval = minimax(board, depth-1, alpha, beta, true); // flip beta/alpha signs
                undo(board, col);
                min_eval = min(min_eval, eval);
                beta = min(beta, eval);
                if (beta <= alpha)
                    break; // prune
            }
        }
        return min_eval;
    }
}
int best(Board& board) {
    int best_col = 3;
    int best_score = -100111;
    if (find_book(movestring) != -1) return find_book(movestring);
    
    for (int i = 0; i < 7; i++) {
        int col = moves[i];
            if (is_valid(board, col)) {
                move(board, col, AI);
                int eval = -minimax(board, depth-1, numeric_limits<int>::min(), numeric_limits<int>::max(), false);
                undo(board, col);
                best_score = max(best_score, eval);
            }
        }

    return best_col;
}

int main() {
    Board board(ROWS, vector<int>(COLS, 0));
    int player;
    cout << "CPU plays first, You play second.\n";
    bool skip = false;
    
    while (true) {
        
        if (not skip) {
            int ai2 = best(board);
            move(board, ai2, AI);
            cout << "CPU plays column: " << ai2 << endl;
            display(board);
            if (is_winning(board)==AI) {
                cout << "CPU wins!\n";
                break;
            }
            if (is_winning(board)==PLAYER) {
                cout << "You win!\n";
                break;
            }
            movestring += to_string(ai2);
        }
        cout << "Your move. (0-6): ";
        cin >> player;
        if (not skip) num_round++;
        if (is_valid(board, player) or player > 6 or player < 0){
            move(board, player, PLAYER);
            movestring += to_string(player);
            cout << "moves: " << movestring << endl;
        }
        else {
            skip = true;
            cout << "Invalid move!\n";
            continue;
        }
    }

    return 0;
}

