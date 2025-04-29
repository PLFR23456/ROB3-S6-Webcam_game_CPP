#include <Servo.h>

Servo myservo1;  // create servo object to control a servo
Servo myservo2;  // create servo object to control a servo
bool turnMode=false;
String inString="";
void setup() {
  Serial.begin(9600);

  myservo1.attach(5);  // attaches the servo on pin 9 to the servo object
  myservo2.attach(6);  // attaches the servo on pin 9 to the servo object

}


void loop() {


    if (Serial.available() > 0) { //si qqchose est envoyé dans le Serial
     int inChar = Serial.read();

    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
     }
    // if you get a newline, print the string, then the string's value:
    if (inChar == ' ') {
      Serial.print("Value:");
      int pos1 =inString.toInt();
      Serial.println(pos1);
      Serial.print("String: ");
      Serial.println(inString);
      // clear the string for new input:
      myservo1.write(pos1);              // tell servo to go to position in variable 'pos'

      inString = "";
      }
          // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      Serial.print("Value:");
      int pos2 =inString.toInt();
      Serial.println(pos2);
      Serial.print("String: ");
      Serial.println(inString);
      // clear the string for new input:
      myservo2.write(pos2);              // tell servo to go to position in variable 'pos'

      inString = "";
      }
    if (inChar == 't') {
      Serial.println("Turn mode !");
      turnMode=!turnMode;
      myservo1.write(90);
      myservo2.write(90);
      inString = "";
      }
      delay(10);   // waits 15ms for the servo to reach the position
    }
          if (turnMode) {
      myservo1.write(90);
      myservo2.write(10);
      delay(100);
      myservo2.write(90);
      delay(100);
      return;  // Ne pas lire le port série pendant le mode "turn"
    }
}
