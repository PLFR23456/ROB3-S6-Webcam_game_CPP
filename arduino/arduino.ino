#include <Servo.h>

const bool SPEAK = false;

// Instancier la classe Servo
Servo joint1Servo;  
Servo joint2Servo;  

bool turnMode = false;
bool programRunning = true;

// Déclarer le buffer du port serie
String serialMessage = "";

int commandePosition[2] = {};

void setup() {
  // Configurer le port serie
  Serial.begin(115200);

  while (!Serial); // attendre l'ouverture du port

  Serial.println("READY");

  // Attacher les instance de servomoteurs à leur port de commande (signal PWM)
  joint1Servo.attach(6); 
  joint2Servo.attach(5);
}

unsigned long previousTime = 0;
unsigned long interval = 100; // Durée entre chaque itération en ms (=> 10 Hz)

unsigned long lastDisplayTime = 0;
unsigned long displayInterval = 1000; // Affichage toutes les 1s

unsigned long lastCommandTime = 0;
const unsigned long timeout = 2000; // 2 secondes


void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - previousTime >= interval) {
    previousTime = currentTime;

    if (programRunning) {
      conversionMessageSerieVersCommandeServo(commandePosition);
      commanderBras(joint1Servo, joint2Servo, commandePosition[0], commandePosition[1]);
    } 
  }

  if (SPEAK) {
    if (currentTime - lastDisplayTime >= displayInterval) {
      lastDisplayTime = currentTime;
      if (millis() - lastCommandTime >= timeout) {
        Serial.println("Aucune commande reçue");
      } else {
        Serial.print("baseServo : ");
        Serial.print(commandePosition[0]);
        Serial.print(" | armServo : ");
        Serial.println(commandePosition[1]);  // Terminer par println()
      }
    }
  }

}

void conversionMessageSerieVersCommandeServo(int servoPos[2]) {
  while (Serial.available()) {
    char c = Serial.read();

    if (c != '\n') {
      serialMessage += c;
    } else {
      serialMessage.trim(); // Supprimer les espaces

      if (serialMessage == "STOP!") {
        programRunning = false;
        Serial.println("ACK: STOP");
        Serial.println("Arrêt des moteurs");
        joint1Servo.detach();
        joint2Servo.detach();
      } else {
        int spaceIndex = serialMessage.indexOf(' ');
        if (spaceIndex > 0 && spaceIndex < serialMessage.length() - 1) {
          String val1 = serialMessage.substring(0, spaceIndex);
          String val2 = serialMessage.substring(spaceIndex + 1);

          if (val1.length() > 0 && val2.length() > 0) {
            servoPos[0] = val1.toInt();
            servoPos[1] = val2.toInt();

            lastCommandTime = millis();

            Serial.print("ACK: ");
            Serial.print(servoPos[0]);
            Serial.print(" ");
            Serial.println(servoPos[1]);
          } else {
            Serial.println("ERR: Valeurs manquantes");
          }
        } else {
          Serial.println("ERR: Commande invalide");
        }
      }

      serialMessage = "";
    }
  }
}

void commanderBras(Servo baseServo, Servo armServo, int baseServoPos, int armServoPos) {
  // Envoyer les commandes aux servomoteurs
  baseServo.write(baseServoPos);
  armServo.write(armServoPos);
}

    
