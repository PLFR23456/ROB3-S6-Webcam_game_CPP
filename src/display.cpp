#include "commande.hpp" // Inclure le fichier d'en-tête commande.hpp pour les variables globales
#include "camera.hpp"  // Inclure le fichier d'en-tête camera.hpp pour les variables globales
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <sstream>
// Simule les variables globales (à relier à ton code réel)
bool running = false;
extern double gainK;
extern int tol;

int display() {
    sf::RenderWindow window(sf::VideoMode(500, 300), "Color Tracking - Interface");

    // Sliders
    sf::RectangleShape gainSlider(sf::Vector2f(200, 5));
    gainSlider.setPosition(150, 60);
    gainSlider.setFillColor(sf::Color::White);

    sf::CircleShape gainKnob(8);
    gainKnob.setFillColor(sf::Color::Red);

    sf::RectangleShape tolSlider(sf::Vector2f(200, 5));
    tolSlider.setPosition(150, 120);
    tolSlider.setFillColor(sf::Color::White);

    sf::CircleShape tolKnob(8);
    tolKnob.setFillColor(sf::Color::Blue);

    // Boutons
    sf::RectangleShape startButton(sf::Vector2f(100, 30));
    startButton.setPosition(50, 200);
    startButton.setFillColor(sf::Color(100, 200, 100));

    sf::RectangleShape stopButton(sf::Vector2f(100, 30));
    stopButton.setPosition(200, 200);
    stopButton.setFillColor(sf::Color(200, 100, 100));

    sf::RectangleShape quitButton(sf::Vector2f(100, 30));
    quitButton.setPosition(350, 200);
    quitButton.setFillColor(sf::Color(100, 100, 200));

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    sf::Text gainText("Gain K", font, 16);
    gainText.setPosition(50, 50);
    sf::Text tolText("Tolerance", font, 16);
    tolText.setPosition(50, 110);

    sf::Text startText("Start", font, 16);
    startText.setPosition(75, 205);
    sf::Text stopText("Stop", font, 16);
    stopText.setPosition(230, 205);
    sf::Text quitText("Quitter", font, 16);
    quitText.setPosition(370, 205);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);
                if (startButton.getGlobalBounds().contains(mouse.x, mouse.y)) running = true;
                if (stopButton.getGlobalBounds().contains(mouse.x, mouse.y)) running = false;
                if (quitButton.getGlobalBounds().contains(mouse.x, mouse.y)) window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseMoved) {
                auto mouse = sf::Mouse::getPosition(window);
                // Gain slider
                if (mouse.y > 55 && mouse.y < 75 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    gainK = float(mouse.x - 150) / 200.0f * 2.0f; // 0.0 à 2.0
                }
                // Tol slider
                if (mouse.y > 115 && mouse.y < 135 && mouse.x > 150 && mouse.x < 350 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    tol = (mouse.x - 150) / 2; // 0 à 100
                }
            }
        }

        // Position des knobs
        gainKnob.setPosition(150 + gainK/2.0f*200 - 8, 56);
        tolKnob.setPosition(150 + tol*2 - 8, 116);

        window.clear(sf::Color(30, 30, 30));
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

        // Affiche valeurs
        std::ostringstream oss;
        oss << "Gain K: " << gainK;
        sf::Text gainVal(oss.str(), font, 14);
        gainVal.setPosition(370, 50);
        window.draw(gainVal);

        oss.str(""); oss.clear();
        oss << "Tol: " << tol;
        sf::Text tolVal(oss.str(), font, 14);
        tolVal.setPosition(370, 110);
        window.draw(tolVal);

        window.display();
    }
    return 0;
}