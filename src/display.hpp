#ifndef DISPLAY_H
#define DISPLAY_H

#include "commande.hpp"

enum class GameStatus {
    NOT_PLAYING,
    INITIALIZING,
    PLAYING,
    WINNING,
    PAUSED
};

class GameSession {
private:
    bool isPageOpened = false;
    bool isLabyrinthDisplayed = false;
    bool isWallTouched = false;

    Position startBox;
    Position endBox;
    GameStatus status = GameStatus::NOT_PLAYING;

public:
    GameSession(Position start, Position end)
        : startBox(start), endBox(end) {}

    // Getters
    bool getPageStatus() const { return isPageOpened; }
    bool getLabyrinthStatus() const { return isLabyrinthDisplayed; }
    GameStatus getStatus() const { return status; }

    // Setters
    void setLabyrinthStatus(bool status) { isLabyrinthDisplayed = true; }

    void openPage() { isPageOpened = true; }
    void closePage() { isPageOpened = false; }

    void startGame() { status = GameStatus::PLAYING; }

    void pauseGame() {
        isLabyrinthDisplayed = false;
        if (status == GameStatus::PLAYING) {
            status = GameStatus::PAUSED;
        }
    }

    void resumeGame() {
        isLabyrinthDisplayed = true;
        if (status == GameStatus::PAUSED) {
            status = GameStatus::PLAYING;
        }
    }

    void enterStartBox() {
        if (status == GameStatus::NOT_PLAYING) {
            status = GameStatus::INITIALIZING;
        }
    }

    void enterEndBox() {
        if (status == GameStatus::PLAYING) {
            status = GameStatus::WINNING;
        }
    }

    void touchWall() {
        isLabyrinthDisplayed = false;
        if (status == GameStatus::PLAYING) {
            isWallTouched = true;
        }
    }

    void reset() {
        isLabyrinthDisplayed = false;
        isPageOpened = false;
        status = GameStatus::NOT_PLAYING;
    }
};

// Prototype de fonction
int display(GameSession& game);

#endif
