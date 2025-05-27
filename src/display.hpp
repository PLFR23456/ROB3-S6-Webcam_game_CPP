#ifndef DISPLAY_H
#define DISPLAY_H

#include "commande.hpp"

enum class GameStatus {
    NOT_PLAYING,
    IN_START_BOX,
    PLAYING,
    IN_END_BOX,
    TOUCHING_WALLS,
    PAUSED
};

class GameSession {
private:
    bool isPageOpen = false;
    bool isStarted = false;

    Position startBox;
    Position endBox;
    GameStatus status = GameStatus::NOT_PLAYING;

public:
    GameSession(Position start, Position end)
        : startBox(start), endBox(end) {}

    // Getters
    bool pageOpen() const { return isPageOpen; }
    bool started() const { return isStarted; }
    GameStatus getStatus() const { return status; }

    // Setters
    void openPage() { isPageOpen = true; }
    void closePage() { isPageOpen = false; }

    void startGame() {
        isStarted = true;
        status = GameStatus::PLAYING;
    }

    void pauseGame() {
        if (status == GameStatus::PLAYING) {
            status = GameStatus::PAUSED;
        }
    }

    void resumeGame() {
        if (status == GameStatus::PAUSED) {
            status = GameStatus::PLAYING;
        }
    }

    void enterStartBox() {
        status = GameStatus::IN_START_BOX;
    }

    void enterEndBox() {
        if (status == GameStatus::PLAYING) {
            status = GameStatus::IN_END_BOX;
        }
    }

    void touchWall() {
        if (status == GameStatus::PLAYING) {
            status = GameStatus::TOUCHING_WALLS;
        }
    }

    void reset() {
        isPageOpen = false;
        isStarted = false;
        status = GameStatus::NOT_PLAYING;
    }
};

// Prototype de fonction
int display(GameSession& game);

#endif
