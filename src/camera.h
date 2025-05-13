#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <iostream>

// Déclaration des variables globales pour la position de la souris
extern int mouse_x;
extern int mouse_y;

// Déclaration de la fonction de callback pour la souris
void onMouse(int event, int x, int y, int flags, void* userdata);

// Déclaration de la fonction principale camera
int camera();

#endif // CAMERA_H