#include <Servo.h>

// Instancier la classe Servo
Servo joint1Servo;  
Servo joint2Servo;  

bool turnMode = false;

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
  commanderBras(joint1Servo, joint2Servo);
}

void commanderBras(Servo baseServo, Servo armServo) {
  // Récupérer le signal entrant brut (caractères sous format binaire) du port serie dans un buffer 
  if (Serial.available()) int serialCharacter = Serial.read();
  
  // Ajoute le caractère au buffer (chaîne)
  if (isDigit(serialCharacter)) serialMessage += (char)serialCharacter;

  // Traitement du signal
  if (serialCharacter == ' ') { // Si le caractère courant est un espace
    // Convertir le premier mot du message (commande du premier servomoteur) en entier
    int baseServoPos = serialMessage.toInt();
    
    Serial.println("Commandes de position angulaire :");
    Serial.print("baseServo :");
    Serial.println(baseServoPos);

    // Envoyer la commande au servomoteur
    myservo1.write(baseServo);

    // Vider le buffer
    serialMessage = "";
  } else if (serialCharacter == '\n') { // Si le caractère courant est un retour à la ligne
    // Convertir le second mot du message (commande du second servomoteur) en entier
    int armServoPos = serialMessage.toInt();
    
    Serial.println("Commandes de position angulaire :");
    Serial.print("armServo :");
    Serial.println(armServoPos);

    // Envoyer la commande au servomoteur
    myservo1.write(armServo);

    // Vider le buffer
    serialMessage = "";
  }

  if (serialCharacter == 't') {
    Serial.println("Turn mode !");
    turnMode=!turnMode;
    myservo1.write(90);
    myservo2.write(90);
    serialMessage = "";
  }
    
  delay(10);   // waits 15ms for the servo to reach the position
    
  if (turnMode) {
    myservo1.write(90);
    myservo2.write(10);
    delay(100);
    myservo2.write(90);
    delay(100);
    return;  // Ne pas lire le port série pendant le mode "turn"
  }
}
