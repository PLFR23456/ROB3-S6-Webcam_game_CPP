#ifndef COMMANDE_H
#define COMMANDE_H

#include <string>
#include <boost/asio.hpp>
#include <mutex>

#define XSIGN (-1) // 1 = droite, (-1) = gauche
#define YSIGN 1 // 1 = bas, (-1) = haut

const bool isXBase = true; // true = X est la base, false = Y est la base
const double gainK = 0.005; // Coefficient de proportionnalité
const double pixelToAngle = 640.0/480.0; // Coefficient de proportionnalité

// changer ordrexy si la caméra bouge à droite à la place de remonter
// changer signex si la camera va a gauche au lieu de droite
// changer signey si la camera remonte au lieu de descendre


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
void asservirServo(Position* mesure, boost::asio::serial_port& serial);

#endif // COMMANDE_H