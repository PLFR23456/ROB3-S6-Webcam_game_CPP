#include <Servo.h>

// Instancier la classe Servo
Servo joint1Servo;  
Servo joint2Servo;  

bool turnMode = false;
bool programRunning = true;

// Déclarer le buffer du port serie
String serialMessage = "";

void setup() {
  // Configurer le port serie
  Serial.begin(9600);

  // Attacher les instance de servomoteurs à leur port de commande (signal PWM)
  joint1Servo.attach(5); 
  joint2Servo.attach(6);
}


void loop() {
  if (programRunning) {
    commanderBras(joint1Servo, joint2Servo);
  }
}

void commanderBras(Servo baseServo, Servo armServo) {
  if (Serial.available()) { // Si un signal est détecté sur le port serie
    int serialCharacter = Serial.read(); // Récupérer le signal entrant brut (caractères sous format binaire) du port serie dans un buffer 
    
    // Ajoute le caractère au buffer (chaîne)
    if (isDigit(serialCharacter)) serialMessage += (char)serialCharacter;

    // Traitement du signal
    if (serialCharacter == ' ') { // Si le caractère courant est un espace
      // Convertir le premier mot du message (commande du premier servomoteur) en entier
      int baseServoPos = serialMessage.toInt();
      
      Serial.println("Commandes de position angulaire :");
      Serial.print("baseServo : ");
      Serial.println(baseServoPos);

      // Envoyer la commande au servomoteur
      baseServo.write(baseServoPos);

      // Vider le buffer
      serialMessage = "";
    } else if (serialCharacter == '\n') { // Si le caractère courant est un retour à la ligne
      // Convertir le second mot du message (commande du second servomoteur) en entier
      int armServoPos = serialMessage.toInt();
      
      Serial.println("Commandes de position angulaire :");
      Serial.print("armServo : ");
      Serial.println(armServoPos);

      // Envoyer la commande au servomoteur
      armServo.write(armServoPos);

      // Vider le buffer
      serialMessage = "";
    }

    if (serialCharacter == '!') {
      if (serialMessage = "STOP") {
        programRunning = false;
        Serial.println("Arrêt des moteurs");
        baseServo.detach();
        armServo.detach();
      }
    }
    if (serialCharacter == 't') {
      Serial.println("Turn mode !");
      turnMode=!turnMode;
      baseServo.write(90);
      armServo.write(90);
      serialMessage = "";
    }
      
    delay(10);   // waits 15ms for the servo to reach the position
      
    if (turnMode) {
      baseServo.write(90);
      armServo.write(10);
      delay(100);
      armServo.write(90);
      delay(100);
      return;  // Ne pas lire le port série pendant le mode "turn"
    }
  }
}
