#include "camera.hpp"
#include "commande.hpp"

#include <thread>
#include <iostream>

// Définition des variables globales
Position consigne = {90, 90};
std::mutex consigne_mutex;
std::atomic<bool> stop_signal{false};

int main() {
    Position mesure = {320, 240};

    std::thread thread_asservissement(asservirServo, &mesure); // thread secondaire, pas d’UI

    camera(); // appel de la fonction avec UI OpenCV, dans le thread principal

    thread_asservissement.join();

    std::cout << "Fermeture propre du programme." << std::endl;
    return 0;
}