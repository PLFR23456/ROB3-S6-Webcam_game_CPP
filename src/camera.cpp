#include <opencv2/opencv.hpp>   //  OpenCV pour la vision par ordinateur
#include <iostream>            //  pour afficher des messages dans la console
#include <mutex>              //  pour la synchronisation entre threads
#include <thread>

Position consigne= {0, 0};
std::mutex consigne_mutex; 
/*
int mouse_x = 0;
int mouse_y = 0;
void onMouse(int event, int x, int y, int flags, void* userdata){
        if (event == cv::EVENT_MOUSEMOVE){
            mouse_x = x;
            mouse_y = y;
        }
    }
*/              //Controle de la souris

struct MasqueCouleur {
    cv::Scalar mini{160, 150, 150};  // 
    cv::Scalar maxi{180, 255, 255};  // 
    int minArea{50};                 // Surface minimale pour filtrer le bruit
} redColor;

int sources = 2 ;
int screensources = 0; //0 = camera ; 1 = masque


int camera() {
    
    cv::VideoCapture cap(2);
    cv::namedWindow("Webcam");
    cv::namedWindow("Mask");
    //cv::setMouseCallback("Webcam", onMouse, nullptr); recuperer mouvement souris
    std::cout << "Appuyez sur 'q' pour quitter" << std::endl;

    
    if (!cap.isOpened()) { // isOpened() renvoie 'true' si la caméra fonctionne, 'false' sinon
        std::cerr << "Erreur: Impossible d'ouvrir la webcam!" << std::endl;
        return -1;
    }
    
    // cv::Mat est une classe qui stocke les images dans OpenCV
    // 'frame' va contenir chaque image capturée par la caméra
    cv::Mat frame, hsv, mask;
    
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);
    

    // Boucle infinie qui continue jusqu'à ce qu'on appuie sur 'q'
    while (true) {
        // '>>' est un opérateur qui capture une nouvelle image de la caméra (cap)
        // et la stocke dans notre variable 'frame'
        // C'est comme dire "prends une photo et mets-la dans frame"
        cap >> frame;
        
        // Vérifier si l'image capturée est vide
        // empty() renvoie 'true' si aucune image n'a été capturée
        if (frame.empty()) {
            std::cerr << "Erreur: Image vide!" << std::endl;
            break;  // Sortir de la boucle while si l'image est vide
        }


        // Convertir en HSV pour une meilleure détection des couleurs
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        
        // Créer le masque pour la couleur
        cv::inRange(hsv, redColor.mini, redColor.maxi, mask);
        
        
        //cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3)));
        //cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3)));
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
                    if(x < xmin) xmin = x;
                    if(x > xmax) xmax = x;
                    if(y < ymin) ymin = y;
                    if(y > ymax) ymax = y;
                }
            }
        }
        // Si on a trouvé assez de pixels de la couleur
        if(count > redColor.minArea) {
            // Calculer le point central
            cv::Point2f color_center(sumX/float(count), sumY/float(count));
            
            // Calculer l'écart avec le centre de la caméra
            float dx = color_center.x - cam_center.x;
            float dy = color_center.y - cam_center.y;

            // Dessiner des repères visuels
            cv::rectangle(frame, cv::Point(xmin, ymin), cv::Point(xmax, ymax), cv::Scalar(0,0,255), 2, 0); // Rectangle rouge (BGR format)
            cv::circle(frame, color_center, 5, cv::Scalar(0,255,0), -1);  // Point vert: centre de la couleur
            cv::circle(frame, cam_center, 5, cv::Scalar(0,0,255), -1);    // Point rouge: centre caméra
            cv::line(frame, cam_center, color_center, cv::Scalar(255,0,0), 2);  // Ligne bleue entre les deux

            // Afficher les coordonnées et l'écart
            std::string coord_text = "Position: (" + std::to_string(int(color_center.x)) + 
                                   ", " + std::to_string(int(color_center.y)) + ")";
            std::string offset_text = "Offset: dx=" + std::to_string(int(dx)) + 
                                    ", dy=" + std::to_string(int(dy));
            
            cv::putText(frame, coord_text, cv::Point(10,30), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
            cv::putText(frame, offset_text, cv::Point(10,60), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,0,255), 2);
            
            
            
            // met a jour la variable global
            std::lock_guard<std::mutex> lock(consigne_mutex);
            consigne.x = color_center.x;
            consigne.y = color_center.y;
            }

        


        // Afficher les images
        if(screensources == 0) {
            cv::imshow("Webcam", frame);
        } else {
            cv::imshow("Mask", mask);
        }   

        // waitKey(30) attend 30 millisecondes et vérifie si une touche est pressée
        // Si 'q' est pressé,on sort de la boucle
        char key = cv::waitKey(30);
        if (key == 'q') {
            break;
        }
        else if (key == 'n'){
            screensources = screensources+1; // alterne entre 0 et 1
            screensources = screensources % sources; // alterne entre 0 et 1
        }
    }

    // libère la camera et ferme fenetres
    cap.release();
    cv::destroyAllWindows();
    

    return 0;
}