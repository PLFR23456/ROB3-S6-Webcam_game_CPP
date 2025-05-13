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
  Serial.begin(9600);

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
      Serial.println("Commandes de position angulaire :");
      Serial.print("baseServo : ");
      Serial.println(commandePosition[0]);
      Serial.println("Commandes de position angulaire :");
      Serial.print("armServo : ");
      Serial.println(commandePosition[1]);

      compteurAffichage = 0;
    }

    //commanderBras(joint1Servo, joint2Servo, commandePosition[0], commandePosition[1]);
    
  } else {
    Serial.println("Arrêt des moteurs");
    joint1Servo.detach();
    joint2Servo.detach();
  }
}

void conversionMessageSerieVersCommandeServo(int servoPos[2]) {
  if (Serial.available()) { // Si un signal est détecté sur le port serie
    int serialCharacter = Serial.read(); // Récupérer le signal entrant brut (caractères sous format binaire) du port serie dans un buffer

    // Ajoute le caractère au buffer (chaîne)
    if (isDigit(serialCharacter)) serialMessage += (char)serialCharacter; 

    // Traitement du signal
    if (serialCharacter == ' ') { // Si le caractère courant est un espace
      // Convertir le premier mot du message (commande du premier servomoteur) en entier
      servoPos[0] = serialMessage.toInt();

      // Vider le buffer
      serialMessage = "";
    } else if (serialCharacter == '\n') { // Si le caractère courant est un retour à la ligne
      // Convertir le second mot du message (commande du second servomoteur) en entier
      servoPos[1] = serialMessage.toInt();

      // Vider le buffer
      serialMessage = "";
    }

    if ((serialCharacter == '!') && (serialMessage = "STOP")) programRunning = false;
  }
}

void commanderBras(Servo baseServo, Servo armServo, int baseServoPos, int armServoPos) {
  // Afficher les commandes
  Serial.println("Commandes de position angulaire :");
  Serial.print("baseServo : ");
  Serial.println(baseServoPos);
  Serial.println("Commandes de position angulaire :");
  Serial.print("armServo : ");
  Serial.println(armServoPos);

  // Envoyer les commandes aux servomoteurs
  baseServo.write(baseServoPos);
  armServo.write(armServoPos);
}

    
