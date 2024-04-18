#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream>

using namespace std;
class WelcomeWindow {
private:
    sf::RenderWindow& window;
    sf::Font font;
    std::string playerName;
    bool enterPressedLastCheck = false;

    std::chrono::steady_clock::time_point lastKeyPress = std::chrono::steady_clock::now();
    const std::chrono::milliseconds keyPressDelay{1};  // 200 ms between key presses

    void setText(sf::Text &text, float x, float y) {
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(sf::Vector2f(x, y));
    }

public:
    WelcomeWindow(sf::RenderWindow& win) : window(win) {
        if (!font.loadFromFile("files/font.ttf")) {
            std::cerr << "Could not load font\n";
        }
    }

    int WIDTH = window.getSize().x;
    int HEIGHT = window.getSize().y;

    void drawUnchanging() {
        window.clear(sf::Color::Blue);

        int rows, cols, mineCount;

        ifstream file("files/config.cfg");

        // Check if the file opened successfully
        if (!file.is_open()) {
            cerr << "Error opening board_config.cfg" << endl;
            return;
        }

        string line;

        if (getline(file, line) && !line.empty()) {
            cols = stoi(line);
        } else {
            cerr << "Could not read cols from file." << endl;
        }

        if (getline(file, line) && !line.empty()) {
            rows = stoi(line);
        } else {
            cerr << "Could not read rows from file." << endl;
        }

//        int temp = cols;
//        cols = rows;
//        rows = temp;

        if (getline(file, line) && !line.empty()) {
            mineCount = stoi(line);
        } else {
            cerr << "Could not read mineCount from file." << endl;
        }

        sf::Text title("WELCOME TO MINESWEEPER!", font, 24);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold | sf::Text::Underlined);
        float x = WIDTH / 2;
        float y = (HEIGHT / 2) - 150;
        setText(title, x, y);
        window.draw(title);

        title.setString("Enter your name:");
        title.setCharacterSize(20);
        title.setStyle(sf::Text::Bold);
        y = (HEIGHT / 2) - 75;
        setText(title, x, y);
        window.draw(title);

        if (!playerName.empty()) {
            sf::Text typed(playerName + '|', font, 18);
            typed.setFillColor(sf::Color::Yellow);
            typed.setStyle(sf::Text::Bold);
            y = (HEIGHT / 2) - 45;
            setText(typed, x, y);
            window.draw(typed);
        }

        window.display();
    }

    void handleInput(const sf::Event& event) {
        auto now = std::chrono::steady_clock::now();

        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode < 128) {
                char enteredChar = static_cast<char>(event.text.unicode);
                if (std::isalpha(enteredChar) && playerName.length() < 10) {
                    playerName += enteredChar;

                    // DONE: capitalize first letter of playerName and lowercase rest
                    if (playerName.length() == 1) {
                        playerName[0] = std::toupper(playerName[0]);
                    } else {
                        playerName[playerName.length() - 1] = std::tolower(playerName[playerName.length() - 1]);
                    }
                }
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::BackSpace && !playerName.empty() &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeyPress) > keyPressDelay) {
                playerName.pop_back();
                lastKeyPress = now;
            } else if (event.key.code == sf::Keyboard::Enter && !playerName.empty()) {
                if (!enterPressedLastCheck) {
                    enterPressedLastCheck = true;
                    lastKeyPress = now;
                }
            }
        }

        drawUnchanging();
    }

    bool isDone() const {
        return !playerName.empty() && enterPressedLastCheck;
    }

    void setDone(bool done) {
        enterPressedLastCheck = done;
    }

    std::string getPlayerName() const {
        return playerName;
    }
};