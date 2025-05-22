#include "camera.hpp"
#include "commande.hpp"

#include <thread>
#include <iostream>

#define LISTEN 0 // 1 = écoute du port série, 0 = pas d'écoute

// Définition des variables globales
Position consigne = {90, 90};
std::mutex consigne_mutex;
std::atomic<bool> stop_signal{false};

void attendreReady(boost::asio::serial_port& serial) {
    std::string ligne;
    char c;

    std::cout << "En attente de READY de l’Arduino..." << std::endl;

    while (true) {
        boost::asio::read(serial, boost::asio::buffer(&c, 1));
        if (c == '\n') {
            ligne.erase(std::remove_if(ligne.begin(), ligne.end(),
                                       [](char ch) { return !isprint(ch); }),
                        ligne.end()); // Supprime les caractères non imprimables

            if (ligne == "READY") {
                std::cout << "Arduino prêt. Début de l’asservissement." << std::endl;
                break;
            } else {
                std::cerr << "Message ignoré : " << ligne << std::endl;
                ligne.clear(); // Réinitialiser pour la prochaine ligne
            }
        } else {
            ligne += c;
        }
    }
}



int main() {
    // ----------------------------------- PORT SERIE ----------------------------------- //
    // Initialiser le port série
    boost::asio::io_context io;
    boost::asio::serial_port serial(io);

    // Ouvrir le port série
    serial.open("/dev/tty.usbmodem144301"); // Remplacer le chemin par celui du port série approprié (ls /dev/tty* pour trouver le bon port) TESTER CU
        if (!serial.is_open()) {
            std::cerr << "Erreur : le port série n'a pas pu être ouvert :" << std::endl;
            return 1;
        } else {
            std::cout << "Port série ouvert avec succès." << std::endl;
        }
    
    
    // Configurer le port série
    try {
        serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
        serial.set_option(boost::asio::serial_port_base::character_size(8));
        serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    } catch (std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    if (LISTEN) {
        char c; // Buffer pour stocker les caractères reçus
        std::string ligne; // Chaîne pour stocker la ligne complète

        while (!stop_signal) {
            boost::asio::read(serial, boost::asio::buffer(&c, 1));
            if (c == '\n') {
                std::cout << "Reçu : " << ligne << std::endl;
                ligne.clear();
            } else {
                ligne += c;
            }
        }
    }

    attendreReady(serial); // Attendre le message "READY" de l'Arduino

    // ----------------------------------- PROGRAMME ----------------------------------- //

    Position mesure = {320, 240};

    std::thread thread_asservissement(asservirServo, &mesure, std::ref(serial)); // thread secondaire, pas d’UI

    camera(); // appel de la fonction avec UI OpenCV, dans le thread principal

    thread_asservissement.join();

    std::cout << "Fermeture propre du programme." << std::endl;
    return 0;
}