#include "commande.hpp"
#include "camera.hpp"
#include "display.hpp"

#include <iostream> // Pour afficher des messages dans la console
#include <thread> // Pour la gestion des threads
#include <mutex> // Pour protéger les variables partagées

// Surface minimale pour filtrer le bruit
MasqueCouleur Mask1 {
    cv::Scalar(140, 140, 140),
    cv::Scalar(190, 255, 255),
    50
};

int sources = 2 ;
int screensources = 0; //0 = camera ; 1 = masque

// Variable modifiable par le trackbar
// TODO : Ajouter une protection mutex pour ces variables (peut-être créer un tampon ?) 
double gainK = 0.1;
double correctorTimeConstant = 0.01; 
double correctorTimeConstantC = 0.01; 
double correctorTimeConstantD = 0.01;
// TODO FIN
int tol = 20; // Définition globale
const int grad_size = 120;

cv::Mat frame_for_click; // Pour stocker la frame pour le clic
cv::Scalar last_color; // À déclarer en global

ProcessedFrame processed_data;

void onGainKChange(int value, void*) { gainK = value / 100.0;} // Le trackbar va de 0 à 200, donc gainK de 0.0 à 2.0
void onCorrectorTimeConstantChange(int value, void*) {correctorTimeConstant = value / 1000.0;} // Le trackbar va de 0 à 2000, donc correctorTimeConstant de 0.0 à 2.0
void onCorrectorTimeConstantCChange(int value, void*) {correctorTimeConstantC = value / 1000.0;} // Le trackbar va de 0 à 2000, donc correctorTimeConstantC de 0.0 à 2.0
void onCorrectorTimeConstantDChange(int value, void*) {correctorTimeConstantD = value / 1000.0;} // Le trackbar va de 0 à 2000, donc correctorTimeConstantD de 0.0 à 2.0
void onTolChange(int value, void*) {tol = value;}

// Définition des plages de couleurs pour différents vêtements
const MasqueCouleur VETEMENT_ROUGE {
    cv::Scalar(160, 100, 100),  // Rouge HSV min
    cv::Scalar(180, 255, 255),  // Rouge HSV max
    500  // Surface minimale pour éviter le bruit
};

const MasqueCouleur VETEMENT_BLEU {
    cv::Scalar(100, 100, 100),  // Bleu HSV min
    cv::Scalar(130, 255, 255),  // Bleu HSV max
    500
};

const MasqueCouleur VETEMENT_VERT {
    cv::Scalar(40, 100, 100),   // Vert HSV min
    cv::Scalar(80, 255, 255),   // Vert HSV max
    500
};

void traiterCamera(cv::VideoCapture& cap, ProcessedFrame& data) {
    cv::Mat frame, hsv, mask_rouge, mask_bleu, mask_vert;
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);

    while (!stop_signal) {
        cap >> frame;
        if (frame.empty()) continue;

        frame.copyTo(frame_for_click);
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        // Détection pour chaque couleur
        cv::inRange(hsv, VETEMENT_ROUGE.mini, VETEMENT_ROUGE.maxi, mask_rouge);
        cv::inRange(hsv, VETEMENT_BLEU.mini, VETEMENT_BLEU.maxi, mask_bleu);
        cv::inRange(hsv, VETEMENT_VERT.mini, VETEMENT_VERT.maxi, mask_vert);

        // Lissage des masques
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::erode(mask_rouge, mask_rouge, kernel);
        cv::dilate(mask_rouge, mask_rouge, kernel);
        cv::erode(mask_bleu, mask_bleu, kernel);
        cv::dilate(mask_bleu, mask_bleu, kernel);
        cv::erode(mask_vert, mask_vert, kernel);
        cv::dilate(mask_vert, mask_vert, kernel);

        // Détection des contours pour chaque couleur
        std::vector<std::vector<cv::Point>> contours;
        
        // Pour les vêtements rouges
        cv::findContours(mask_rouge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for(const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if(area > VETEMENT_ROUGE.minArea) {
                cv::Rect bbox = cv::boundingRect(contour);
                cv::rectangle(frame, bbox, cv::Scalar(0,0,255), 2);
                cv::putText(frame, "Vetement Rouge", 
                           cv::Point(bbox.x, bbox.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                           cv::Scalar(0,0,255), 2);
            }
        }

        // Pour les vêtements bleus
        cv::findContours(mask_bleu, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for(const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if(area > VETEMENT_BLEU.minArea) {
                cv::Rect bbox = cv::boundingRect(contour);
                cv::rectangle(frame, bbox, cv::Scalar(255,0,0), 2);
                cv::putText(frame, "Vetement Bleu", 
                           cv::Point(bbox.x, bbox.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                           cv::Scalar(255,0,0), 2);
            }
        }

        // Pour les vêtements verts
        cv::findContours(mask_vert, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        for(const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if(area > VETEMENT_VERT.minArea) {
                cv::Rect bbox = cv::boundingRect(contour);
                cv::rectangle(frame, bbox, cv::Scalar(0,255,0), 2);
                cv::putText(frame, "Vetement Vert", 
                           cv::Point(bbox.x, bbox.y - 10),
                           cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                           cv::Scalar(0,255,0), 2);
            }
        }

        {
            std::lock_guard<std::mutex> lock(data.mutex);
            frame.copyTo(data.frame);
            // Combinons les masques pour l'affichage
            data.mask = mask_rouge | mask_bleu | mask_vert;
            data.ready = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    while (!stop_signal) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
}

int camera() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Erreur: Impossible d'ouvrir la webcam!" << std::endl;
        return -1;
    }

    // Création des trackbars
    
    // Création du callback pour la souris

    std::thread worker(traiterCamera, std::ref(cap), std::ref(processed_data));

    while (!stop_signal) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

    worker.join();
    return 0;
}
