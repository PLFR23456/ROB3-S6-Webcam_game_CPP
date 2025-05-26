#ifndef COMMANDE_H
#define COMMANDE_H

#include <string>
#include <boost/asio.hpp>
#include <mutex>

struct Position {
    float x;
    float y;
};
typedef struct Position Position;

extern Position consigne; // Déclaration uniquement
extern std::mutex consigne_mutex; // Déclaration uniquement
extern double currentXError;
extern double currentYError;
extern double dXError;
extern double dYError;
extern double summedXError;
extern double summedYError;
extern double gainK; 
extern double correctorTimeConstant;
extern double correctorTimeConstantC; 
extern double correctorTimeConstantD;
extern bool running;


// Fonctions de contrôle
void calculerCommande(Position* mesure, Position* consigne, Position* commande);
std::string formaterCommande(Position* commande);
void envoyerCommande(Position* commande, boost::asio::serial_port& serial);
void asservirServo(Position* mesure, boost::asio::serial_port& serial);

#endif // COMMANDE_H