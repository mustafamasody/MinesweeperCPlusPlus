#include <SFML/Graphics.hpp>
#include <iostream>
#include "WelcomeWindow.h"
#include "Game.h"

int main() {

    sf::RenderWindow window(sf::VideoMode(800, 600), "Minesweeper");
    WelcomeWindow welcome(window);
    Game* game = nullptr;

    enum class GameState { Welcome, GameActive } currentState = GameState::Welcome;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (currentState == GameState::GameActive && game != nullptr) {
                game->handleInput(event);
            } else if (currentState == GameState::Welcome) {
                welcome.handleInput(event);
            }
        }

        if (currentState == GameState::Welcome && welcome.isDone()) {
            std::string playerName = welcome.getPlayerName();

            game = new Game(window, playerName);

            currentState = GameState::GameActive;
        }

        window.clear(sf::Color::Black);

        if (currentState == GameState::Welcome) {
            welcome.drawUnchanging();
        } else if (currentState == GameState::GameActive && game != nullptr) {
            game->updateGame();
            game->renderGame();
            if (game->isGameOver() || game->isVictory()) {
//                delete game;
//                game = nullptr;
//                currentState = GameState::Welcome;
//                welcome.setDone(false);
//                cout << "Game over or victory\n";
                // reveal all tiles
                game->revealAllTiles();
                //game->showAllMines();
            }


        }

    }

    delete game;
    return 0;
}