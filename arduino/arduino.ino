#include <Servo.h>

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
  joint1Servo.attach(5); 
  joint2Servo.attach(6);

  periodeAffichage = 10000;
  compteurAffichage = 0;
}

void loop() {
  compteurAffichage++;

  if (programRunning) {
    conversionMessageSerieVersCommandeServo(commandePosition);
    
    // Affiche les valeurs
    if (compteurAffichage == periodeAffichage) {
      // Afficher les commandes
      // Serial.println("Commandes de position angulaire :");
      // Serial.print("baseServo : ");
      // Serial.println(commandePosition[0]);
      // Serial.println("Commandes de position angulaire :");
      // Serial.print("armServo : ");
      // Serial.println(commandePosition[1]);

      compteurAffichage = 0;
    }

    commanderBras(joint1Servo, joint2Servo, commandePosition[0], commandePosition[1]);
    
  } else {
    Serial.println("Arrêt des moteurs");
    joint1Servo.detach();
    joint2Servo.detach();
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
        }
      }

      // Réinitialiser pour la prochaine ligne
      serialMessage = "";
    }
  }
}

void commanderBras(Servo baseServo, Servo armServo, int baseServoPos, int armServoPos) {
  // Afficher les commandes
  // Serial.println("Commandes de position angulaire :");
  // Serial.print("baseServo : ");
  // Serial.println(baseServoPos);
  // Serial.println("Commandes de position angulaire :");
  // Serial.print("armServo : ");
  // Serial.println(armServoPos);

  // Envoyer les commandes aux servomoteurs
  baseServo.write(baseServoPos);
  armServo.write(armServoPos);
}

    
