#ifndef COMMANDE_H
#define COMMANDE_H

#include <string>
#include <boost/asio.hpp>
#include <mutex>

#define gainK 0.1

struct Position {
    float x;
    float y;
};
typedef struct Position Position;

extern Position consigne; // Déclaration uniquement
extern std::mutex consigne_mutex; // Déclaration uniquement

// Fonctions de contrôle
void calculerCommande(Position* mesure, Position* consigne, Position* commande);
std::string formaterCommande(Position* commande);
void envoyerCommande(Position* commande, boost::asio::serial_port& serial);
void asservirServo(Position* mesure);

#endif // COMMANDE_H