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
int gifnumber = 0;
int playingsound = 0;
extern int tol;
extern MasqueCouleur Mask1;
extern Position consigne;
cv::Mat srcrgb, src;

extern double gainK;
extern double correctorTimeConstant;
extern double correctorTimeConstantC; 
extern double correctorTimeConstantD;

int display() {
    sf::RenderWindow window(sf::VideoMode({720u, 1000u}), "Color Tracking - Interface");
    float basey = 480.0 + 20.0 + 20.0;
    float offsetybutton = 30;

    // sf::RectangleShape correctorSliderD(sf::Vector2f(200, 5));
    // correctorSliderD.setPosition({150, basey + 300});
    // correctorSliderD.setFillColor(sf::Color::White);
    // sf::CircleShape correctorKnobD(8);
    // correctorKnobD.setFillColor(sf::Color::Magenta);

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
  
    // sf::Text correctorTextD(font);
    // correctorTextD.setString("Corr Time D");
    // correctorTextD.setFont(font);
    // correctorTextD.setCharacterSize(16);
    // correctorTextD.setPosition({50.f, basey + 290.f});

    sf::Text startText(font);
    startText.setString("Start");
    startText.setFont(font);
    startText.setCharacterSize(16);
    startText.setPosition({75.f, basey + offsetybutton+5.5f});

    sf::Text stopText(font);
    stopText.setString("Stop");
    stopText.setFont(font);
    stopText.setCharacterSize(16);
    stopText.setPosition({230.f, basey + offsetybutton+5.5f});

    sf::Text quitText(font);
    quitText.setString("Quitter");
    quitText.setFont(font);
    quitText.setCharacterSize(16);
    quitText.setPosition({370.f, basey + offsetybutton+5.5f});

    sf::Text playText(font);
    playText.setString("Lancer");
    playText.setFont(font);
    playText.setCharacterSize(16);
    playText.setPosition({230.f, basey + offsetybutton+5.5f+60.0f});

    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("./extras/logo2.png")) {
        std::cerr << "Erreur chargement logo.png" << std::endl;
    }
    sf::Sprite logoSprite(logoTexture);
    logoSprite.setScale({640.f / logoTexture.getSize().x , 480.f / logoTexture.getSize().y/2});
    logoSprite.setPosition({20.f,240+ 20.f});

    sf::Texture labTexture;
    if (!labTexture.loadFromFile("./extras/labyrinth.png")) {
        std::cerr << "Erreur chargement labyrinthe.png" << std::endl;
        return -1;
    }
    sf::Sprite labSprite(labTexture);
    labSprite.setPosition({20.f, 20.f});

    // Ajouter cette déclaration au début de la fonction display() avec les autres variables
    sf::SoundBuffer buffer;
    std::unique_ptr<sf::Sound> sound;

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
                    jeu ? playText.setString("Jouer") : playText.setString("Pause") ; 
                //tirer un numero de 1 à 6
                gifnumber = rand() % 6 + 1; // tirage aléatoire entre 1 et 6
                std::cout << "Gif number: " << gifnumber << std::endl;
                }
                
                
                // if (mouse.y > basey + 295 && mouse.y < basey + 315 && mouse.x > 150 && mouse.x < 350) {
                //     correctorTimeConstantD = (mouse.x - 150) / 2000.0f * 5.0f;
                // }

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
                
                // if (mouse.y > basey+295 && mouse.y < basey+325 && mouse.x > 150 && mouse.x < 350) {
                //     correctorTimeConstantD = float(mouse.x - 150) / 2000.0f * 5.0f; // 0.0 à 2.0
                //     if (event->is<sf::Event::MouseButtonPressed>()) {
                //         break;
                //     }
                // }
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
                if(jeu){
                                // Puis superposer le labyrinthe avec transparence
                float scaleX = static_cast<float>(src.cols) / labTexture.getSize().x;
                float scaleY = static_cast<float>(src.rows) / labTexture.getSize().y;
                labSprite.setScale({scaleX, scaleY});
                labSprite.setColor(sf::Color(255, 255, 255, 255)); // 128 pour semi-transparent
                window.draw(labSprite);}
            }
        }
        else {
            window.draw(logoSprite);
        } 
        
        if(running && status==4){
            sf::Text endText(font);
            endText.setString("GAME OVER!!");
            endText.setFont(font);
            endText.setCharacterSize(24);
            endText.setFillColor(sf::Color::Red);
            endText.setPosition({50.f, basey + 100.f});
            window.draw(endText);
            //afficher le gif correspondant au numero
            std::string gifPath = "./extras/screamgif/00" + std::to_string(gifnumber) + ".gif";
            sf::Texture gifTexture;
            if (!gifTexture.loadFromFile(gifPath)) {
                std::cerr << "Erreur chargement " << gifPath << std::endl;
            } else {
                sf::Sprite gifSprite(gifTexture);
                gifSprite.setPosition({20.f, 20.f});
                float scaleX = static_cast<float>(640) / gifTexture.getSize().x;
                float scaleY = static_cast<float>(480) / gifTexture.getSize().y;
                gifSprite.setScale({scaleX, scaleY});
                window.draw(gifSprite);
                playingsound = true;
                jeu = false;
                
            }
        }
        
        


        // correctorKnobD.setPosition({static_cast<float>(150 + (correctorTimeConstantD / 5) * 2000 - 8), static_cast<float>(basey + 296)});

        if(running){
        startButton.setPosition({50, basey + offsetybutton});
        stopButton.setPosition({200, basey + offsetybutton});
        quitButton.setPosition({350, basey + offsetybutton});
        playButton.setPosition({50, basey + offsetybutton+60.f});
        startText.setPosition({75.f, basey + offsetybutton+5.5f});
        stopText.setPosition({230.f, basey + offsetybutton+5.5f});
        quitText.setPosition({370.f, basey + offsetybutton+5.5f});
        playText.setPosition({230.f, basey + offsetybutton+5.5f +60.0f});
        window.draw(playButton);
        window.draw(playText);

        // window.draw(correctorKnobD);
        // window.draw(correctorTextD);
    
        
        // oss.str(""); oss.clear();
        // oss << "CorrectorTimeConstantD: " << correctorTimeConstantD;
        // sf::Text correctorTimeConstantD(font);
        // correctorTimeConstantD.setString(oss.str());
        // correctorTimeConstantD.setFont(font);
        // correctorTimeConstantD.setCharacterSize(14);
        // correctorTimeConstantD.setPosition({370, basey + 290});
        // window.draw(correctorTimeConstantD);



    }
        else{
        startButton.setPosition({50, basey});
        stopButton.setPosition({200, basey});
        quitButton.setPosition({350, basey});
        startText.setPosition({75.f, basey + 5.f});
        stopText.setPosition({230.f, basey + 5.f});
        quitText.setPosition({370.f, basey + 5.f});
    } // on affiche les boutons + bas

        
        if(!jeu){window.draw(startButton);
        window.draw(startText);}

        window.draw(stopButton);
        window.draw(quitButton);
        window.draw(stopText);
        window.draw(quitText);
                


        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        window.display();
        if (playingsound){//joue le son extras/sound/1.mp3
                sf::SoundBuffer buffer;
                if (!buffer.loadFromFile("./extras/sounds/1.mp3")) {
                    std::cerr << "Erreur chargement son" << std::endl;
                } else {
                    sound = std::make_unique<sf::Sound>(buffer);
                sound->play();
                while (sound->getStatus() == sf::SoundSource::Status::Playing) {    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
                playingsound = false; // Réinitialiser le flag pour ne pas jouer le son à chaque frame}
    }}
    return 0;
}