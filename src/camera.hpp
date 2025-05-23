#ifndef CAMERA_H
#define CAMERA_H

#include <atomic>
#include <mutex>

extern std::atomic<bool> stop_signal;

//VARIABLES MODIFIABLES
extern int tol;
extern double gainK; // Gain K pour le PID
// Déclaration de la fonction de callback pour la souris
void onMouse(int event, int x, int y, int flags, void* userdata);

// Déclaration de la fonction principale camera
int camera();

#endif // CAMERA_H