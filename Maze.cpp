#include "Maze.h"
#include <fstream>
#include <sstream>

// Directions offsets: Haut, Bas, Gauche, Droite
const int DY[] = {-1, 1, 0, 0};
const int DX[] = {0, 0, -1, 1};

struct StateNode {
    Maze maze;
    std::vector<char> history;
};

Maze::Maze(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Fichier introuvable: " + path);

    std::vector<std::string> lines;
    std::string buf;
    while (std::getline(f, buf)) {
        lines.push_back(buf);
        if (buf.size() > (size_t)w) w = buf.size();
    }
    h = lines.size();

    if (w > MAX_W || h > MAX_H) throw std::length_error("Map trop grande");

    grid.resize(h, std::vector<Square>(w));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Square& s = grid[y][x];
            s.pos = {y, x};

            char c = (x < (int)lines[y].size()) ? lines[y][x] : ' ';
            s.type = (SpriteType)c;

            if (s.type == SpriteType::PLAYER || s.type == SpriteType::PLAYER_ON_GOAL) {
                playerPos = {y, x};
                // Si sur goal, on remet le goal en dessous
                if (s.type == SpriteType::PLAYER_ON_GOAL) s.type = SpriteType::GOAL;
                else s.type = SpriteType::GROUND; // Sinon sol
            }
        }
    }
    computeDeadlocks();
}

// Algorithme de detection des deadlocks
void Maze::computeDeadlocks() {
    deadMap.assign(h, std::vector<bool>(w, false));
    std::queue<std::pair<int, int>> q;

    // 1. On marque tout ce qui n'est pas mur comme "mort" par defaut
    // sauf les goals qui sont "vivants"
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            if (!isWall(y, x)) deadMap[y][x] = true;
            if (isGoal(y, x)) {
                deadMap[y][x] = false;
                q.push({y, x});
            }
        }
    }

    // 2. On remonte depuis les goals (Pull)
    while(!q.empty()) {
        auto curr = q.front(); q.pop();
        int cy = curr.first;
        int cx = curr.second;

        for(int i=0; i<4; i++) {
            // Case precedente (d'ou vient la caisse)
            int py = cy - DY[i];
            int px = cx - DX[i];

            // Case joueur (il doit etre derriere pour pousser)
            int ppy = py - DY[i];
            int ppx = px - DX[i];

            if (py < 0 || py >= h || px < 0 || px >= w) continue;
            if (ppy < 0 || ppy >= h || ppx < 0 || ppx >= w) continue;

            if (isWall(py, px) || isWall(ppy, ppx)) continue;

            // Si c'etait mort, on le ressuscite et on continue
            if (deadMap[py][px]) {
                deadMap[py][px] = false;
                q.push({py, px});
            }
        }
    }
}

bool Maze::isDead(int y, int x) const {
    if (y < 0 || y >= h || x < 0 || x >= w) return true;
    return deadMap[y][x];
}

void Maze::draw(GraphicAllegro5& g) const {
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            switch(grid[y][x].type) {
                case SpriteType::WALL: g.drawT(g.getSprite(BITMAP_WALL), x, y); break;
                case SpriteType::BOX_PLACED: g.drawT(g.getSprite(BITMAP_BOX_PLACED), x, y); break;
                case SpriteType::BOX: g.drawT(g.getSprite(BITMAP_BOX), x, y); break;
                case SpriteType::GOAL: g.drawT(g.getSprite(BITMAP_GOAL), x, y); break;
                default: break;
            }
        }
    }
    g.drawT(g.getSpritePlayer(playerDir), playerPos.second, playerPos.first);
}

// Helpers compacts
bool Maze::isWall(int y, int x) const { return grid[y][x].type == SpriteType::WALL; }
bool Maze::isBox(int y, int x) const {
    auto t = grid[y][x].type;
    return t == SpriteType::BOX || t == SpriteType::BOX_PLACED;
}
bool Maze::isGoal(int y, int x) const {
    auto t = grid[y][x].type;
    return t == SpriteType::GOAL || t == SpriteType::BOX_PLACED;
}

bool Maze::isWon() const {
    for(const auto& row : grid)
        for(const auto& s : row)
            if(s.type == SpriteType::BOX) return false;
    return true;
}

bool Maze::moveBox(const std::pair<int, int>& pos, char dir) {
    int ny = pos.first + DY[(int)dir];
    int nx = pos.second + DX[(int)dir];

    // Check collisions
    if (isWall(ny, nx)) return false;

    // Check si case occupée (par une autre caisse)
    auto t = grid[ny][nx].type;
    if (t != SpriteType::GROUND && t != SpriteType::GOAL) return false;

    // Update case départ
    if (grid[pos.first][pos.second].type == SpriteType::BOX_PLACED)
        grid[pos.first][pos.second].type = SpriteType::GOAL;
    else
        grid[pos.first][pos.second].type = SpriteType::GROUND;

    // Update case arrivée
    if (isGoal(ny, nx)) grid[ny][nx].type = SpriteType::BOX_PLACED;
    else grid[ny][nx].type = SpriteType::BOX;

    playerDir = dir;
    return true;
}

bool Maze::update(char dir) {
    if (dir < 0 || dir > 3) return false;

    int ny = playerPos.first + DY[(int)dir];
    int nx = playerPos.second + DX[(int)dir];

    if (isWall(ny, nx)) return false;

    bool moved = false;
    if (!isBox(ny, nx)) {
        moved = true;
    } else {
        if (moveBox({ny, nx}, dir)) moved = true;
    }

    if (moved) {
        playerPos = {ny, nx};
        playerDir = dir;
        return isWon();
    }
    return false;
}

void Maze::showSolution(GraphicAllegro5& g, const std::vector<char>& path) {
    std::cout << "Demo solution: " << path.size() << " steps." << std::endl;
    for(char m : path) {
        g.clear();
        update(m);
        draw(g);
        g.display();
        al_rest(0.05);
    }
}

std::string Maze::hash() const {
    std::string s = std::to_string(playerPos.first) + ":" + std::to_string(playerPos.second);
    for(const auto& v : grid) for(const auto& sq : v) s += (char)sq.type;
    return s;
}

std::vector<char> Maze::solveBFS() {
    std::queue<StateNode> q;
    std::set<std::string> visited;

    q.push({ *this, {} });
    visited.insert(hash());

    std::cout << "Start BFS..." << std::endl;

    while(!q.empty()) {
        StateNode curr = q.front(); q.pop();

        if (curr.maze.isWon()) {
            std::cout << "BFS Found! " << curr.history.size() << " moves." << std::endl;
            return curr.history;
        }

        for(char d=0; d<4; d++) {
            Maze next = curr.maze;
            auto posBefore = next.getPlayerPos();
            next.update(d);

            // Si pas bougé, inutile de traiter
            if (posBefore == next.getPlayerPos()) continue;

            // Check Deadlock
            int boxY = next.getPlayerPos().first + DY[(int)d];
            int boxX = next.getPlayerPos().second + DX[(int)d];

            // Si on a poussé une caisse SUR une case morte -> poubelle
            if (next.isBox(boxY, boxX) && isDead(boxY, boxX)) continue;

            std::string key = next.hash();
            if (visited.find(key) == visited.end()) {
                visited.insert(key);
                std::vector<char> nextPath = curr.history;
                nextPath.push_back(d);
                q.push({ next, nextPath });
            }
        }
    }
    return {};
}

std::vector<char> Maze::solveDFS() {
    std::stack<StateNode> s;
    std::set<std::string> visited;

    s.push({ *this, {} });
    visited.insert(hash());

    std::cout << "Start DFS..." << std::endl;

    while(!s.empty()) {
        StateNode curr = s.top(); s.pop();

        if (curr.maze.isWon()) {
            std::cout << "DFS Found! " << curr.history.size() << " moves." << std::endl;
            return curr.history;
        }

        for(char d=0; d<4; d++) {
            Maze next = curr.maze;
            auto p1 = next.getPlayerPos();
            next.update(d);

            if (p1 == next.getPlayerPos()) continue;

            // Deadlock check pruning
            int by = next.getPlayerPos().first + DY[(int)d];
            int bx = next.getPlayerPos().second + DX[(int)d];

            if (next.isBox(by, bx) && isDead(by, bx)) continue;

            std::string key = next.hash();
            if (visited.find(key) == visited.end()) {
                visited.insert(key);
                std::vector<char> nextPath = curr.history;
                nextPath.push_back(d);
                s.push({ next, nextPath });
            }
        }
    }
    return {};
}
