#include <Servo.h>

// ------------------ VARIABLES GLOBALES ------------------ //
const bool SPEAK = false;
bool programRunning = true;

// Instancier la classe Servo
Servo base;  
Servo arm;  

// Déclarer le tableau des commandes de position angulaire
int positionCommand[2] = {};

// Cadencement de la boucle principale
unsigned long currentTime;
unsigned long previousTime = 0;
const unsigned long loopPeriod = 100; // Durée entre chaque itération en ms (=> 10 Hz)

// Cadencement de l'affichage (toute les printingPeriod tour de boucles)
unsigned long printingCounter = 0;
const unsigned long printingPeriod = 1000; // Affichage toutes les 1s

// Détection des timeout 
unsigned long lastCommandCounter = 0;
const unsigned long lastCommandTimeout = 2000; // 2 secondes


// ------------------ SETUP ------------------ //
void setup() {
  // Attacher les instance de servomoteurs à leur port de commande (signal PWM)
  base.attach(6); 
  arm.attach(5);

  // Configurer le port serie
  Serial.begin(115200);

  // Attendre l'ouverture du port
  while (!Serial); 

  // Signaler que tout est prêt
  Serial.println("READY");
}

// ------------------ LOOP ------------------ //
void loop() {
  currentTime = millis();
  
  if (currentTime - previousTime >= loopPeriod) {
    previousTime = currentTime;

    if (programRunning) {
      serialMessageToCommand(positionCommand);
      sendCommand(base, arm, positionCommand[0], positionCommand[1]);
    } 
  }

  if (SPEAK) { // Si on est autorisé à parler
    if (currentTime - printingCounter >= printingPeriod) {
      printingCounter = currentTime;
      if (millis() - lastCommandCounter >= lastCommandTimeout) {
        Serial.println("Aucune commande reçue");
      } else {
        Serial.print("SPEAK: ");
        Serial.print(positionCommand[0]);
        Serial.print(" ");
        Serial.println(positionCommand[1]);
      }
    }
  }

}

// ------------------ FONCTIONS ------------------ //
void serialMessageToCommand(int servoPosCommand[2]) {
  // Buffer pour la lecture du port série
  String serialMessage = "";
  while (Serial.available()) {
    char c = Serial.read();

    if (c != '\n') serialMessage += c;
    else {
      serialMessage.trim(); // Supprimer les espaces

      if (serialMessage == "STOP!") {
        programRunning = false;

        // Acquittement (on accuse la bonne réception de la demande d'arrêt)
        Serial.println("ACK: STOP");

        // On déconnecte les moteurs
        base.detach();
        arm.detach();
      } else {
        int spaceIndex = serialMessage.indexOf(' '); // On applique la méthode sur la string ppur trouver l'indice de l'espace
        if (spaceIndex > 0 && spaceIndex < serialMessage.length() - 1) { // Il ne doit y avoir qu'un seul espace et il doit être au centre des deux valeurs (ni au début ni à la fin du message)
          String val1 = serialMessage.substring(0, spaceIndex); // On récupère la valeur de la première commande
          String val2 = serialMessage.substring(spaceIndex + 1); // On récupère la valeur de la seconde commande

          if (val1.length() > 0 && val2.length() > 0) {
            lastCommandCounter = millis(); // On a bien reçu une commande

            servoPosCommand[0] = val1.toInt();
            servoPosCommand[1] = val2.toInt();

            // Acquittement (on accuse la bonne réception de la commande)
            Serial.print("ACK: ");
            Serial.print(servoPosCommand[0]);
            Serial.print(" ");
            Serial.println(servoPosCommand[1]);
          } else {
            Serial.println("ERR: Valeurs manquantes");
          }
        } else {
          Serial.println("ERR: Commande invalide");
        }
      }
    }
  }
}

void sendCommand(Servo baseServo, Servo armServo, int baseServoPosCommand, int armServoPosCommand) {
  // Envoyer les commandes aux servomoteurs
  baseServo.write(baseServoPosCommand);
  armServo.write(armServoPosCommand);
}

    
