#include <opencv2/opencv.hpp>
#include <iostream>

int camera() {
    cv::VideoCapture cap(0); // 0 = caméra par défaut

    if (!cap.isOpened()) {
        std::cerr << "Erreur : impossible d'ouvrir la caméra." << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame; // Capture une image

        if (frame.empty()) {
            std::cerr << "Erreur : image vide capturée." << std::endl;
            break;
        }

        cv::imshow("Flux vidéo", frame);

        if (cv::waitKey(1) == 27) { // Touche 'ESC' pour quitter
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
