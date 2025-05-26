#include "commande.hpp"
#include "camera.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/Config.hpp>
#include <SFML/Graphics/RenderWindow.hpp>


#include <string>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>

bool running = false;
bool jeu = false;
int status = 0;
extern int tol;
extern MasqueCouleur Mask1;
cv::Mat srcrgb, src;

extern double gainK;
extern double correctorTimeConstant;
extern double correctorTimeConstantC; 
extern double correctorTimeConstantD;

int display() {
    sf::RenderWindow window(sf::VideoMode({720u, 1000u}), "Color Tracking - Interface");
    float basey = 480.0 + 20.0 + 20.0;

    sf::RectangleShape tolSlider(sf::Vector2f(200, 5));
    tolSlider.setPosition({150, basey + 120});
    tolSlider.setFillColor(sf::Color::White);
    sf::CircleShape tolKnob(8);
    tolKnob.setFillColor(sf::Color::Blue);

    sf::RectangleShape gainSlider(sf::Vector2f(200, 5));
    gainSlider.setPosition({150, basey + 60});
    gainSlider.setFillColor(sf::Color::White);
    sf::CircleShape gainKnob(8);
    gainKnob.setFillColor(sf::Color::Red);

    sf::RectangleShape correctorSliderA(sf::Vector2f(200, 5));
    correctorSliderA.setPosition({150, basey + 180});
    correctorSliderA.setFillColor(sf::Color::White);
    sf::CircleShape correctorKnobA(8);
    correctorKnobA.setFillColor(sf::Color::Yellow);

    sf::RectangleShape correctorSliderC(sf::Vector2f(200, 5));
    correctorSliderC.setPosition({150, basey + 240});
    correctorSliderC.setFillColor(sf::Color::White);
    sf::CircleShape correctorKnobC(8);
    correctorKnobC.setFillColor(sf::Color::Cyan);

    sf::RectangleShape correctorSliderD(sf::Vector2f(200, 5));
    correctorSliderD.setPosition({150, basey + 300});
    correctorSliderD.setFillColor(sf::Color::White);
    sf::CircleShape correctorKnobD(8);
    correctorKnobD.setFillColor(sf::Color::Magenta);

    sf::RectangleShape startButton(sf::Vector2f(100, 30));
    startButton.setPosition({50, basey + 350});
    startButton.setFillColor(sf::Color(100, 200, 100));

    sf::RectangleShape stopButton(sf::Vector2f(100, 30));
    stopButton.setPosition({200, basey + 350});
    stopButton.setFillColor(sf::Color(200, 100, 100));

    sf::RectangleShape quitButton(sf::Vector2f(100, 30));
    quitButton.setPosition({350, basey + 350});
    quitButton.setFillColor(sf::Color(100, 100, 200));

    sf::RectangleShape playButton(sf::Vector2f(400, 30));
    playButton.setPosition({50, basey + 410});
    playButton.setFillColor(sf::Color(218, 165, 32));

    sf::Font font;
    font.openFromFile("./extras/DejaVuSans.ttf");


    // Remplacer la section des textes par :
    sf::Text gainText(font);
    gainText.setString("Gain K");
    gainText.setFont(font);
    gainText.setCharacterSize(16);
    gainText.setPosition({50.f, basey + 50.f});

    sf::Text tolText(font);
    tolText.setString("Tolerance");
    tolText.setFont(font);
    tolText.setCharacterSize(16);
    tolText.setPosition({50.f, basey + 110.f});

    sf::Text correctorTextA(font);
    correctorTextA.setString("Corr Time A");
    correctorTextA.setFont(font);
    correctorTextA.setCharacterSize(16);
    correctorTextA.setPosition({50.f, basey + 170.f});

    sf::Text correctorTextC(font);
    correctorTextC.setString("Corr Time C");
    correctorTextC.setFont(font);
    correctorTextC.setCharacterSize(16);
    correctorTextC.setPosition({50.f, basey + 230.f});

    sf::Text correctorTextD(font);
    correctorTextD.setString("Corr Time D");
    correctorTextD.setFont(font);
    correctorTextD.setCharacterSize(16);
    correctorTextD.setPosition({50.f, basey + 290.f});

    sf::Text startText(font);
    startText.setString("Start");
    startText.setFont(font);
    startText.setCharacterSize(16);
    startText.setPosition({75.f, basey + 355.f});

    sf::Text stopText(font);
    stopText.setString("Stop");
    stopText.setFont(font);
    stopText.setCharacterSize(16);
    stopText.setPosition({230.f, basey + 355.f});

    sf::Text quitText(font);
    quitText.setString("Quitter");
    quitText.setFont(font);
    quitText.setCharacterSize(16);
    quitText.setPosition({370.f, basey + 355.f});

    sf::Text playText(font);
    playText.setString("Lancer");
    playText.setFont(font);
    playText.setCharacterSize(16);
    playText.setPosition({230.f, basey + 415.f});

    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("./extras/logo2.png")) {
        std::cerr << "Erreur chargement logo.png" << std::endl;
    }
    sf::Sprite logoSprite(logoTexture);
    logoSprite.setScale({480.f / logoTexture.getSize().x , 480.f / logoTexture.getSize().y});
    logoSprite.setPosition({20.f, 20.f});

    sf::Texture labTexture;
    if (!labTexture.loadFromFile("./extras/labyrinth.png")) {
        std::cerr << "Erreur chargement labyrinthe.png" << std::endl;
        return -1;
    }
    sf::Sprite labSprite(labTexture);
    labSprite.setPosition({20.f, 20.f});

    while (window.isOpen()) {               // Déclare la variable event avant la boucle
            while (const std::optional event = window.pollEvent()) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouse = window.mapPixelToCoords(pixelPos);

            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                if (startButton.getGlobalBounds().contains(mouse)) running = true;
                if (stopButton.getGlobalBounds().contains(mouse)) running = false;
                if (quitButton.getGlobalBounds().contains(mouse)) { window.close(); stop_signal = true; }
                if (playButton.getGlobalBounds().contains(mouse)) {jeu = !jeu; 
                    jeu ? playText.setString("Jouer") : playText.setString("Pause") ; }
                if (mouse.y > basey + 175 && mouse.y < basey + 195 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstant = (mouse.x - 150) / 2000.0f * 5.0f;
                }
                if (mouse.y > basey + 235 && mouse.y < basey + 255 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstantC = (mouse.x - 150) / 2000.0f * 5.0f;
                }
                if (mouse.y > basey + 295 && mouse.y < basey + 315 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstantD = (mouse.x - 150) / 2000.0f * 5.0f;
                }

                if (running) {
                    std::lock_guard<std::mutex> lock(processed_data.mutex);
                    if (processed_data.ready && !processed_data.frame.empty()) {
                        srcrgb = processed_data.frame.clone();
                        int localX = mouse.x - 20;
                        int localY = mouse.y - 20;
                        if (localX >= 0 && localX < srcrgb.cols && localY >= 0 && localY < srcrgb.rows) {
                            cv::Vec3b color = srcrgb.at<cv::Vec3b>(localY, localX);
                            cv::Mat bgr(1, 1, CV_8UC3, color);
                            cv::Mat hsv;
                            cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
                            cv::Vec3b hsvPixel = hsv.at<cv::Vec3b>(0, 0);
                            int H = hsvPixel[0];
                            int S = hsvPixel[1];
                            int V = hsvPixel[2];
                            Mask1.mini = cv::Scalar(std::max(0, H - tol), std::max(0, S - tol * 2), std::max(0, V - tol * 4));
                            Mask1.maxi = cv::Scalar(std::min(180, H + tol), std::min(255, S + tol * 2), std::min(255, V + tol * 5));
                            std::cout << "Nouvelle couleur HSV : " << H << "," << S << "," << V << std::endl;
                        }
                    }   
                }
            }

            if (event->is<sf::Event::MouseButtonPressed>() || event->is<sf::Event::MouseMoved>()) {
                if (mouse.y > basey + 55 && mouse.y < basey + 75 && mouse.x > 150 && mouse.x < 350) {
                    gainK = (mouse.x - 150) / 200.0f * 2.0f;
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        break;
                    }
                }
                if (mouse.y > basey + 115 && mouse.y < basey + 135 && mouse.x > 150 && mouse.x < 350) {
                    tol = (mouse.x - 150) / 2;
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        break;
                    }
                }
                if (mouse.y > basey+175 && mouse.y < basey+195 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstant = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        break;
                    }
                }
                if (mouse.y > basey+235 && mouse.y < basey+255 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstantC = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        break;
                    }
                }
                if (mouse.y > basey+295 && mouse.y < basey+325 && mouse.x > 150 && mouse.x < 350) {
                    correctorTimeConstantD = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        break;
                    }
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        if (running) {
            std::lock_guard<std::mutex> lock(processed_data.mutex);
            if (processed_data.ready && !processed_data.frame.empty()) {
                cv::Mat srcrgb = processed_data.frame;
                cv::Mat src;
                cv::cvtColor(srcrgb, src, cv::COLOR_BGR2RGBA);
                sf::Image image({static_cast<unsigned int>(src.cols), static_cast<unsigned int>(src.rows)},reinterpret_cast<const std::uint8_t*>(src.ptr()));
                sf::Texture texture;
                texture.loadFromImage(image);
                sf::Sprite sprite(texture);
                sprite.setPosition({20.f, 20.f});
                window.draw(sprite);
                                // Puis superposer le labyrinthe avec transparence
                float scaleX = static_cast<float>(src.cols) / labTexture.getSize().x;
                float scaleY = static_cast<float>(src.rows) / labTexture.getSize().y;
                labSprite.setScale({scaleX, scaleY});
                labSprite.setColor(sf::Color(255, 255, 255, 255)); // 128 pour semi-transparent
                window.draw(labSprite);
            }
        } else {
            window.draw(logoSprite);
        }

        tolKnob.setPosition({static_cast<float>(150 + tol * 2 - 8), static_cast<float>(basey + 116)});
        gainKnob.setPosition({static_cast<float>(150 + gainK / 2.0f * 200 - 8), static_cast<float>(basey + 56)});
        correctorKnobA.setPosition({static_cast<float>(150 + (correctorTimeConstant / 5) * 2000 - 8), static_cast<float>(basey + 176)});
        correctorKnobC.setPosition({static_cast<float>(150 + (correctorTimeConstantC / 5) * 2000 - 8),static_cast<float>(basey + 236)});
        correctorKnobD.setPosition({static_cast<float>(150 + (correctorTimeConstantD / 5) * 2000 - 8), static_cast<float>(basey + 296)});

        if(running){
        startButton.setPosition({50, basey + 350});
        stopButton.setPosition({200, basey + 350});
        quitButton.setPosition({350, basey + 350});
        playButton.setPosition({50, basey + 410});
        startText.setPosition({75.f, basey + 355.f});
        stopText.setPosition({230.f, basey + 355.f});
        quitText.setPosition({370.f, basey + 355.f});
        playText.setPosition({230.f, basey + 415.f});

        window.draw(gainSlider);
        window.draw(gainKnob);
        window.draw(tolSlider);
        window.draw(tolKnob);
        window.draw(gainText);
        window.draw(tolText);
        window.draw(correctorSliderA);
        window.draw(correctorKnobA);
        window.draw(correctorTextA);
        window.draw(correctorSliderC);
        window.draw(correctorKnobC);
        window.draw(correctorTextC);
        window.draw(correctorSliderD);
        window.draw(correctorKnobD);
        window.draw(correctorTextD);
    
        std::ostringstream oss;
        oss << "Gain K: " << gainK;
        sf::Text gainVal(font);
        gainVal.setString(oss.str());
        gainVal.setCharacterSize(14);
        gainVal.setPosition({370, basey + 50});
        window.draw(gainVal);

        oss.str(""); oss.clear();
        oss << "Tol: " << tol;
        sf::Text tolVal(font);
        tolVal.setString(oss.str());
        tolVal.setCharacterSize(14);
        tolVal.setPosition({370, basey + 110});
        window.draw(tolVal);

        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstant: " << correctorTimeConstant;
        sf::Text correctorTimeConstant(font);
        correctorTimeConstant.setString(oss.str());
        correctorTimeConstant.setFont(font);
        correctorTimeConstant.setCharacterSize(14);
        correctorTimeConstant.setPosition({370, basey + 170});
        window.draw(correctorTimeConstant);

        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstantC: " << correctorTimeConstantC;
        sf::Text correctorTimeConstantC(font);
        correctorTimeConstantC.setString(oss.str());
        correctorTimeConstantC.setFont(font);
        correctorTimeConstantC.setCharacterSize(14);
        correctorTimeConstantC.setPosition({370, basey + 230});
        window.draw(correctorTimeConstantC);

        oss.str(""); oss.clear();
        oss << "CorrectorTimeConstantD: " << correctorTimeConstantD;
        sf::Text correctorTimeConstantD(font);
        correctorTimeConstantD.setString(oss.str());
        correctorTimeConstantD.setFont(font);
        correctorTimeConstantD.setCharacterSize(14);
        correctorTimeConstantD.setPosition({370, basey + 290});
        window.draw(correctorTimeConstantD);


        window.draw(playText);
        window.draw(playButton);
    }
        else{
        startButton.setPosition({50, basey});
        stopButton.setPosition({200, basey});
        quitButton.setPosition({350, basey});
        startText.setPosition({75.f, basey + 5.f});
        stopText.setPosition({230.f, basey + 5.f});
        quitText.setPosition({370.f, basey + 5.f});
    } // on affiche les boutons + bas

        
        window.draw(startButton);
        window.draw(stopButton);
        window.draw(quitButton);
        window.draw(startText);
        window.draw(stopText);
        window.draw(quitText);
        

        

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        window.display();
    }
    return 0;
}