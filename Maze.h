#ifndef MAZE_H_INCLUDED
#define MAZE_H_INCLUDED

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <list>       // [FIX 1] Added missing include
#include <iostream>
#include "GraphicAllegro5.h"

// Max size for the field
#define NB_MAX_WIDTH     100
#define NB_MAX_HEIGHT    100

enum class SpriteType : unsigned char
{
    GROUND = ' ', OUTSIDE = 'X',
    WALL = '#', PLAYER = '@', PLAYER_ON_GOAL = '+',
    BOX = '$', BOX_PLACED = '*', GOAL = '.'
};

// Directions for neighbors
const std::vector<std::pair<int,int>> neighbours = {
    {-1, 0}, // N (top)
    {1, 0}, // S (bottom)
    {0, -1}, // O (left)
    {0, 1}, // E (right)
};

class GraphicAllegro5;

enum Direction
{
    NONE = -1,
    TOP = 0, BOTTOM = 1, LEFT = 2, RIGHT = 3,
    TOP_LEFT = 4, TOP_RIGHT = 5, BOTTOM_LEFT = 6, BOTTOM_RIGHT = 7,
    DIRECTION_MAX = 4, DIRECTION_DIAGO_MAX = 8,
};

struct Square
{
    std::pair<int, int> position;
    SpriteType sprite;
};

class Maze
{
    private:
        std::vector<std::vector<Square>> m_field;
        std::pair<int, int> m_playerPosition;
        unsigned int m_lig = 0, m_col = 0; // size of field
        char m_playerDirection = RIGHT;
        
        bool fin = false;
        bool bas = false; // [FIX 2] Added missing member variable 'bas'
        
        std::list<std::pair<int,int>> boxes;
        std::list<std::pair<int,int>> goals;

    public:

        Maze(const std::string& levelPath);
        Maze(const Maze& other) = default;
        ~Maze() = default;

        bool updatePlayer(char dir);

        bool isWall(const std::pair<int, int>& position) const;
        bool isBox(const std::pair<int, int>& position) const;
        bool isGoal(const std::pair<int, int>& position) const;
        bool isFree(const std::pair<int, int>& position) const;

        bool pushBox(const std::pair<int, int>& position, char dir);

        bool isSolution() const;

        void playSolution(GraphicAllegro5& g, const std::vector<char>& movesSolution);
        void draw(GraphicAllegro5& g) const;

        unsigned int getNbLines() const { return this->m_lig; }
        unsigned int getNbCols() const { return this->m_col; }

        const std::pair<int, int>& getPlayerPosition() const { return this->m_playerPosition; }
        
        void solve();
        void solvNaif(); // [FIX 3] Added missing prototype

        // --- NOUVELLES METHODES POUR L'IA ---
        std::string getSignature() const;

        // Algorithmes de résolution
        std::vector<char> solveBFS(); 
        std::vector<char> solveDFS(); 
};

#endif // MAZE_H_INCLUDED
