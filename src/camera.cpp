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


void traiterCamera(cv::VideoCapture& cap, ProcessedFrame& data) {
    cv::Mat frame, hsv, mask;
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);

    // Charger le labyrinthe
    Labyrinthe lab;
    lab.image = cv::imread("./extras/labyrinth.png", cv::IMREAD_GRAYSCALE);
    if(lab.image.empty()) {
        std::cerr << "Erreur: Impossible de charger le labyrinthe!" << std::endl;
        return;
    }
    
    // Redimensionner le labyrinthe à la taille de la caméra
    cv::resize(lab.image, lab.image, cv::Size(cam_width, cam_height));
    
    while (!stop_signal) {
        cap >> frame;
        if (frame.empty()) continue;
        frame.copyTo(frame_for_click);
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, Mask1.mini, Mask1.maxi, mask);

        // Lissage du masque
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::erode(mask, mask, kernel);
        cv::erode(mask, mask, kernel);
        cv::erode(mask, mask, kernel);
        cv::dilate(mask, mask, kernel);
        
        // Variables pour calculer le centre
        int sumX = 0, sumY = 0;
        int count = 0;
        int xmin = mask.cols, xmax = 0,ymin = mask.rows, ymax = 0;
        // Parcourir le masque pour trouver tous les pixels de la couleur
        for(int y = 0; y < mask.rows; y++) {
            for(int x = 0; x < mask.cols; x++) {
                if(mask.at<uchar>(y, x) > 0) {  // Si le pixel est blanc dans le masque
                    sumX += x;
                    sumY += y;
                    count++;
                }
            }
        }

        // Si on a trouvé assez de pixels de la couleur
        if(count > Mask1.minArea && running==true) {
            cv::Point2f color_center(sumX/float(count), sumY/float(count));
            
            // Vérifier la collision avec les murs
            int ccx = static_cast<int>(color_center.x);
            int ccy = static_cast<int>(color_center.y);
            
            // S'assurer que les coordonnées sont dans les limites
            if(ccx >= 0 && ccx < lab.image.cols && ccy >= 0 && ccy < lab.image.rows && jeu==true) {
                // Si le pixel est noir (mur), c'est une collision
                if(lab.image.at<uchar>(ccy, ccx) < 128) {
                    std::cout << "PERDU ! Collision avec un mur" << std::endl;
                    status = 4; // Mettre à jour le statut pour indiquer une collision
                    // Option : retour au début
                    // consigne.x = lab.startPos.x;
                    // consigne.y = lab.startPos.y;
                }
                else {
                    status = 2; // Pas de collision
                }
            }

            // Dessiner le point de départ (vert) et d'arrivée (rouge)
            cv::circle(frame, lab.startPos, 10, cv::Scalar(0,255,0), -1);
            cv::circle(frame, lab.endPos, 10, cv::Scalar(0,0,255), -1);
            
            // Calculer l'écart avec le centre de la caméra
            float dx = color_center.x - cam_center.x;
            float dy = color_center.y - cam_center.y;

            // Trouver et dessiner les contours du masque
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            cv::circle(frame, color_center, 5, cv::Scalar(0,255,0), -1);  // Point vert: centre de la couleur
            cv::circle(frame, cam_center, 5, cv::Scalar(0,0,255), -1);    // Point rouge: centre caméra
            cv::line(frame, cam_center, color_center, cv::Scalar(255,0,0), 2);  // Ligne bleue entre les deux

            // Afficher les coordonnées et l'écart à l'écran
            std::string coord_text = "Position: (" + std::to_string(int(color_center.x)) + ", " + std::to_string(int(color_center.y)) + ")";
            int baseLine = 0;
            cv::Scalar textColor = (count > Mask1.minArea) ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255);
            cv::Scalar bgColor(0, 0, 0); // fond noir

            std::vector<std::string> lines = {
                "Position : (" + std::to_string(int(color_center.x)) + ", " + std::to_string(int(color_center.y)) + ")"};

            int x = 15, y = 20;
            for (const auto& line : lines) {
                int fontFace = cv::FONT_HERSHEY_SIMPLEX;
                double fontScale = 0.6;
                int thickness = 1;

                cv::Size textSize = cv::getTextSize(line, fontFace, fontScale, thickness, &baseLine);
                cv::rectangle(frame, cv::Point(x - 5, y - textSize.height - 2), cv::Point(x + textSize.width + 5, y + baseLine + 2), bgColor, cv::FILLED);
                cv::putText(frame, line, cv::Point(x, y), fontFace, fontScale, textColor, thickness);
                y += textSize.height + baseLine + 10;
            }

            // met a jour la variable global
            std::lock_guard<std::mutex> lock(consigne_mutex); // se ferme tout seul à la fin du "}"
            consigne.x = color_center.x;
            consigne.y = color_center.y;
        }

        else{
            std::lock_guard<std::mutex> lock(consigne_mutex);
            consigne.x = 320;
            consigne.y = 240;
        }
        cv::Mat hsv_grad(grad_size, grad_size, CV_8UC3);
        for (int y = 0; y < grad_size; ++y) {
            for (int x = 0; x < grad_size; ++x) {
                // Interpolation linéaire entre mini et maxi
                int h = Mask1.mini[0] + x * (Mask1.maxi[0] - Mask1.mini[0]) / (grad_size - 1);
                int s = Mask1.mini[1] + y * (Mask1.maxi[1] - Mask1.mini[1]) / (grad_size - 1);
                int v = (Mask1.mini[2] + Mask1.maxi[2]) / 2; // Valeur centrale du V
                hsv_grad.at<cv::Vec3b>(y, x) = cv::Vec3b(h, s, v);
            }
        }

        // Conversion HSV -> BGR pour affichage
        cv::Mat bgr_grad;
        cv::cvtColor(hsv_grad, bgr_grad, cv::COLOR_HSV2BGR);


        // Position en bas à droite
        int x_offset = frame.cols - grad_size - 10;
        int y_offset = frame.rows - grad_size - 10;

        // Affichage du carré sur la frame
        bgr_grad.copyTo(frame(cv::Rect(x_offset, y_offset, grad_size, grad_size)));

        {
        std::lock_guard<std::mutex> lock(data.mutex);
        frame.copyTo(data.frame); // copie la frame courante dans la structure partagée
        mask.copyTo(data.mask);   // copie le masque courant dans la structure partagée
        data.ready = true;        // indique qu'une nouvelle frame est prête
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // pour ne pas surcharger
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
