#include "Maze.h"
#include "GraphicAllegro5.h"
#include <fstream>
#include <exception>
#include <sstream>

// Structure interne pour stocker un état dans la File (BFS) ou Pile (DFS)
struct Node {
    Maze state;              // L'état complet du jeu à cet instant
    std::vector<char> path;  // La liste des mouvements pour arriver ici
};

Maze::Maze(const std::string& levelPath)
{
    std::vector<std::string> lines;
    std::string line;
    std::ifstream iss(levelPath);

    if (!iss.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + levelPath);
    }

    while (std::getline(iss, line))
    {
        lines.push_back(line);
        this->m_lig++;
        this->m_col = (this->m_col < line.size() ? line.size() : this->m_col);
    }

    this->m_field.resize(this->m_lig, std::vector<Square>(this->m_col));

    if (this->m_col <= NB_MAX_WIDTH && this->m_lig <= NB_MAX_HEIGHT)
    {
        for (unsigned int i=0; i<lines.size(); i++)
        {
            for (unsigned int j=0; j<this->m_col; j++)
            {
                if (j < lines[i].size())
                {
                    Square s;
                    s.position = std::make_pair(i, j);
                    s.sprite = (SpriteType)lines[i][j];
                    this->m_field[i][j] = s;

                    if (s.sprite == SpriteType::PLAYER || s.sprite == SpriteType::PLAYER_ON_GOAL)
                    {
                        this->m_playerPosition = std::make_pair(i, j);
                        if (s.sprite == SpriteType::PLAYER_ON_GOAL)
                        {
                            this->m_field[i][j].sprite = SpriteType::GOAL;
                        }
                    }
                }
                else
                {
                    Square s;
                    s.position = std::make_pair(i, j);
                    s.sprite = SpriteType::GROUND;
                    this->m_field[i][j] = s;
                }
            }
        }
    }
    else
    {
        throw std::out_of_range("Width or height too large !");
    }
}

void Maze::draw(GraphicAllegro5& g) const
{
    for (unsigned int i=0; i<this->m_field.size(); ++i)
    {
        for (unsigned int j=0; j<this->m_field[i].size(); ++j)
        {
            const auto s = this->m_field[i][j];
            if (s.sprite == SpriteType::WALL) g.drawT(g.getSprite(BITMAP_WALL), j, i);
            else if (s.sprite == SpriteType::BOX_PLACED) g.drawT(g.getSprite(BITMAP_BOX_PLACED), j, i);
            else if (s.sprite == SpriteType::BOX) g.drawT(g.getSprite(BITMAP_BOX), j, i);
            else if (s.sprite == SpriteType::GOAL) g.drawT(g.getSprite(BITMAP_GOAL), j, i);
        }
    }
    g.drawT(g.getSpritePlayer(this->m_playerDirection), this->m_playerPosition.second, this->m_playerPosition.first);
}

bool Maze::isWall(const std::pair<int, int>& position) const
{
    return this->m_field[position.first][position.second].sprite == SpriteType::WALL;
}

bool Maze::isBox(const std::pair<int, int>& position) const
{
    auto s = this->m_field[position.first][position.second].sprite;
    return (s == SpriteType::BOX || s == SpriteType::BOX_PLACED);
}

bool Maze::isGoal(const std::pair<int, int>& position) const
{
    auto s = this->m_field[position.first][position.second].sprite;
    return (s == SpriteType::GOAL || s == SpriteType::BOX_PLACED);
}

bool Maze::isFree(const std::pair<int, int>& position) const
{
    auto s = this->m_field[position.first][position.second].sprite;
    return (s == SpriteType::GROUND || s == SpriteType::GOAL);
}

bool Maze::isSolution() const
{
    for (unsigned int i=0; i<this->m_field.size(); ++i)
    {
        for (unsigned int j=0; j<this->m_field[i].size(); ++j)
        {
            if (this->m_field[i][j].sprite == SpriteType::BOX) return false;
        }
    }
    return true;
}

bool Maze::pushBox(const std::pair<int, int>& position, char dir)
{
    auto newPosition = position;
    newPosition.first += neighbours[dir].first;
    newPosition.second += neighbours[dir].second;

    if (this->isWall(newPosition) || !this->isFree(newPosition))
        return false;

    // Mise à jour case actuelle (départ caisse)
    if (this->m_field[position.first][position.second].sprite == SpriteType::BOX_PLACED)
        this->m_field[position.first][position.second].sprite = SpriteType::GOAL;
    else
        this->m_field[position.first][position.second].sprite = SpriteType::GROUND;

    // Mise à jour nouvelle case (arrivée caisse)
    if (this->isGoal(newPosition))
        this->m_field[newPosition.first][newPosition.second].sprite = SpriteType::BOX_PLACED;
    else
        this->m_field[newPosition.first][newPosition.second].sprite = SpriteType::BOX;

    this->m_playerDirection = dir;
    return true;
}

bool Maze::updatePlayer(char dir)
{
    if (dir < 0 || dir > DIRECTION_MAX) return false;

    bool moved = false;
    auto newPositionPlayer = this->getPlayerPosition();
    newPositionPlayer.first += neighbours[dir].first;
    newPositionPlayer.second += neighbours[dir].second;

    if (this->isWall(newPositionPlayer)) return false;

    if (!this->isBox(newPositionPlayer))
    {
        moved = true;
    }
    else if (this->pushBox(newPositionPlayer, dir))
    {
        moved = true;
    }

    if (moved)
    {
        this->m_playerPosition = newPositionPlayer;
        this->m_playerDirection = dir;
        return (this->isSolution());
    }

    return false;
}

void Maze::playSolution(GraphicAllegro5& g, const std::vector<char>& movesSolution)
{
    std::cout << "Replaying solution... (" << movesSolution.size() << " steps)" << std::endl;
    for (const auto& move : movesSolution)
    {
        g.clear();
        this->updatePlayer(move);
        this->draw(g);
        g.display();
        al_rest(0.1); // Petite pause pour voir l'animation
    }
}

void Maze::solve(){
    auto pos = getPlayerPosition();
    std::pair<int,int> nextPos = pos;
    std::pair<int,int> testPos = nextPos;
    for(int i =0; i<DIRECTION_MAX;i++){
        std::pair<int,int> temp = nextPos;
            if(i==0){
                temp.first -=1;
                if(isBox(temp)){
                     boxes.push_back(temp);
                }
                if(isGoal(temp)){
                    goals.push_back(temp);
                }
            }
            else if(i==1){
                temp.first +=1;
                if(isBox(temp)){
                    boxes.push_back(temp);
                }
                if(isGoal(temp)){
                    goals.push_back(temp);
                }
            }
            else if(i==2){
                temp.second -=1;
                if(isBox(temp)){
                    boxes.push_back(temp);
                }
                if(isGoal(temp)){
                    goals.push_back(temp);
                }
            }
            else if(i==3){
                temp.second +=1;
                if(isBox(temp)){
                    boxes.push_back(temp);
                }
                if(isGoal(temp)){
                    goals.push_back(temp);
                }
            }
    }
    //si colonne pair et possible de déscendre alors déscendre si pair et peut pas déscendre alors monter sinon aller à droite
    if(!fin){
    testPos.first+=1;
    if(!isWall(testPos)&& !bas && !isBox(testPos)){
        updatePlayer(BOTTOM);
        nextPos.first+=1;
    }

    else if(bas){
        testPos.first-=2;
        if(!isWall(testPos) && !isBox(testPos)){
          updatePlayer(TOP);
          nextPos.first-=1;
        }
        else{
            bas = false;
        }

    }
    else{
        testPos.first-=1;
        testPos.second+=1;
        if(!isWall(testPos)&& !isBox(testPos)){
            updatePlayer(RIGHT);
            nextPos.second+=1;
        }
        else if(isWall(testPos)){
            fin =true;
        }
        bas = true;
    }
}
if(fin){
    boxes.sort();
    goals.sort();
    boxes.unique();
    goals.unique();
    std::pair<int,int> targetBox = boxes.front();
    std::pair<int,int> targetGoal = goals.front();
    int y =nextPos.first - targetBox.first;
    int x = nextPos.second - targetBox.second;
    if(x<0){
        for(int i =0; i<(x*-1);i++){
            updatePlayer(RIGHT);
        }
    }
    if(x>0){
        for(int i =0; i<(x);i++){
            updatePlayer(LEFT);
        }
    }
    if(y<0){
        for(int i =0; i<(y*-1);i++){
            updatePlayer(BOTTOM);
        }
    }
    if(y>0){
        for(int i =0;i<y;i++){
        updatePlayer(TOP);
        }
    }
}
}


// ---------------- ALGORITHMES IA ----------------

std::string Maze::getSignature() const {
    // Création d'une clé unique pour l'état actuel
    std::string sig = "";
    // Position du joueur
    sig += std::to_string(m_playerPosition.first) + "," + std::to_string(m_playerPosition.second) + ";";
    // Position des caisses (on parcourt toute la grille)
    for(const auto& row : m_field) {
        for(const auto& square : row) {
            // Optimisation : seuls les caisses et murs importent
            sig += (char)square.sprite;
        }
    }
    return sig;
}

// --- BFS : Breadth-First Search (Parcours en Largeur) ---
// Utilise une FILE (Queue). Trouve le chemin le plus court.
std::vector<char> Maze::solveBFS()
{
    std::queue<Node> q;
    std::set<std::string> visited;

    // État initial
    Node startNode = { *this, {} };
    q.push(startNode);
    visited.insert(this->getSignature());

    std::cout << "Starting BFS..." << std::endl;

    while (!q.empty())
    {
        Node current = q.front();
        q.pop();

        if (current.state.isSolution()) {
            std::cout << "BFS Found solution! Length: " << current.path.size() << std::endl;
            return current.path;
        }

        // Tester les 4 directions : TOP(0), BOTTOM(1), LEFT(2), RIGHT(3)
        for (char dir = 0; dir < 4; ++dir)
        {
            Node nextNode = current; // Copie de l'état (nécessaire pour simuler)

            // On sauvegarde la pos avant mouvement pour vérifier si on a bougé
            auto oldPos = nextNode.state.getPlayerPosition();
            nextNode.state.updatePlayer(dir);
            auto newPos = nextNode.state.getPlayerPosition();

            // Si le joueur a effectivement bougé (pas de mur)
            if (oldPos != newPos)
            {
                std::string sig = nextNode.state.getSignature();
                // Si état jamais visité
                if (visited.find(sig) == visited.end()) {
                    visited.insert(sig);
                    nextNode.path.push_back(dir);
                    q.push(nextNode);
                }
            }
        }
    }

    std::cout << "BFS: No solution found." << std::endl;
    return {};
}

// --- DFS : Depth-First Search (Parcours en Profondeur) ---
// Utilise une PILE (Stack). Ne garantit PAS le chemin le plus court.
std::vector<char> Maze::solveDFS()
{
    std::stack<Node> s;
    std::set<std::string> visited;

    Node startNode = { *this, {} };
    s.push(startNode);
    visited.insert(this->getSignature());

    std::cout << "Starting DFS..." << std::endl;

    while (!s.empty())
    {
        Node current = s.top();
        s.pop();

        if (current.state.isSolution()) {
            std::cout << "DFS Found solution! Length: " << current.path.size() << std::endl;
            return current.path;
        }

        for (char dir = 0; dir < 4; ++dir)
        {
            Node nextNode = current;

            auto oldPos = nextNode.state.getPlayerPosition();
            nextNode.state.updatePlayer(dir);
            auto newPos = nextNode.state.getPlayerPosition();

            if (oldPos != newPos)
            {
                std::string sig = nextNode.state.getSignature();
                if (visited.find(sig) == visited.end()) {
                    visited.insert(sig);
                    nextNode.path.push_back(dir);
                    s.push(nextNode);
                }
            }
        }
    }

    std::cout << "DFS: No solution found." << std::endl;
    return {};
}

void Maze::solvNaif(){
    for(int i = 0; i < DIRECTION_MAX; i++){
        if(i == 0){
                updatePlayer(TOP);
            for(int j =0; j < DIRECTION_MAX; j++){
                if(j==0){
                    updatePlayer(TOP);
                }
                else if(j==1){
                    updatePlayer(BOTTOM);
                }
                else if(j==2){
                    updatePlayer(LEFT);
                }
                else if(j==3){
                    updatePlayer(RIGHT);
                }
            }
        }
        else if(i == 1){
                updatePlayer(BOTTOM);
            for(int j =0; j <DIRECTION_MAX; j++){
                if(j==0){
                    updatePlayer(TOP);
                }
                else if(j==1){
                    updatePlayer(BOTTOM);
                }
                else if(j==2){
                    updatePlayer(LEFT);
                }
                else if(j==3){
                    updatePlayer(RIGHT);
                }
            }
        }
        else if(i == 2){
                updatePlayer(BOTTOM);
            for(int j =0; j <DIRECTION_MAX; j++){
                if(j==0){
                    updatePlayer(TOP);
                }
                else if(j==1){
                    updatePlayer(BOTTOM);
                }
                else if(j==2){
                    updatePlayer(LEFT);
                }
                else if(j==3){
                    updatePlayer(RIGHT);
                }
            }
        }
        else if(i == 3){
                updatePlayer(BOTTOM);
            for(int j =0; j <DIRECTION_MAX; j++){
                if(j==0){
                    updatePlayer(TOP);
                }
                else if(j==1){
                    updatePlayer(BOTTOM);
                }
                else if(j==2){
                    updatePlayer(LEFT);
                }
                else if(j==3){
                    updatePlayer(RIGHT);
                }
            }
        }
    }
}


