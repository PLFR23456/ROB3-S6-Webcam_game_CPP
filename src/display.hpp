#ifndef DISPLAY_H
#define DISPLAY_H
extern bool running;
extern int status;
//doc
// 0= not playing
// 1= in the start box (waiting ~3 seconds)
// 2= playing
// 3= in the end box
// 4= touching the walls 
int display();
#endif