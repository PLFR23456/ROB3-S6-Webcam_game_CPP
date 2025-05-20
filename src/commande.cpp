#include "commande.hpp"
#include "camera.hpp"

#include <iostream> // Pour afficher des messages dans la console

void calculerCommande(Position* mesure, Position* consigne, Position* commande) {
    // Calcul de l'erreur
    float erreur_x = consigne->x - mesure->x;
    float erreur_y = consigne->y - mesure->y;

    // Calcul de la commande
    commande->x = gainK * erreur_x; // Coefficient de proportionnalité (Correcteur proptionnel)
    commande->y = gainK * erreur_y; // Coefficient de proportionnalité (Correcteur proptionnel)
}

std::string formaterCommande(Position* commande) {
    // Envoi de la commande au port série
    std::string messageCommande = std::to_string(commande->x) + " " + std::to_string(commande->y) + "\n";
    return messageCommande;
}

void envoyerCommande(Position* commande, boost::asio::serial_port& serial) {
    boost::system::error_code ec;
    boost::asio::write(serial, boost::asio::buffer(formaterCommande(commande)), ec); 
    // Détection d'erreur
    if(ec) std::cerr << "Erreur d'envoi : " << ec.message() << "\n";
    else std::cout << "Commande envoyée !";
}



void asservirServo(Position* mesure) {
    try {
        // Initialiser le port série
        boost::asio::io_context io;
        boost::asio::serial_port serial(io);
    
        // Ouvrir le port série
        serial.open("/dev/cu.usbmodem143301"); // Remplacer le chemin par celui du port série approprié
    
        // Configurer le port série
        serial.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serial.set_option(boost::asio::serial_port_base::character_size(8));
        serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

        Position commande;
        Position consigne_locale;
        // Boucle d'asservissement
        while (!stop_signal) {
            {
                std::lock_guard<std::mutex> lock(consigne_mutex);
                consigne_locale = consigne;
            }
            calculerCommande(mesure, &consigne_locale, &commande);
            envoyerCommande(&commande, serial);
        
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        serial.close();
    } catch (std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
}

