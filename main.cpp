#include "Maze.h"
#include "GraphicAllegro5.h"
#include <iostream>

GraphicAllegro5 graphic(1024, 768);

int main()
{
    // Assurez-vous que le fichier existe dans le dossier de l'exécutable
    const std::string level = "levels/Easy1.txt";

    try {
        Maze m(level);
        graphic.show();

        std::cout << "--- COMMANDES ---" << std::endl;
        std::cout << "Fleches : Bouger" << std::endl;
        std::cout << "R : Reset niveau" << std::endl;
        std::cout << "B : Solve BFS (Largeur - Court)" << std::endl;
        std::cout << "D : Solve DFS (Profondeur - Long)" << std::endl;
        std::cout << "-----------------" << std::endl;

        while (!graphic.keyGet(ALLEGRO_KEY_ESCAPE) && !graphic.keyGet(ALLEGRO_KEY_Q))
        {
            // Contrôles manuels
            m.solve();
            if (graphic.keyGet(ALLEGRO_KEY_UP)) m.updatePlayer(TOP);
            if (graphic.keyGet(ALLEGRO_KEY_DOWN)) m.updatePlayer(BOTTOM);
            if (graphic.keyGet(ALLEGRO_KEY_LEFT)) m.updatePlayer(LEFT);
            if (graphic.keyGet(ALLEGRO_KEY_RIGHT)) m.updatePlayer(RIGHT);

            // Touche 'C' : Vérification manuelle
            if (graphic.keyGet(ALLEGRO_KEY_C))
            {
                if (m.isSolution()) { std::cout << "Level solved !" << std::endl; }
                else { std::cout << "Level not solved !" << std::endl; }
            }

            // Touche 'B' : Résolution BFS
            if (graphic.keyGet(ALLEGRO_KEY_B))
            {
                // On recharge le niveau pour partir d'un état propre
                m = Maze(level);
                std::vector<char> solution = m.solveBFS();
                if (!solution.empty()) {
                    m.playSolution(graphic, solution);
                }
            }

            // Touche 'D' : Résolution DFS
            if (graphic.keyGet(ALLEGRO_KEY_D))
            {
                m = Maze(level);
                std::vector<char> solution = m.solveDFS();
                if (!solution.empty()) {
                    m.playSolution(graphic, solution);
                }
            }

            // Touche 'R' : Reset
            if (graphic.keyGet(ALLEGRO_KEY_R))
            {
                m = Maze(level);
            }

            // Affichage
            graphic.clear();
            m.draw(graphic);
            graphic.display();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

