#ifndef DISPLAY_H
#define DISPLAY_H
extern bool isGamePageOpen;
//isGamePageOpen si on est pas sur l'écran logo
extern bool isGamePre_Started;
//si la partie est lancée
extern bool isGameStarted;
// si on a passe la startbox
extern int status;
extern Position startbox;
extern Position endbox;
extern bool isCameraShaking;
//doc
// 0= not playing
// 1= in the start box (waiting ~3 seconds)
// 2= playing
// 3= in the end box
// 4= touching the walls 
int display();
#endif