#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
const int MAX = 7;
const int MAX_STEP = 50;

class point {
public: int x; int y; void print() { cout << x << ", " << y << endl; }
};

struct Stack {
public:
    int x[MAX_STEP];
    int y[MAX_STEP];
    int size = 0;

    void push(point a) {
        x[size] = a.x;
        y[size] = a.y;
        size = size + 1;
    }

    void filtr() {
        for (int i = 0; i < size; i++) {
            for (int j = i + 1; j < size; j++) {
                if (x[i] == x[j] && y[i] == y[j]) {
                    for (int k = i + 1; k <= j; k++) {
                        x[k] = -1;
                    }
                    break;}}}

        int nsize = 0;
        for (int i = 0; i < size; i++) {
            if (x[i] != -1) {
                x[nsize] = x[i];
                y[nsize] = y[i];
                nsize++;
            }
        }
        size = nsize;
    }
};

int potential(int lab[MAX][MAX], int rows, int cols, int i, int j, int xEx, int yEx) {

    int rowOffset[] = { 1,0,0,-1 };
    int colOffset[] = { 0,1,-1,0 };
    int s = 0;

    for (int k = 0; k < 4; k++) {
        int nR = i + rowOffset[k];
        int nC = j + colOffset[k];

        if (nR > 0 && nR < rows - 1 && nC >0 && nC < cols - 1 && (lab[nR][nC] == 1 || lab[nR][nC] == -1)) {
            s++;
        }
    }

    return s + abs(xEx - i) + abs(yEx - j);
}

point find(int lab[MAX][MAX], int rows, int cols, point r) {
    int rowOffset[] = { 1,0,0,-1 };
    int colOffset[] = { 0,1,-1,0 };

    bool f = false;
    point res;
    res.x = -1;
    res.y = -1;

    for (int k = 0; k < 4; k++) {
        int nR = r.x + rowOffset[k];
        int nC = r.y + colOffset[k];

        if (nR > 0 && nR < rows - 1 && nC >0 && nC < cols - 1 && lab[nR][nC] != -1 && (lab[nR][nC] <= lab[res.x][res.y] || !f)) {
            res.x = nR;
            res.y = nC;
            f = true;
        }
    }
    return res;
}

int main() {
    ifstream file("C:\\Users\\User\\Desktop\\lab1.txt");

    int lab[MAX][MAX];
    int rows = 0; //строки
    int cols = 0; //столбцы

    while (rows < MAX && file >> lab[rows][0]) {
        cols = 0;
        while (file.peek() != '\n' && cols < MAX) {
            file >> lab[rows][++cols];
        }
        rows++;
    }
    file.close();

    Stack run;

    point enter;
    point exit;
    point r;

    for (int i = 0; i < MAX; i++) {
        if (lab[0][i] == 2) { enter.x = 0; enter.y = i; r.x = 1; r.y = i; }
        else if (lab[rows - 1][i] == 2) { enter.x = rows-1; enter.y = i; r.x = rows-2; r.y = i; }
        else if (lab[i][0] == 2) { enter.x = i; enter.y = 0; r.x = i; r.y = 1; }
        else if (lab[i][cols - 1] == 2) { enter.x = i; enter.y = cols-1; r.x = i; r.y = cols-2; }

        else if (lab[0][i] == 3) { exit.x = 0; exit.y = i; }
        else if (lab[rows - 1][i] == 3) { exit.x = rows - 1; exit.y = i; }
        else if (lab[i][0] == 3) { exit.x = i; exit.y = 0; }
        else if (lab[i][cols - 1] == 3) { exit.x = i; exit.y = cols - 1; }}

    run.push(enter);
    run.push(r);

    int rowOffset[] = { 1,0,0,-1 };
    int colOffset[] = { 0,1,-1,0 };

    while (true) {
        bool foundExit = false;

        for (int k = 0; k < 4; k++) {
            int nR = r.x + rowOffset[k];
            int nC = r.y + colOffset[k];

            if (nR == exit.x && nC == exit.y) {
                run.push(exit);
                foundExit = true;
                break;
            }
            else if (nR >= 0 && nR < rows && nC >= 0 && nC < cols) {
                if (lab[nR][nC] == 0) {
                    lab[nR][nC] = potential(lab, rows, cols, nR, nC, exit.x, exit.y);
                }
                else if (lab[nR][nC] == 1) {
                    lab[nR][nC] = -1;
                }
            }
        }
        if (foundExit) break;

        lab[r.x][r.y] = 99;
        r = find(lab, rows, cols, r);
        run.push(r);
        if (run.size == MAX_STEP - 1) { cout << "NON-EXITABLE LABIRINT"; break; }
    }

    run.filtr();
    ofstream outfile("C:\\Users\\User\\Desktop\\RUN.txt");
    for (int i = 0; i < run.size; i++) { outfile << run.x[i] << ", " << run.y[i] << endl; }
    outfile.close();
}