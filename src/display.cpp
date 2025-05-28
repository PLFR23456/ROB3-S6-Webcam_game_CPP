#include "display.hpp"
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
#include <ctime>

//GLOBAL INDICATORS
int labnumber = (gamemode-1)/2 + 1;
bool isCameraShaking = false;

int score = 0;
int gamemode = 1;
int soundnumber = 0;
int gifnumber = 0;
int playingsound = 0;
int bouclestart = 0;

//SHARED DATA
extern Position startbox; // Déclaré dans main.cpp
extern Position endbox; // Déclaré dans main.cpp

extern int tol; // Déclarée dans camera.cpp
extern MasqueCouleur Mask1; // Déclarée dans camera.cpp
cv::Mat srcrgb, src;

void table(int gamemode, Position& startpos, Position& endbox){
    int res = (gamemode-1)/2;
    switch (res) {
        case 0: // Mode 0
            startpos = {300, 80}; // Position de la startbox
            endbox = {200, 400}; // Position de la endbox
            break;
        case 1: // Mode 1
            startpos = {35, 35}; // Position centrale par défaut
            endbox = {605, 445}; // Position centrale par défaut
            break;
        case 2: // Mode 2
            startpos = {140, 35}; // Position de la startbox
            endbox = {500, 445}; // Position de la endbox
            break;
        case 3: // Mode 3
            startpos = {510, 40}; // Position de la startbox
            endbox = {640-35, 480-35}; // Position de la endbox
            break;
        default: // Mode par défaut
            startpos = {100, 100}; // Position centrale par défaut
            endbox = {0, 0}; // Position centrale par défaut
            break;
    }
    return;
}


int display(GameSession& game) {
    srand(time(nullptr));
    sf::RenderWindow window(sf::VideoMode({720u, 1000u}), "Color Tracking - Interface");
    float basey = 480.0 + 20.0 + 20.0;
    float offsetybutton = 30;
    sf::Font font;
    sf::SoundBuffer buffer;
    std::unique_ptr<sf::Sound> sound;
    if (!font.openFromFile("./extras/DejaVuSans.ttf")) {
        std::cerr << "Error loading font file" << std::endl;
        return -1;
    }

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

    sf::RectangleShape colorBox(sf::Vector2f(150, 30)); 
    colorBox.setPosition({350 + 100 + 30, basey + offsetybutton}); 
    colorBox.setFillColor(sf::Color(64, 64, 64)); 

    sf::RectangleShape scoreBox(sf::Vector2f(150, 30)); 
    scoreBox.setPosition({350 + 100 + 30, basey + offsetybutton+60.f});
    scoreBox.setFillColor(sf::Color(64, 64, 64));

    sf::RectangleShape gamemodeBox(sf::Vector2f(400, 30));
    gamemodeBox.setPosition({50, basey + offsetybutton+120.f});
    gamemodeBox.setFillColor(sf::Color(109, 7, 26)); 

    sf::RectangleShape CreditsBox(sf::Vector2f(150, 30));
    CreditsBox.setPosition({350 + 100 + 30, basey + offsetybutton+120.f});
    CreditsBox.setFillColor(sf::Color(64, 64, 64));


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
    playText.setPosition({220.f, basey + offsetybutton+5.5f+60.0f});

    sf::Text colorText(font);
    colorText.setString("Color tracked :");
    colorText.setFont(font);
    colorText.setCharacterSize(16);
    colorText.setPosition({colorBox.getPosition().x + 10.f, colorBox.getPosition().y - 20.f});

    sf::Text scoreText(font);
    scoreText.setString("Score: " + std::to_string(score));
    scoreText.setFont(font);
    scoreText.setCharacterSize(16);
    scoreText.setPosition({scoreBox.getPosition().x + 10.f, scoreBox.getPosition().y - 20.f});

    sf::Text scoreValueText(font);
    scoreValueText.setString(std::to_string(score));
    scoreValueText.setFont(font);
    scoreValueText.setCharacterSize(16);
    scoreValueText.setPosition({scoreBox.getPosition().x + 10.f, scoreBox.getPosition().y + 5.5f});

    sf::Text gamemodeText(font);
    gamemodeText.setString("Mode de jeu : 1");
    gamemodeText.setFont(font);
    gamemodeText.setCharacterSize(16);
    gamemodeText.setPosition({190.f, gamemodeBox.getPosition().y + 5.5f});

    sf::Text creditsText(font);
    creditsText.setString("Credits");
    creditsText.setFont(font);
    creditsText.setCharacterSize(16);
    creditsText.setPosition({CreditsBox.getPosition().x + 45.f, CreditsBox.getPosition().y + 5.5f});

    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("./extras/logo2.png")) {
        std::cerr << "Erreur chargement logo.png" << std::endl;
    }
    sf::Sprite logoSprite(logoTexture);
    logoSprite.setScale({640.f / logoTexture.getSize().x , 480.f / logoTexture.getSize().y/2});
    logoSprite.setPosition({20.f,240+ 20.f});

    sf::Texture labTexture;
    if (!labTexture.loadFromFile("./extras/lab"+std::to_string(labnumber) +".png")) {
        std::cerr << "Erreur chargement lab"+std::to_string(labnumber) +"png" << std::endl;
        return -1;
    }
    sf::Sprite labSprite(labTexture);
    labSprite.setPosition({20.f, 20.f});
    float scaleX = 640.0f / labTexture.getSize().x;
    float scaleY = 480.0f / labTexture.getSize().y;
    labSprite.setScale({scaleX, scaleY});
    labSprite.setTexture(labTexture,true); 

    while (window.isOpen()) {              
        while (const std::optional event = window.pollEvent()) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouse = window.mapPixelToCoords(pixelPos);
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                if (startButton.getGlobalBounds().contains(mouse)) game.openPage();
                if (stopButton.getGlobalBounds().contains(mouse)) game.closePage();
                if (quitButton.getGlobalBounds().contains(mouse)) { 
                    window.close(); 
                    stop_signal = true; // Signal d'interruption du thread de traitement
                }
                if (playButton.getGlobalBounds().contains(mouse)) {
                    if ((game.getLabyrinthStatus()==false)) {
                        game.resumeGame(); // Reprendre si en pause, démarrer si non joué
                        playText.setString("Pause");
                    } else if (game.getLabyrinthStatus()==true) {
                        game.pauseGame();
                        playText.setString("Jouer");
                    }
                    
                    // Tirer un numéro de 1 à 6
                    gifnumber = rand() % 6 + 1; // tirage aléatoire entre 1 et 6
                    std::cout << "Gif number: " << gifnumber << std::endl;
                }

                if (gamemodeBox.getGlobalBounds().contains(mouse)) {
                    // rajouter 1 à gamemode et le garder entre 1 et 6
                    gamemode++;
                    if (gamemode > 8) gamemode = 1;
                    gamemodeText.setString("Mode de jeu : " + std::to_string(gamemode));
                    // changer la couleur de la box en fonction du mode de jeu
                    gamemodeBox.setFillColor(sf::Color(109, 7, 26 + (gamemode - 1) * 20)); // Couleur différente pour chaque mode
                    isCameraShaking = (gamemode-1)%2;
                    labnumber = (gamemode-1)/2 + 1; // 1, 2, 3, 4, 5, 6
                    std::cout << "Mode de jeu changé : " << gamemode << std::endl;
                    // Charger le labyrinthe correspondant
                    labTexture = sf::Texture();
                    if (!labTexture.loadFromFile("./extras/lab" + std::to_string(labnumber) + ".png")) {
                        std::cerr << "Erreur chargement lab" << std::to_string(labnumber) + ".png" << std::endl;
                    } else {
                        // Forcer l'échelle pour obtenir 640x480
                        float scaleX = 640.0f / labTexture.getSize().x;
                        float scaleY = 480.0f / labTexture.getSize().y;
                        
                        labSprite.setTexture(labTexture, true); //true pour reset la taille
                        labSprite.setPosition({20.f, 20.f});
                        labSprite.setScale({scaleX, scaleY});
                        
                        table(gamemode, startbox, endbox); // Mettre à jour les positions de la startbox et de la endbox
                    }
                }
                // Réglage des bouton en jeu
                if (game.getPageStatus()) { 
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
                            //mettre la couleur BGR
                            colorBox.setFillColor(sf::Color(color[2], color[1], color[0])); // BGR to RGB
                        }
                    }   
                }
            }

            if (event->is<sf::Event::MouseButtonPressed>() || event->is<sf::Event::MouseMoved>()) {
            }
        }

        window.clear(sf::Color(30, 30, 30));

        if (game.getPageStatus()) {
            std::lock_guard<std::mutex> lock(processed_data.mutex);
            if (processed_data.ready && !processed_data.frame.empty()) {
                cv::Mat srcrgb = processed_data.frame;
                cv::Mat src;
                cv::cvtColor(srcrgb, src, cv::COLOR_BGR2RGBA);
                sf::Image image({static_cast<unsigned int>(src.cols), static_cast<unsigned int>(src.rows)},
                    reinterpret_cast<const std::uint8_t*>(src.ptr()));
                sf::Texture texture;
                if (!texture.loadFromImage(image)) {
                    std::cerr << "Failed to load texture from camera image" << std::endl;
                    continue;
                }
                sf::Sprite sprite(texture);
                sprite.setPosition({20.f, 20.f});
                window.draw(sprite);

                if(game.getLabyrinthStatus()) {
                    // Garder l'échelle fixe au lieu de la recalculer
                    labSprite.setColor(sf::Color(255, 255, 255, 128));
                    if (game.getStatus() == GameStatus::PLAYING) {
                        labSprite.setColor(sf::Color(255, 255, 255, 255));
                    }
                    window.draw(labSprite);
                }
            }

            if(game.getLabyrinthStatus()) {
                //dessiner un carré rouge dans le coin inferieur droit
                sf::RectangleShape endBox(sf::Vector2f(30.f, 30.f));
                //violet
                endBox.setFillColor(sf::Color::Magenta);
                endBox.setPosition({static_cast<float>(20) + endbox.x,static_cast<float>(20) + endbox.y});
                window.draw(endBox);
                sf::Text endText(font);
                endText.setString("E");
                endText.setFont(font);
                endText.setCharacterSize(24);
                endText.setFillColor(sf::Color::White);
                endText.setPosition({static_cast<float>(20) + endbox.x + static_cast<float>(6),static_cast<float>(20) + endbox.y+6});
                window.draw(endText);


                sf::RectangleShape startBox(sf::Vector2f(30.f, 30.f));
                startBox.setFillColor(sf::Color::Green);
                startBox.setPosition({static_cast<float>(20) + startbox.x,static_cast<float>(20) + startbox.y});
                window.draw(startBox);
                sf::Text startText(font);
                startText.setString("S");
                startText.setFont(font);
                startText.setCharacterSize(24);
                startText.setFillColor(sf::Color::White);
                startText.setPosition({static_cast<float>(20) + startbox.x + static_cast<float>(6),static_cast<float>(20) + startbox.y + static_cast<float>(6)});
                window.draw(startText);
            }

            if (game.getLabyrinthStatus() && (game.getStatus() == GameStatus::NOT_PLAYING)) {
                bouclestart = 0;
            }
            
            if (game.getStatus() == GameStatus::INITIALIZING) {
                bouclestart++;
                if(bouclestart > 100) { 
                    std::cout << "Lancement du jeu" << std::endl;
                    game.startGame();
                    bouclestart = 0;
                }
            }



            if (game.getStatus() == GameStatus::WINNING) {
                //jouer le gif ./extras/win.gif avec les frames qui tournent 5 fois puis stop
                sf::Text winText(font);
                winText.setString("WIN ! +1 POINT");
                winText.setFont(font);
                winText.setCharacterSize(24);
                winText.setFillColor(sf::Color::Green);
                winText.setPosition({100.f, basey + 200.f});
                window.draw(winText);
                //faire tourner le gif 5 fois
                for(int i=0; i<5; i++){
                    std::string gifPath = "./extras/win.gif";
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
                        soundnumber = 3;
                    }
                }
                score++;
                game.setLabyrinthStatus(false);
                game.idleGame();
                playText.setString("Jouer");
            }
            if (game.getStatus() == GameStatus::INITIALIZING) {
                sf::Text startText(font);
                startText.setString("Restez dans la startbox ...");
                startText.setFont(font);
                startText.setCharacterSize(24);
                startText.setFillColor(sf::Color::White);
                startText.setPosition({100.f, basey + 200.f});
                window.draw(startText);
            }
            if (game.getWallTouched()) {
                sf::Text endText(font);
                endText.setString("!!GAME OVER!!");
                endText.setStyle(sf::Text::Bold);
                endText.setFont(font);
                endText.setCharacterSize(24);
                endText.setFillColor(sf::Color::Red);
                endText.setPosition({100.f, basey + 200.f});
                window.draw(endText);
                //afficher le gif correspondant au numero
                std::string gifPath = "./extras/screamgif/00" + std::to_string(gifnumber) + ".gif";
                std::cout << "Gif number: affiché " << gifnumber << std::endl;
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
                    soundnumber = rand() % 2 + 1; // Tirage aléatoire entre 1 et 2 pour le son
                    playText.setString("Jouer");
                    game.reset();
                    
                }
            }
        }
        else {
            window.draw(logoSprite);
        } 
    
        // Configuration de l'interface
        if (game.getPageStatus()) { // Si le jeu est lancé
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
            startButton.setFillColor(sf::Color(64, 64, 64));
            stopButton.setFillColor(sf::Color(200, 100, 100));
            window.draw(colorBox);
            window.draw(colorText);
            window.draw(scoreBox);
            window.draw(scoreText);
            window.draw(scoreValueText);
            window.draw(gamemodeBox);
            window.draw(gamemodeText);
            window.draw(CreditsBox);
            window.draw(creditsText);
        } else {
            startButton.setPosition({50, basey});
            stopButton.setPosition({200, basey});
            quitButton.setPosition({350, basey});
            startText.setPosition({75.f, basey + 5.f});
            stopText.setPosition({230.f, basey + 5.f});
            quitText.setPosition({370.f, basey + 5.f});
            startButton.setFillColor(sf::Color(100, 200, 100));
            stopButton.setFillColor(sf::Color(64, 64, 64));
        }
        
        

        window.draw(stopButton);
        window.draw(quitButton);
        window.draw(stopText);
        window.draw(quitText);
        window.draw(startButton);
        window.draw(startText);

        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        
        if (playingsound) {
            //joue le son extras/sound/1.mp3
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile("./extras/sounds/"+std::to_string(soundnumber)+".mp3")) {
                std::cerr << "Erreur chargement son" << std::endl;
            } else {
                sound = std::make_unique<sf::Sound>(buffer);
                sound->play();
                while (sound->getStatus() == sf::SoundSource::Status::Playing) {    
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            playingsound = false; // Réinitialiser le flag pour ne pas jouer le son à chaque frame}
        }
    }
    return 0;
}


