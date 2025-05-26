#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp> // Acronyme de "Open Computer Vision"
#include <atomic>

// Déclaration de la variable de signal d'arrêt (atomic = pas besoin de mutex, pas toujours possible de l'utiliser)
extern std::atomic<bool> stop_signal;
extern int tol;
extern cv::Mat lastFrame;


// Structure tampon partagée entre le thread de traitement et le thread principal 
struct ProcessedFrame {
    cv::Mat frame;
    cv::Mat mask;
    std::mutex mutex;
    bool ready = false;
};
extern ProcessedFrame processed_data;

struct MasqueCouleur {
    cv::Scalar mini;  // Min HSV
    cv::Scalar maxi;  // Max HSV
    int minArea;      // Aire minimale
};
extern MasqueCouleur Mask1;

// ----------------- SIGNATURES DES FONCTIONS ----------------- //
// Callback pour les trackbars (nécessaire même si vide)
void onGainKChange(int value, void*);
void onCorrectorTimeConstantChange(int value, void*);
void onCorrectorTimeConstantCChange(int value, void*);
void onCorrectorTimeConstantDChange(int value, void*);
void onTolChange(int value, void*);

// Déclaration de la fonction principale camera
void traiterCamera(cv::VideoCapture& cap, ProcessedFrame& data);
int camera();

#endif // CAMERA_H