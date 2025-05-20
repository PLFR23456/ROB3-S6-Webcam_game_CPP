#include "camera.hpp"
#include "commande.hpp"

#include <thread>
#include <iostream>

// Définition des variables globales
Position consigne = {0, 0};
std::mutex consigne_mutex;
std::atomic<bool> stop_signal{false};

int main() {
    Position mesure = {0, 0};

    std::thread thread_consigne(camera);

    asservirServo(&mesure); // Se termine quand stop_signal == true

    thread_consigne.join(); // Attend que la caméra se termine aussi

    std::cout << "Fermeture propre du programme." << std::endl;
    return 0;
}