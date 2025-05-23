#include "camera.hpp"

#include <opencv2/opencv.hpp> // Pour la vision par ordinateur
#include <iostream> // Pour afficher des messages dans la console
#include <thread> // Pour la gestion des threads
#include <mutex>

struct MasqueCouleur {
    cv::Scalar mini{140, 140, 140};  // 
    cv::Scalar maxi{190, 255, 255};  // 
    int minArea{50};                 // Surface minimale pour filtrer le bruit
} Mask1;

int sources = 2 ;
int screensources = 0; //0 = camera ; 1 = masque

cv::Mat frame_for_click; // Pour stocker la frame pour le clic
cv::Scalar last_color; // À déclarer en global

// Lorsqu'il y a un clic de la souris sur la fenetre
void onMouseSimple(int event, int x, int y, int, void*) {
    if (event == cv::EVENT_LBUTTONDOWN && !frame_for_click.empty()) {
        cv::Mat hsv;
        cv::cvtColor(frame_for_click, hsv, cv::COLOR_BGR2HSV);
        cv::Vec3b pix = hsv.at<cv::Vec3b>(y, x);
        Mask1.mini = cv::Scalar(
            std::max(0, pix[0] - tol),
            std::max(0, pix[1] - tol*2),
            std::max(0, pix[2] - tol*2)
        );
        Mask1.maxi = cv::Scalar(
            std::min(180, pix[0] + tol),
            std::min(255, pix[1] + tol*2),
            std::min(255, pix[2] + tol*2)
        );
        last_color = cv::Scalar(pix[0], pix[1], pix[2]); // Sauvegarde la couleur HSV sélectionnée        
        std::cout << "Nouvelle couleur HSV : " << (int)pix[0] << "," << (int)pix[1] << "," << (int)pix[2] << std::endl;
    }
}

int camera() {
    
    // Init - connexion à la caméra
    cv::VideoCapture cap(0);
    cv::namedWindow("Webcam");
    std::cout << "Appuyez sur 'q' pour quitter" << std::endl;
    // Erreur detection
    if (!cap.isOpened()) { // isOpened() renvoie 'true' si la caméra fonctionne, 'false' sinon
        std::cerr << "Erreur: Impossible d'ouvrir la webcam!" << std::endl;
        return -1;
    }
    
    //Initialisation des variables
    cv::Mat frame, hsv, mask;
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "Taille de la caméra: " << cam_width << "x" << cam_height << std::endl;
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);
    cv::setMouseCallback("Webcam", onMouseSimple, nullptr);

    bool pause = false;

    while (true) {
        if (!pause) {
            cap >> frame; // Met l'image de la caméra dans la matrice frame
            
            // Erreur detection
            if (frame.empty()) {
                std::cerr << "Erreur: Image vide!" << std::endl;
                break;
            }
            frame.copyTo(frame_for_click);
            cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV); // conversion HSV
            
            // création du masque -- function faite exprès pour
            cv::inRange(hsv, Mask1.mini, Mask1.maxi, mask);

            // Lissage du masque : ouverture morphologique (érosion puis dilatation)
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            cv::erode(mask, mask, kernel);
            cv::dilate(mask, mask, kernel);

            /**/
            // Calcul de la luminosité moyenne de l'image (canal V en HSV)
            cv::Scalar mean_hsv = cv::mean(hsv);
            int mean_v = static_cast<int>(mean_hsv[2]); // Valeur moyenne du canal V (luminosité)
            // Affichage de la luminosité moyenne
            
            // Ajuste dynamiquement la valeur minimale du masque selon la luminosité ambiante
            // Plus la pièce est sombre, plus on baisse la limite basse de V
            int v_min_base = 130;   // Valeur de base pour Vmin
            int v_max_base = 250; 
            int v_min_auto = std::max(0, v_min_base - (100 - mean_v)); // Plus mean_v est bas, plus v_min_auto baisse
            int v_max_auto = std::min(255, v_max_base + 50);
            // On applique la nouvelle valeur au masque
            
            //Mask1.mini[2] = v_min_auto;
            //Mask1.maxi[2] = v_max_auto;
        
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
                        if(x < xmin) xmin = x; //dimensions du rectangle
                        if(x > xmax) xmax = x;
                        if(y < ymin) ymin = y;
                        if(y > ymax) ymax = y; //
                    }
                }
            }

            // Si on a trouvé assez de pixels de la couleur
            if(count > Mask1.minArea) {
                // Calculer le point central
                cv::Point2f color_center(sumX/float(count), sumY/float(count));
            
                // Calculer l'écart avec le centre de la caméra
                float dx = color_center.x - cam_center.x;
                float dy = color_center.y - cam_center.y;

                // Trouver et dessiner les contours du masque
                std::vector<std::vector<cv::Point>> contours;
                std::vector<cv::Vec4i> hierarchy;
                cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::drawContours(frame, contours, -1, cv::Scalar(0,0,255), 2); // Rouge

                cv::circle(frame, color_center, 5, cv::Scalar(0,255,0), -1);  // Point vert: centre de la couleur
                cv::circle(frame, cam_center, 5, cv::Scalar(0,0,255), -1);    // Point rouge: centre caméra
                cv::line(frame, cam_center, color_center, cv::Scalar(255,0,0), 2);  // Ligne bleue entre les deux

                // Afficher les coordonnées et l'écart à l'écran
                std::string coord_text = "Position: (" + std::to_string(int(color_center.x)) + 
                                   ", " + std::to_string(int(color_center.y)) + ")";
                std::string offset_text = "Offset: dx=" + std::to_string(int(dx)) + 
                                    ", dy=" + std::to_string(int(dy));
                cv::putText(frame, "Lum: " + std::to_string(mean_v), cv::Point(15,80),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
                cv::putText(frame, coord_text, cv::Point(15,15), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
                cv::putText(frame, offset_text, cv::Point(15,50), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
                cv::putText(frame, "Luminosité: " + std::to_string(mean_v), cv::Point(15, 80),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

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
        }

        if (screensources == 0) {
            cv::Mat color_hsv(1, 1, CV_8UC3, last_color);
            cv::Mat color_bgr;
            cv::cvtColor(color_hsv, color_bgr, cv::COLOR_HSV2BGR);
            cv::Vec3b bgr = color_bgr.at<cv::Vec3b>(0, 0);
            cv::rectangle(frame, cv::Point(10, 10), cv::Point(60, 60),
            cv::Scalar(bgr[0], bgr[1], bgr[2]), cv::FILLED);
            cv::imshow("Webcam", frame);
        }
        else cv::imshow("Mask", mask);

        // waitKey(30) attend 30 millisecondes et vérifie si une touche est pressée
        //sert aussi de tempo pour la boucle !!!
        // Si 'q' est pressé,on sort de la boucle
        char key = cv::waitKey(5);
        if (key == 'q') {
            stop_signal = true; // Envoie le signal d'arrêt
            break;
        }
        else if (key == 'n'){
            screensources = (screensources + 1) % sources; // alterne entre 0 et 1
        }
        else if (key == 'p') pause = !pause; // Pause ou reprise
    }

    // libère la camera et ferme fenetres
    cap.release();
    cv::destroyAllWindows();
    

    return 0;
}