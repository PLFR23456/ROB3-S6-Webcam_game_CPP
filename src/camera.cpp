#include "commande.hpp"
#include "camera.hpp"

#include <iostream> // Pour afficher des messages dans la console
#include <thread> // Pour la gestion des threads
#include <mutex> // Pour protéger les variables partagées

// Surface minimale pour filtrer le bruit
struct MasqueCouleur {
    cv::Scalar mini{140, 140, 140};  
    cv::Scalar maxi{190, 255, 255};  
    int minArea{50};                 
} Mask1;

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

void onMouseSimple(int event, int x, int y, int, void*) {
    if (event == cv::EVENT_LBUTTONDOWN && !frame_for_click.empty()) { 
        cv::Mat hsv;
        cv::cvtColor(frame_for_click, hsv, cv::COLOR_BGR2HSV);
        cv::Vec3b pix = hsv.at<cv::Vec3b>(y, x);
        Mask1.mini = cv::Scalar(
            std::max(0, pix[0] - tol),
            std::max(0, pix[1] - tol*2),
            std::max(0, pix[2] - tol*4)
        );
        Mask1.maxi = cv::Scalar(
            std::min(180, pix[0] + tol),
            std::min(255, pix[1] + tol*2),
            std::min(255, pix[2] + tol*5)
        );
        last_color = cv::Scalar(pix[0], pix[1], pix[2]); // Sauvegarde la couleur HSV sélectionnée        
        std::cout << "Nouvelle couleur HSV : " << (int)pix[0] << "," << (int)pix[1] << "," << (int)pix[2] << std::endl;
    }
}

void traiterCamera(cv::VideoCapture& cap, ProcessedFrame& data) {
    cv::Mat frame, hsv, mask;
    int cam_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Point2f cam_center(cam_width/2.0f, cam_height/2.0f);

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

        /**/
        // Calcul de la luminosité moyenne de l'image (canal V en HSV)
        cv::Scalar mean_hsv = cv::mean(hsv);
        int mean_v = static_cast<int>(mean_hsv[2]); // Valeur moyenne du canal V (luminosité)
        // Affichage de la luminosité moyenne
        
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
                    if(y > ymax) ymax = y;
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
            std::string coord_text = "Position: (" + std::to_string(int(color_center.x)) + ", " + std::to_string(int(color_center.y)) + ")";
            std::string offset_text = "Offset: dx=" + std::to_string(int(dx)) + ", dy=" + std::to_string(int(dy));
            std::string summederrortext = "Summed Error: (x=" + std::to_string(summedXError) + ", y=" + std::to_string(summedYError) + ")";
            std::string currenterrortext = "Current Error: (x=" + std::to_string(currentXError) + ", y=" + std::to_string(currentYError) + ")";
            std::string derrortext = "D Error: (x=" + std::to_string(dXError) + ", y=" + std::to_string(dYError) + ")";
            int baseLine = 0;
            cv::Scalar textColor = (count > Mask1.minArea) ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255);
            cv::Scalar bgColor(0, 0, 0); // fond noir

            std::vector<std::string> lines = {
                "Position : (" + std::to_string(int(color_center.x)) + ", " + std::to_string(int(color_center.y)) + ")",
                "Offset : dx = " + std::to_string(int(dx)) + ", dy = " + std::to_string(int(dy)),
                "Brightness : " + std::to_string(mean_v),
                "Summed Error : (x = " + std::to_string(summedXError) + ", y = " + std::to_string(summedYError) + ")",
                "Current Error : (x = " + std::to_string(currentXError) + ", y = " + std::to_string(currentYError) + ")",
                "D Error : (x = " + std::to_string(dXError) + ", y = " + std::to_string(dYError) + ")"
            };

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
            frame.copyTo(data.frame);
            mask.copyTo(data.mask);
            data.ready = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // pour ne pas surcharger
    }
}

int camera() {
    cv::VideoCapture cap(2);
    if (!cap.isOpened()) {
        std::cerr << "Erreur: Impossible d'ouvrir la webcam!" << std::endl;
        return -1;
    }

    // Initialisation de la fenêtre et des trackbars
    cv::namedWindow("Webcam");
    // Création des trackbars
    int gainK_slider = static_cast<int>(gainK * 100);
    cv::createTrackbar("Gain K", "Webcam", &gainK_slider, 200, onGainKChange);
    int correctorTimeConstant_slider = static_cast<int>(correctorTimeConstant * 1000);
    cv::createTrackbar("Corrector Time Constant", "Webcam", &correctorTimeConstant_slider, 2000, onCorrectorTimeConstantChange);
    int correctorTimeConstantC_slider = static_cast<int>(correctorTimeConstantC * 1000);
    cv::createTrackbar("Corrector Time Constant C", "Webcam", &correctorTimeConstantC_slider, 2000, onCorrectorTimeConstantCChange);
    int correctorTimeConstantD_slider = static_cast<int>(correctorTimeConstantD * 1000);
    cv::createTrackbar("Corrector Time Constant D", "Webcam", &correctorTimeConstantD_slider, 2000, onCorrectorTimeConstantDChange);
    int tol_slider = tol;
    cv::createTrackbar("Tolerance", "Webcam", &tol_slider, 100, onTolChange);
    
    // Création du callback pour la souris
    cv::setMouseCallback("Webcam", onMouseSimple, nullptr);

    std::thread worker(traiterCamera, std::ref(cap), std::ref(processed_data));

    while (!stop_signal) {
        cv::Mat to_show;
        {
            std::lock_guard<std::mutex> lock(processed_data.mutex);
            if (processed_data.ready) {
                if (screensources == 0)
                    to_show = processed_data.frame.clone();
                else
                    to_show = processed_data.mask.clone();
            }
        }

        if (!to_show.empty())
            cv::imshow("Webcam", to_show);

        char key = cv::waitKey(10);
        if (key == 'q') {
            stop_signal = true;
            break;
        } else if (key == 'n') {
            screensources = (screensources + 1) % sources;
        }
    }

    worker.join();
    return 0;
}
