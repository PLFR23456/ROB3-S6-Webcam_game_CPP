#include "camera.hpp"
#include "commande.hpp"
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


void MaskCreation(cv::Mat& frame, cv::Mat& mask, cv::Mat& hsv, int& max_area, int& max_idx, std::vector<std::vector<cv::Point>>& contours, cv::Point2f& color_center, int& count) {
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        // Conversion HSV et seuillage adaptatif pour robustesse à la luminosité
        cv::inRange(hsv, Mask1.mini, Mask1.maxi, mask);

        // Amélioration du masque : ouverture-fermeture pour réduire bruit et combler trous
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 2);

        // Recherche du plus grand contour (l'objet le plus gros)
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        
        for (size_t i = 0; i < contours.size(); ++i) {
            int area = cv::contourArea(contours[i]);
            if (area > max_area) {
            max_area = area;
            max_idx = i;
            }
        }
        if (max_idx != -1 && max_area > Mask1.minArea) {
            // Calcul du centre de position du plus grand contour
            cv::Moments mu = cv::moments(contours[max_idx]);
            if (mu.m00 != 0) {
            color_center = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
            count = max_area;
            }
            // dessiner le contour suivi
            cv::drawContours(frame, contours, max_idx, cv::Scalar(255,0,143), 2);
        }

}




void traiterCamera(cv::VideoCapture& cap, ProcessedFrame& data) {
    cv::Mat frame, hsv, mask;
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);

    // Charger le labyrinthe
    Labyrinthe lab;
    int locallabnumber = labnumber;
    lab.image = cv::imread("./extras/lab"+std::to_string(locallabnumber) +".jpg", cv::IMREAD_GRAYSCALE);
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

        if(locallabnumber!= labnumber) {
            std::cout << "changement de lab !" << std::endl;
            locallabnumber = labnumber;
            lab.image = cv::imread("./extras/lab"+std::to_string(locallabnumber) +".jpg", cv::IMREAD_GRAYSCALE);
            if(lab.image.empty()) {
                std::cerr << "Erreur: Impossible de charger le labyrinthe!" << std::endl;
                return;
            }
            // Redimensionner le labyrinthe à la taille de la caméra
            

        }
        // cv::resize(lab.image, lab.image, cv::Size(cam_width, cam_height));
        // //afficher lab.image sur frame
        // cv::Mat lab_colored;
        // cv::cvtColor(lab.image, lab_colored, cv::COLOR_GRAY2BGR);
        // cv::addWeighted(frame, 0.5, lab_colored, 0.5, 0, frame);
        //-------------------SUIVI DE COULEUR-------------------//
        int max_area = 0;
        int max_idx = -1;
        std::vector<std::vector<cv::Point>> contours;
        cv::Point2f color_center;
        int count = 0;
        MaskCreation(frame, mask, hsv,max_area, max_idx, contours, color_center,count);
        //---------------FIN-DE-SUIVI DE COULEUR----------------//
        
        
        

        // Si on a trouvé assez de pixels de la couleur
        if(count > Mask1.minArea && game.getPageStatus()) {
            // Vérifier la collision avec les murs
            int ccx = static_cast<int>(color_center.x);
            int ccy = static_cast<int>(color_center.y);
            
            // S'assurer que les coordonnées sont dans les limites
            if(ccx >= 0 && ccx < lab.image.cols && ccy >= 0 && ccy < lab.image.rows && game.getLabyrinthStatus()) {
                if(game.getStatus() == GameStatus::NOT_PLAYING || game.getStatus() == GameStatus::INITIALIZING) {    
                    if(ccx<startbox.x+30 & ccx>=startbox.x & ccy<startbox.y+30 & ccy>=startbox.y) {
                        std::cout << "STARTBOX ! " << std::endl;
                        game.enterStartBox();
                        std::cout << "DANS LA START BOX" << std::endl;
                    }
                    else {
                        game.idleGame();
                        std::cout << "HORS DE LA START BOX" << std::endl;
                    }
                }

                else{if(ccx<endbox.x+30 & ccx>=endbox.x & ccy<endbox.y+30 & ccy>=endbox.y) {
                        std::cout << "ENDBOX ! " << std::endl;
                        game.enterEndBox();

                    }
                }
                // Si le pixel est noir (mur), c'est une collision
                if (lab.image.at<uchar>(ccy, ccx) < 128) {
                    if(game.getStatus() == GameStatus::PLAYING) {
                        std::cout << "PERDU ! Collision avec un mur" << std::endl;
                        game.touchWall(); // si on touche un mur
                    }
                    // Option : retour au début
                    // consigne.x = lab.startPos.x;
                    // consigne.y = lab.startPos.y;
                }
                
            }


        //----------------------DESSIN--------------------------//
            // Dessiner le point de départ (vert) et d'arrivée (rouge)
            cv::circle(frame, lab.startPos, 10, cv::Scalar(0,255,0), -1);
            cv::circle(frame, lab.endPos, 10, cv::Scalar(0,0,255), -1);
            
            // // Calculer l'écart avec le centre de la caméra
            // float dx = color_center.x - cam_center.x;
            // float dy = color_center.y - cam_center.y;

            // Trouver et dessiner les contours du masque
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            cv::circle(frame, color_center, 5, cv::Scalar(130,0,74), -1);  // Point vert: centre de la couleur

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

        //--------------------FIN DE DESSIN---------------------//


            // met a jour la variable global
            // std::lock_guard<std::mutex> lock(consigne_mutex); // se ferme tout seul à la fin du "}"
            // consigne.x = color_center.x;
            // consigne.y = color_center.y;
        }

        else{
            // std::lock_guard<std::mutex> lock(consigne_mutex);
            // consigne.x = 320;
            // consigne.y = 240;
        }
        //--------------------GRADIENT---------------------//
        // cv::Mat hsv_grad(grad_size, grad_size, CV_8UC3);
        // for (int y = 0; y < grad_size; ++y) {
        //     for (int x = 0; x < grad_size; ++x) {
        //         // Interpolation linéaire entre mini et maxi
        //         int h = Mask1.mini[0] + x * (Mask1.maxi[0] - Mask1.mini[0]) / (grad_size - 1);
        //         int s = Mask1.mini[1] + y * (Mask1.maxi[1] - Mask1.mini[1]) / (grad_size - 1);
        //         int v = (Mask1.mini[2] + Mask1.maxi[2]) / 2; // Valeur centrale du V
        //         hsv_grad.at<cv::Vec3b>(y, x) = cv::Vec3b(h, s, v);
        //     }
        // }
        // // Conversion HSV -> BGR pour affichage
        // cv::Mat bgr_grad;
        // cv::cvtColor(hsv_grad, bgr_grad, cv::COLOR_HSV2BGR);
        // // Position en bas à droite
        // int x_offset = frame.cols - grad_size - 10;
        // int y_offset = frame.rows - grad_size - 10;
        // // Affichage du carré sur la frame
        // bgr_grad.copyTo(frame(cv::Rect(x_offset, y_offset, grad_size, grad_size)));
        //--------------------FIN DE GRADIENT---------------------//


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
