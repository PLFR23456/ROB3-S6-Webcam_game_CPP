#ifndef DISPLAY_H
#define DISPLAY_H

#include "commande.hpp"

extern Position startbox;
extern Position endbox;
extern bool isCameraShaking;
extern int labnumber;
//doc
// 0= not playing
// 1= in the start box (waiting ~3 seconds)
// 2= playing
// 3= in the end box
// 4= touching the walls 


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
    void setLabyrinthStatus(bool status) { isLabyrinthDisplayed = true; }

    void openPage() { isPageOpened = true; }
    void closePage() { isPageOpened = false; }

    void startGame() { status = GameStatus::PLAYING; }



    void pauseGame() {
        if (status == GameStatus::PLAYING) {
            status = GameStatus::PAUSED;
        }
    }

    void resumeGame() {
        if (status == GameStatus::PAUSED || status == GameStatus::NOT_PLAYING) {
            status = GameStatus::PLAYING;
            isLabyrinthDisplayed = true;
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
