#include "commande.hpp"
#include "camera.hpp"

#include <iostream> // Pour afficher des messages dans la console
#include <thread>   // Pour std::this_thread::sleep_for
#include <math.h> 

// ----------------- Configuration ----------------- //
#define XSIGN (-1) // 1 = droite, (-1) = gauche
#define YSIGN (1) // 1 = bas, (-1) = haut

const bool isXBase = true; // true = X est la base, false = Y est la base

// ----------------- Correcteur ----------------- //
// Coefficients du correcteur PID
extern double gainK; 
extern double correctorTimeConstant;
extern double correctorTimeConstantC;
extern double correctorTimeConstantD;

// Erreurs
double previousXError = 0.0;
double previousYError = 0.0;
double currentXError = 0.0;
double currentYError = 0.0;
double dXError = 0.0;
double dYError = 0.0;
// Somme des erreurs pour l'intégrale
double summedXError = 0.0;
double summedYError = 0.0;

// Variables de temps
auto currentTime = std::chrono::high_resolution_clock::now();
auto previousTime = std::chrono::high_resolution_clock::now();
double elapsedTime = 0.0;

// Commande
double XCommand = 0.0;
double YCommand = 0.0;

// ----------------- Camera ----------------- //
const double cropWeight = 1.2*640.0/480.0; // Permet de corriger la fenetre de la camera

// ----------------- Fonctions ----------------- //
void calculerCommande(Position* mesure, Position* consigne, Position* commande) {
    // Calcul du temps écoulé
    currentTime = std::chrono::high_resolution_clock::now();
    elapsedTime = std::chrono::duration<double>(currentTime - previousTime).count();
    previousTime = currentTime;

    // Calcul de l'erreur
    currentXError = consigne->x - mesure->x;
    currentYError = consigne->y - mesure->y;

    // Accumulation des erreurs
    summedXError += currentXError * elapsedTime;
    summedYError += currentYError * elapsedTime;

    dXError = (currentXError - previousXError) / elapsedTime;
    dYError = (currentYError - previousYError) / elapsedTime;


    XCommand = XSIGN * gainK * (correctorTimeConstant * currentXError + summedXError + correctorTimeConstantD * dXError);
    YCommand = YSIGN * gainK * (correctorTimeConstant * currentYError + summedYError + correctorTimeConstantD * dYError) * cropWeight;
    // Bornes
    if (XCommand > 180) {XCommand = 180; summedXError -= currentXError * elapsedTime;dXError=0;}
    if (YCommand > 180) {YCommand = 180; summedYError -= currentYError * elapsedTime;dYError=0;}
    if (XCommand < 0) {XCommand = 0; summedXError -= currentXError * elapsedTime;dXError=0;}
    if (YCommand < 0) {YCommand = 0; summedYError -= currentYError * elapsedTime;dYError=0;}

    // Mise à jour de la commande
    previousXError = currentXError;
    previousYError = currentYError;
    commande->x = XCommand;
    commande->y = YCommand;
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
    
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    serial.close();
}

