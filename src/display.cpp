#include "commande.hpp" // Inclure le fichier d'en-tête commande.hpp pour les variables globales
#include "camera.hpp"  // Inclure le fichier d'en-tête camera.hpp pour les variables globales
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <iostream> // Pour afficher des messages dans la console
#include <thread> // Pour la gestion des threads
#include <mutex> // Pour protéger les variables partagées


// Simule les variables globales (à relier à ton code réel)
bool running = false;
extern double gainK;
extern int tol;
extern MasqueCouleur Mask1;
cv::Mat srcrgb, src;
extern double correctorTimeConstant;
extern double correctorTimeConstantC; 
extern double correctorTimeConstantD;

int display() {
    sf::RenderWindow window(sf::VideoMode(720, 1000), "Color Tracking - Interface");
    int basey= 480+20+20; // Position de base pour les boutons
    // Sliders
    sf::RectangleShape gainSlider(sf::Vector2f(200, 5));
    gainSlider.setPosition(150, basey+60);
    gainSlider.setFillColor(sf::Color::White);

    sf::CircleShape gainKnob(8);
    gainKnob.setFillColor(sf::Color::Red);

    sf::RectangleShape tolSlider(sf::Vector2f(200, 5));
    tolSlider.setPosition(150, basey+120);
    tolSlider.setFillColor(sf::Color::White);

    sf::CircleShape tolKnob(8);

    tolKnob.setFillColor(sf::Color::Blue);

    // Sliders pour correctorTimeConstant, correctorTimeConstantC et D
    sf::RectangleShape correctorSliderA(sf::Vector2f(200, 5));
    correctorSliderA.setPosition(150, basey + 180);
    correctorSliderA.setFillColor(sf::Color::White);

    sf::CircleShape correctorKnobA(8);
    correctorKnobA.setFillColor(sf::Color::Yellow);

    sf::RectangleShape correctorSliderC(sf::Vector2f(200, 5));
    correctorSliderC.setPosition(150, basey + 240);
    correctorSliderC.setFillColor(sf::Color::White);

    sf::CircleShape correctorKnobC(8);
    correctorKnobC.setFillColor(sf::Color::Cyan);

    sf::RectangleShape correctorSliderD(sf::Vector2f(200, 5));
    correctorSliderD.setPosition(150, basey + 300);
    correctorSliderD.setFillColor(sf::Color::White);

    sf::CircleShape correctorKnobD(8);
    correctorKnobD.setFillColor(sf::Color::Magenta);


    // Boutons
    sf::RectangleShape startButton(sf::Vector2f(100, 30));
    startButton.setPosition(50, basey+350);
    startButton.setFillColor(sf::Color(100, 200, 100));

    sf::RectangleShape stopButton(sf::Vector2f(100, 30));
    stopButton.setPosition(200, basey+350);
    stopButton.setFillColor(sf::Color(200, 100, 100));

    sf::RectangleShape quitButton(sf::Vector2f(100, 30));
    quitButton.setPosition(350, basey+350);
    quitButton.setFillColor(sf::Color(100, 100, 200));

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    sf::Text gainText("Gain K", font, 16);
    gainText.setPosition(50, basey+50);
    sf::Text tolText("Tolerance", font, 16);
    tolText.setPosition(50, basey+110);
    sf::Text correctorTextA("Corr Time A", font, 16);
    correctorTextA.setPosition(50, basey + 170);
    sf::Text correctorTextC("Corr Time C", font, 16);
    correctorTextC.setPosition(50, basey + 230);
    sf::Text correctorTextD("Corr Time D", font, 16);
    correctorTextD.setPosition(50, basey + 290);
    sf::Text startText("Start", font, 16);
    startText.setPosition(75, basey+355);
    sf::Text stopText("Stop", font, 16);
    stopText.setPosition(230, basey+355);
    sf::Text quitText("Quitter", font, 16);
    quitText.setPosition(370, basey+355);
    // Flux vidéo

    // Charger le logo
    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("./extras/logo.png")) {
        std::cerr << "Erreur chargement logo.png" << std::endl;
    }
    sf::Sprite logoSprite(logoTexture);
    logoSprite.setScale((float) 480/1024, (float) 480/1024); // Redimensionner le logo
    logoSprite.setPosition(20, 20);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);
                if (startButton.getGlobalBounds().contains(mouse.x, mouse.y)) running = true;
                if (stopButton.getGlobalBounds().contains(mouse.x, mouse.y)) running = false;
                if (quitButton.getGlobalBounds().contains(mouse.x, mouse.y)){ window.close();stop_signal = true;}
                // Slider correctorTimeConstant A
                if (mouse.y > basey+175 && mouse.y < basey+195 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstant = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 5.0
                }
                // Slider correctorTimeConstant C
                if (mouse.y > basey+235 && mouse.y < basey+255 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstantC = float(mouse.x - 150) / 2000.0f * 5.0f;
                }
                // Slider correctorTimeConstant D
                if (mouse.y > basey+295 && mouse.y < basey+315 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstantD = float(mouse.x - 150) / 2000.0f * 5.0f;
                }

                if (running) {
                std::lock_guard<std::mutex> lock(processed_data.mutex);
                if (processed_data.ready && !processed_data.frame.empty()) {
                    srcrgb = processed_data.frame.clone(); // on garde une copie

                    int imgWidth = srcrgb.cols;
                    int imgHeight = srcrgb.rows;

                    auto mouse = sf::Mouse::getPosition(window);

                    // Zone d'affichage vidéo positionnée à (20, 20)
                    int localX = mouse.x - 20;
                    int localY = mouse.y - 20;

                    if (localX >= 0 && localX < imgWidth && localY >= 0 && localY < imgHeight) {
                        cv::Vec3b color = srcrgb.at<cv::Vec3b>(localY, localX); // y puis x
                        cv::Vec3b bgrPixel = srcrgb.at<cv::Vec3b>(localY, localX);
                        cv::Mat bgr(1, 1, CV_8UC3, bgrPixel); // image 1x1 en BGR

                        cv::Mat hsv;
                        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

                        cv::Vec3b hsvPixel = hsv.at<cv::Vec3b>(0, 0);
                        int H = hsvPixel[0];
                        int S = hsvPixel[1];
                        int V = hsvPixel[2];
                        Mask1.mini = cv::Scalar(
                            std::max(0, H - tol),
                            std::max(0, S - tol*2),
                            std::max(0, V - tol*4)
                        );
                        Mask1.maxi = cv::Scalar(
                            std::min(180, H + tol),
                            std::min(255, S + tol*2),
                            std::min(255, V + tol*5)
                        );
                        cv::Scalar last_color = cv::Scalar(H,S,V); // Sauvegarde la couleur HSV sélectionnée        
                        std::cout << "Nouvelle couleur HSV : " << (int)H << "," << (int)S << "," << (int)V << std::endl;
                    }
                }
}
            }
            if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseMoved) {
                auto mouse = sf::Mouse::getPosition(window);
                // Gain slider
                if (mouse.y > basey+55 && mouse.y < basey+75 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    gainK = float(mouse.x - 150) / 200.0f * 2.0f; // 0.0 à 2.0
                }
                // Tol slider
                if (mouse.y > basey+115 && mouse.y < basey+135 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    tol = (mouse.x - 150) / 2; // 0 à 100
                }
                if (mouse.y > basey+175 && mouse.y < basey+195 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstant = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                }
                if (mouse.y > basey+235 && mouse.y < basey+255 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstantC = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                }
                if (mouse.y > basey+295 && mouse.y < basey+325 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    correctorTimeConstantD = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        if (running) {
            std::lock_guard<std::mutex> lock(processed_data.mutex);
            if (processed_data.ready && !processed_data.frame.empty()) {
                cv::Mat srcrgb = processed_data.frame;
                cv::Mat src;
                cv::cvtColor(srcrgb, src, cv::COLOR_BGR2RGBA); //BGR2RGBA

                sf::Image image;
                image.create(src.cols, src.rows, src.ptr());
                sf::Texture texture;
                texture.loadFromImage(image);
                sf::Sprite sprite(texture);// Bloc vidéo à (20,20)
                sprite.setPosition(20, 20);
                window.draw(sprite);
            }
        } else {
            // Affiche le logo tant que running == false
            window.draw(logoSprite);
        }

        // Position des knobs
        // Position des knobs
        gainKnob.setPosition(150 + gainK/2.0f*200 - 8, basey+56);
        tolKnob.setPosition(150 + tol*2 - 8, basey+116);

        correctorKnobA.setPosition(150 + (correctorTimeConstant / 5.0f) * 2000 - 8, basey + 176);
        correctorKnobC.setPosition(150 + (correctorTimeConstantC / 5.0f) * 2000 - 8, basey + 236);
        correctorKnobD.setPosition(150 + (correctorTimeConstantD / 5.0f) * 2000 - 8, basey + 296);


        window.draw(gainSlider);
        window.draw(gainKnob);
        window.draw(tolSlider);
        window.draw(tolKnob);
        window.draw(startButton);
        window.draw(stopButton);
        window.draw(quitButton);
        window.draw(gainText);
        window.draw(tolText);
        window.draw(startText);
        window.draw(stopText);
        window.draw(quitText);
        window.draw(correctorSliderA);
        window.draw(correctorKnobA);
        window.draw(correctorTextA);
        window.draw(correctorSliderC);
        window.draw(correctorKnobC);
        window.draw(correctorTextC);
        window.draw(correctorSliderD);
        window.draw(correctorKnobD);
        window.draw(correctorTextD);


        // Affiche valeurs
        std::ostringstream oss;
        oss << "Gain K: " << gainK;
        sf::Text gainVal(oss.str(), font, 14);
        gainVal.setPosition(370, basey+50);
        window.draw(gainVal);

        oss.str(""); oss.clear();
        oss << "Tol: " << tol;
        sf::Text tolVal(oss.str(), font, 14);
        tolVal.setPosition(370, basey+110);
        window.draw(tolVal);
        
        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstant: " << correctorTimeConstant;
        sf::Text tocorrectorTimeConstant(oss.str(), font, 14);
        tocorrectorTimeConstant.setPosition(370, basey+170);
        window.draw(tocorrectorTimeConstant);

        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstantC: " << correctorTimeConstantC;
        sf::Text tocorrectorTimeConstantC(oss.str(), font, 14);
        tocorrectorTimeConstantC.setPosition(370, basey+230);
        window.draw(tocorrectorTimeConstantC);

        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstantD: " << correctorTimeConstantD;
        sf::Text tocorrectorTimeConstantD(oss.str(), font, 14);
        tocorrectorTimeConstantD.setPosition(370, basey+290);
        window.draw(tocorrectorTimeConstantD);



        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        window.display();
    }
    return 0;
}