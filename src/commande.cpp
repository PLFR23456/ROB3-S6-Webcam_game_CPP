#include "commande.hpp"
#include "camera.hpp"

#include <iostream> // Pour afficher des messages dans la console
#include <thread>   // Pour std::this_thread::sleep_for

int compteurAffichage = 0;
#define PERIODE_AFFICHAGE 10 // Afficher toutes les 10 itérations

void calculerCommande(Position* mesure, Position* consigne, Position* commande) {
    // Calcul de l'erreur
    float erreur_x = consigne->x - mesure->x;
    float erreur_y = consigne->y - mesure->y;

    // Calcul de la commande
    commande->x = commande->x - gainK * erreur_x; // Coefficient de proportionnalité (Correcteur proptionnel)
    commande->y = commande->y + gainK * erreur_y; // Coefficient de proportionnalité (Correcteur proptionnel)
    if (commande->x > 180) commande->x = 180; // Limiter la commande à 180
    if(commande->x < 0) commande->x = 0; // Limiter la commande à 0
    if (commande->y < 0) commande->y = 0; // Limiter la commande à 0
    if (commande->y > 180) commande->y = 180; // Limiter la commande à 180
}

std::string formaterCommande(Position* commande) {
    // Envoi de la commande au port série
    std::string messageCommande = std::to_string((int)commande->x) + " " + std::to_string((int)commande->y) + "\n";
    return messageCommande;
}

void envoyerCommande(Position* commande, boost::asio::serial_port& serial) {
    boost::system::error_code ec;
    std::string messageCommande = formaterCommande(commande);
    boost::asio::write(serial, boost::asio::buffer(messageCommande), ec); 
    // Détection d'erreur
    if(ec) std::cerr << "Erreur d'envoi : " << ec.message() << "\n";
    else {
        if (compteurAffichage == PERIODE_AFFICHAGE) {
            std::cout << "Commande envoyée : " << messageCommande << "\n";
            std::cout << "Consigne : " << consigne.x << ", " << consigne.y ;
            compteurAffichage = 0; // Réinitialiser le compteur
        }
        compteurAffichage++;
    }
}



void asservirServo(Position* mesure) {
    try {
        // Initialiser le port série
        boost::asio::io_context io;
        boost::asio::serial_port serial(io);
    
        // Ouvrir le port série
        // "/dev/ttyACM0" pour le bo pierre louis
        // "/dev/cu.usbmodem143301" pour le moche estebean
        serial.open("/dev/ttyACM0"); // Remplacer le chemin par celui du port série approprié
    
        // Configurer le port série
        serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
        serial.set_option(boost::asio::serial_port_base::character_size(8));
        serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

        Position commande = consigne;
        Position consigne_locale;
        // Boucle d'asservissement
        while (!stop_signal) {
            {
                std::lock_guard<std::mutex> lock(consigne_mutex);
                consigne_locale = consigne;
            }
            calculerCommande(mesure, &consigne_locale, &commande);
            envoyerCommande(&commande, serial);
        
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        serial.close();
    } catch (std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

