#include <Servo.h>

#define SPEAK 0

// Instancier la classe Servo
Servo joint1Servo;  
Servo joint2Servo;  

bool turnMode = false;
bool programRunning = true;

// Déclarer le buffer du port serie
String serialMessage = "";

int commandePosition[2] = {};

int periodeAffichage;
int compteurAffichage;

void setup() {
  // Configurer le port serie
  Serial.begin(115200);

  // Attacher les instance de servomoteurs à leur port de commande (signal PWM)
  joint1Servo.attach(6); 
  joint2Servo.attach(5);

  periodeAffichage = 10000;
  compteurAffichage = 0;

  unsigned long lastDisplayTime = 0;
  unsigned long displayInterval = 1000; // Affichage toutes les 1s
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
    } else {
      Serial.println("Arrêt des moteurs");
      joint1Servo.detach();
      joint2Servo.detach();
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

    // Accumule les caractères sauf retour à la ligne
    if (c != '\n') {
      serialMessage += c;
    } else {
      // Une ligne complète a été reçue, traitement
      serialMessage.trim(); // Supprime les espaces en début/fin

      // Cas spécial : STOP!
      if (serialMessage == "STOP!") {
        programRunning = false;
      } else {
        // Séparer les deux valeurs avec l’espace
        int spaceIndex = serialMessage.indexOf(' ');
        if (spaceIndex > 0) {
          String val1 = serialMessage.substring(0, spaceIndex);
          String val2 = serialMessage.substring(spaceIndex + 1);

          // Conversion et stockage
          servoPos[0] = val1.toInt();
          servoPos[1] = val2.toInt();

          lastCommandTime = millis();
        }
      }

      // Réinitialiser pour la prochaine ligne
      serialMessage = "";
    }
  }
}

void commanderBras(Servo baseServo, Servo armServo, int baseServoPos, int armServoPos) {
  // Envoyer les commandes aux servomoteurs
  baseServo.write(baseServoPos);
  armServo.write(armServoPos);
}

    
