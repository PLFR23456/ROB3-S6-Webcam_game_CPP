#include "commande.hpp"
#include "camera.hpp"

#include <iostream> // Pour afficher des messages dans la console
#include <thread>   // Pour std::this_thread::sleep_for
#include <math.h>    // Pour std::pow

void calculerCommande(Position* mesure, Position* consigne, Position* commande) {
    // Calcul de l'erreur
    float erreur_x = consigne->x - mesure->x;
    float erreur_y = consigne->y - mesure->y;

    // Calcul de la commande
    commande->x = commande->x + XSIGN * pixelToAngle * gainK * erreur_x; // Coefficient de proportionnalité (Correcteur proportionnel)
    commande->y = commande->y + YSIGN * pixelToAngle * gainK * erreur_y; // Coefficient de proportionnalité (Correcteur proportionnel)

    // Bornes
    if (commande->x > 180) commande->x = 180; // Limiter la commande à 180
    if (commande->y > 180) commande->y = 180; // Limiter la commande à 180
    if (commande->x < 0) commande->x = 0; // Limiter la commande à 0
    if (commande->y < 0) commande->y = 0; // Limiter la commande à 0
}

std::string formaterCommande(Position* commande) {
    int baseCommand = (isXBase) ? (int)commande->x : (int)commande->y;
    int armCommand = (isXBase) ? (int)commande->y : (int)commande->x;

    // Envoi de la commande au port série
    std::string messageCommande = std::to_string(baseCommand) + " " + std::to_string(armCommand) + "\n";
    return messageCommande;
}

void envoyerCommande(Position* commande, boost::asio::serial_port& serial) {
    boost::system::error_code ec;
    std::string messageCommande = formaterCommande(commande);

    {
        std::lock_guard<std::mutex> lock(consigne_mutex);
        std::cout << "CMD envoyée : \t" << messageCommande;
    }

    boost::asio::write(serial, boost::asio::buffer(messageCommande), ec); 

    if(ec) {
        std::cerr << "Erreur d'envoi : " << ec.message() << "\n";
        return;
    }

    // Lecture de la réponse de l'Arduino
    char c;
    std::string ligne;
    while (true) {
        boost::asio::read(serial, boost::asio::buffer(&c, 1));
        if (c == '\n') break;
        ligne += c;
    }

    // Analyse de la réponse (parser)
    if (ligne.rfind("ACK:", 0) == 0) {
        int val1, val2;
        std::istringstream iss(ligne.substr(4));
        if (iss >> val1 >> val2) {
            std::cout << "ACK reçu : \t" << val1 << " " << val2 << std::endl;
        } else {
            std::cerr << "Format ACK invalide : " << ligne << std::endl;
        }
    } else if (ligne.rfind("ERR:", 0) == 0) {
        std::cerr << "Erreur Arduino : " << ligne.substr(5) << std::endl;
    } else {
        std::cerr << "Réponse inconnue : " << ligne << std::endl;
    }
}


void asservirServo(Position* mesure, boost::asio::serial_port& serial) {
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
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    serial.close();
}

