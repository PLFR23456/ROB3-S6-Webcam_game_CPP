#include "camera.hpp"
#include "display.hpp"

#include <thread>
#include <iostream>

std::atomic<bool> stop_signal{false}; // TODO : À supprimer pour cette branche

// Définition des variables globales
Position startbox = {300, 80};
Position endbox = {200, 480-80};

GameSession game(startbox, endbox);

int main() {
    // Initialisation de la session de jeu
    std::thread thread_camera(camera); // Traitement de l'image
    display(game); // Interface graphique et gestion des événements

    // On sort du jeu
    thread_camera.join();
    std::cout << "Fermeture propre du programme." << std::endl;
    return 0;
}