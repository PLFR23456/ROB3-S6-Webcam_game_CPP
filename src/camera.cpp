// Inclure les bibliothèques nécessaires
#include <opencv2/opencv.hpp>   // Bibliothèque OpenCV pour la vision par ordinateur
#include <iostream>            // Bibliothèque pour afficher des messages dans la console

int mouse_x = 0;
int mouse_y = 0;
void onMouse(int event, int x, int y, int flags, void* userdata){
        if (event == cv::EVENT_MOUSEMOVE){
            mouse_x = x;
            mouse_y = y;
        }
    }


int camera() {
    // cv::VideoCapture est une classe qui permet de capturer la vidéo
    // Le '0' signifie qu'on utilise la première caméra trouvée (généralement la webcam intégrée)
    // 'cap' est le nom qu'on donne à notre objet qui va gérer la caméra
    cv::VideoCapture cap(0);
    cv::namedWindow("Webcam");
    cv::setMouseCallback("Webcam", onMouse, nullptr);

    
    // Vérifier si la caméra a bien été ouverte
    // isOpened() renvoie 'true' si la caméra fonctionne, 'false' sinon
    if (!cap.isOpened()) {
        // std::cerr est utilisé pour afficher des messages d'erreur
        std::cerr << "Erreur: Impossible d'ouvrir la webcam!" << std::endl;
        return -1;  // Quitter le programme avec un code d'erreur
    }

    // Afficher un message d'instruction à l'utilisateur
    std::cout << "Appuyez sur 'q' pour quitter" << std::endl;

    // cv::Mat est une classe qui stocke les images dans OpenCV
    // 'frame' va contenir chaque image capturée par la caméra
    cv::Mat frame;

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

        // Afficher l'image dans une fenêtre nommée "Webcam"
        // imshow crée ou utilise une fenêtre et y affiche notre image
        cv::Point pt;
        pt.x = mouse_x;
        pt.y = mouse_y;
        cv::ellipse(frame,pt,
        cv::Size(100, 50),5.0,350.0, 10.0,
        255, 10, 0, 0);
        cv::imshow("Webcam", frame);
        std::string coord_text = "Position: (" + std::to_string(mouse_x) + ", " + std::to_string(mouse_y) + ")";
        std::cout << coord_text << std::endl;

        // waitKey(30) attend 30 millisecondes et vérifie si une touche est pressée
        // Si la touche 'q' est pressée, on sort de la boucle
        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    // Nettoyage : on libère la caméra pour que d'autres programmes puissent l'utiliser
    cap.release();
    // Fermer toutes les fenêtres créées par OpenCV
    cv::destroyAllWindows();
    
    // Fin du programme
    return 0;
}