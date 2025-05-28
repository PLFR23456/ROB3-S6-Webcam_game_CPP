#ifndef DISPLAY_H
#define DISPLAY_H

#include "camera.hpp"

extern bool isCameraShaking;
extern int labnumber;

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
    bool getWallTouched() { return isWallTouched; }

    // Setters
    void setLabyrinthStatus(bool stat) { isLabyrinthDisplayed = stat; }

    void openPage() { isPageOpened = true; }
    void closePage() { isPageOpened = false; }

    void startGame() { status = GameStatus::PLAYING; }

    void idleGame() {
        status = GameStatus::NOT_PLAYING;
    }

    void pauseGame() {
        if (status == GameStatus::PLAYING || status == GameStatus::INITIALIZING) {
            status = GameStatus::PAUSED;
        }
        isLabyrinthDisplayed = false;
    }

    void resumeGame() {
        if (status == GameStatus::PAUSED || status == GameStatus::NOT_PLAYING) {
            isLabyrinthDisplayed = true;
        
            if( status == GameStatus::PAUSED){
                status = GameStatus::PLAYING;
            }
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
        if (status == GameStatus::PLAYING) {
            isWallTouched = true;
        }
    }

    void reset() {
        isLabyrinthDisplayed = false;
        status = GameStatus::NOT_PLAYING;
        isWallTouched = false;
    }
};

extern GameSession game;
// Prototype de fonction
int display(GameSession& game);

#endif
