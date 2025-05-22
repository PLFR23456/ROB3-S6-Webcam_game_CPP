#ifndef COMMANDE_H
#define COMMANDE_H

#include <string>
#include <boost/asio.hpp>
#include <mutex>

#define signex 1 // 1 = positif, -1 = négatif
#define signey 0 // 1 = positif, -1 = négatif
#define ordrexy 1 // 0 = x;y, 1 = y;x 

// changer ordrexy si la caméra bouge à droite à la place de remonter
// changer signex si la camera va a gauche au lieu de droite
// changer signey si la camera remonte au lieu de descendre


#define gainK 0.005
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