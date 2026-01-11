#include "Maze.h"
#include "GraphicAllegro5.h"
#include <iostream>

GraphicAllegro5 g(1024, 768);

int main() {
    std::string mapFile = "levels/Easy3.txt";

    try {
        Maze game(mapFile);
        g.show();

        std::cout << "=== SOKOBAN ===" << std::endl;
        std::cout << "[Arrows] Move" << std::endl;
        std::cout << "[B] BFS Solver" << std::endl;
        std::cout << "[D] DFS Solver" << std::endl;
        std::cout << "[R] Reset" << std::endl;

        bool running = true;
        while(running) {
            // Quit
            if (g.keyGet(ALLEGRO_KEY_ESCAPE)) running = false;

            // Manual moves
            if (g.keyGet(ALLEGRO_KEY_UP)) game.update(TOP);
            if (g.keyGet(ALLEGRO_KEY_DOWN)) game.update(BOTTOM);
            if (g.keyGet(ALLEGRO_KEY_LEFT)) game.update(LEFT);
            if (g.keyGet(ALLEGRO_KEY_RIGHT)) game.update(RIGHT);

            // Solvers
            if (g.keyGet(ALLEGRO_KEY_B)) {
                game = Maze(mapFile); // clean start
                auto path = game.solveBFS();
                if (!path.empty()) game.showSolution(g, path);
                game = Maze(mapFile);
            }

            if (g.keyGet(ALLEGRO_KEY_D)) {
                game = Maze(mapFile);
                auto path = game.solveDFS();
                if (!path.empty()) game.showSolution(g, path);
                game = Maze(mapFile);
            }

            if (g.keyGet(ALLEGRO_KEY_R)) {
                game = Maze(mapFile);
            }

            // Draw
            g.clear();
            game.draw(g);
            g.display();
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
