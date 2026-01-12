#ifndef MAZE_H_INCLUDED
#define MAZE_H_INCLUDED
#include <list>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <set>
#include <iostream>
#include "GraphicAllegro5.h"

#define MAX_W 100
#define MAX_H 100

// Types de cases
enum class SpriteType : unsigned char {
    GROUND = ' ', OUTSIDE = 'X', WALL = '#',
    PLAYER = '@', PLAYER_ON_GOAL = '+',
    BOX = '$', BOX_PLACED = '*', GOAL = '.'
};

// Directions
enum Direction { NONE = -1, TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3 };

struct Square {
    std::pair<int, int> pos;
    SpriteType type;
};

class Maze
{
private:
    std::vector<std::vector<Square>> grid;
    std::vector<std::vector<bool>> deadMap; // true si deadlock

    std::pair<int, int> playerPos;
    int h = 0, w = 0;
    char playerDir = RIGHT;
    bool bas =false;
    bool fin = false;
    bool bloque = false;
    std::list<std::pair<int,int>> boxes;
    std::list<std::pair<int,int>> goals;
    void computeDeadlocks(); // Algo detection des deadlocks

public:
    Maze(const std::string& path);
    Maze(const Maze& other) = default;
    ~Maze() = default;

    // Core
    bool update(char dir);
    bool moveBox(const std::pair<int, int>& pos, char dir);
    bool isWon() const;

    // Helpers
    bool isWall(int y, int x) const;
    bool isBox(int y, int x) const;
    bool isGoal(int y, int x) const;
    bool isDead(int y, int x) const;

    // Getters
    int getHeight() const { return h; }
    int getWidth() const { return w; }
    const std::pair<int, int>& getPlayerPos() const { return playerPos; }

    // Render & IA
    void draw(GraphicAllegro5& g) const;
    void showSolution(GraphicAllegro5& g, const std::vector<char>& path);

    std::string hash() const; // Pour le visited set
    std::vector<char> solveBFS();
    std::vector<char> solveDFS();
    void solve();
    void solveNaif();
};

#endif


