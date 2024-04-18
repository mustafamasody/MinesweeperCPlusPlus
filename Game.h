#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <chrono>
#include "Tile.h"
#include "SpriteManager.h"
#include <fstream>
#include <iostream>
#include <istream>

using namespace std;
using namespace std::chrono;

class Game {
private:
    sf::RenderWindow& window;
    vector<vector<Tile>> grid;
    int rows, cols, mineCount;
    bool gameOver, victory, paused;
    SpriteManager spriteManager;
    steady_clock::time_point startTime, pauseTime;
    duration<int> pausedDuration;
    map<int, sf::Sprite> digitSprites;

    bool showingLeaderboard = false;

    std::string playerName;

    int flagCount;
    int initialMineCount;
    map<int, sf::Sprite> mineCounterDigits;


    sf::Sprite debugButton;
    sf::Texture* textureDebug;
    bool debugMode;

    sf::Sprite pauseButton;
    sf::Texture* texturePause;
    sf::Texture* texturePlay;

    sf::Sprite leaderboardButton;
    sf::Texture textureLeaderboard;

    sf::Sprite smileyFace;
    sf::Texture* textureFaceHappy;
    sf::Texture* textureFaceWin;
    sf::Texture* textureFaceLose;


public:

    Game(sf::RenderWindow& win, const std::string& name)
            : window(win), gameOver(false), victory(false), playerName(name) {
        spriteManager.loadSprites(); // Ensure all sprites are loaded
        digitSprites = spriteManager.getDigitSprites(); // Retrieve digit sprites
        loadConfig();
        initializeGame();
    }

    const int UI_HEIGHT = 100;

    void loadConfig() {
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

        spriteManager.loadSprites();
        digitSprites = spriteManager.getDigitSprites();
        startTime = steady_clock::now();
    }

    void initializeGame() {
        grid.resize(rows, vector<Tile>());

        map<int, sf::Texture*> numberTextures;
        numberTextures[1] = &spriteManager.getTexture("number_1");
        numberTextures[2] = &spriteManager.getTexture("number_2");
        numberTextures[3] = &spriteManager.getTexture("number_3");
        numberTextures[4] = &spriteManager.getTexture("number_4");
        numberTextures[5] = &spriteManager.getTexture("number_5");
        numberTextures[6] = &spriteManager.getTexture("number_6");
        numberTextures[7] = &spriteManager.getTexture("number_7");
        numberTextures[8] = &spriteManager.getTexture("number_8");

        textureFaceHappy = &spriteManager.getTexture("face_happy");
        textureFaceWin = &spriteManager.getTexture("face_win");
        textureFaceLose = &spriteManager.getTexture("face_lose");

        smileyFace.setTexture(*textureFaceHappy);
        smileyFace.setPosition(window.getSize().x / 2 - smileyFace.getGlobalBounds().width / 2, window.getSize().y - UI_HEIGHT / 2 - smileyFace.getGlobalBounds().height / 2);

        textureDebug = &spriteManager.getTexture("debug");

        texturePause = &spriteManager.getTexture("pause");
        texturePlay = &spriteManager.getTexture("play");

        pauseButton.setTexture(*texturePause);
        pauseButton.setPosition(window.getSize().x - 275, window.getSize().y - UI_HEIGHT / 2 - pauseButton.getGlobalBounds().height / 2);
        pausedDuration = duration<int>::zero();

        textureLeaderboard = spriteManager.getTexture("leaderboard");
        leaderboardButton.setTexture(textureLeaderboard);
        leaderboardButton.setPosition((cols * 32) - 210, 31.4 * (rows + 0.5f));

        initialMineCount = mineCount;
        flagCount = 0;

        mineCounterDigits = spriteManager.getDigitSprites();

        debugButton.setTexture(*textureDebug);
        debugButton.setPosition(window.getSize().x - 340, window.getSize().y - UI_HEIGHT / 2 - debugButton.getGlobalBounds().height / 2);

        debugMode = false;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                sf::Texture& hidden = spriteManager.getTexture("tile_hidden");
                sf::Texture& mine = spriteManager.getTexture("mine");
                sf::Texture& revealed = spriteManager.getTexture("tile_revealed");
                sf::Texture& flag = spriteManager.getTexture("flag");

                grid[i].emplace_back(hidden, mine, revealed, flag, numberTextures, [this](bool flagAdded) {
                    if (flagAdded) {
                        ++flagCount;
                    } else {
                        --flagCount;
                    }
                    updateMineCounter();
                });
                grid[i][j].setPosition(j * 32, i * 32);
            }
        }
        placeMines();
        calculateAdjacency();
    }

    void revealAdjacentTiles(int row, int col) {
        for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
                int ni = row + di, nj = col + dj;
                if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && !grid[ni][nj].getIsFlagged()) {
                    grid[ni][nj].reveal();
                }
            }
        }
    }

    void updateGame() {

        if (gameOver || victory) {
            debugMode = false;
        }

        if (paused) return;

        if (gameOver) {
            smileyFace.setTexture(victory ? *textureFaceWin : *textureFaceLose);
            for (auto& row : grid) {
                for (auto& tile : row) {
                    if (tile.getIsMine()) {
                        tile.reveal();
                    }
                }
            }
        } else {
            checkVictory();
        }
    }

    void renderGame() {
        window.clear(sf::Color::White);
        for (auto& row : grid) {
            for (Tile& tile : row) {
                tile.draw(window);
            }
        }
        drawUI();
        window.display();
    }

    void updateMineCounter() {
        drawMineCounter();
    }

    void drawMineCounter() {
        int counter = initialMineCount - flagCount;
        //int xPosition = 21; // Adjust as necessary for the correct starting position
        //int yPosition = window.getSize().y - UI_HEIGHT + 50; // Adjust for the correct vertical position

        int xPosition = 21;
        int yPosition = 32 * (rows + 0.5f) + 16;

        if (counter < 0) {
            // 12, 32 × (number of rows + 0.5) + 16

            int minusXPosition = 12;

            sf::Sprite& negativeSprite = mineCounterDigits[10]; // Assuming 10 is the index for the negative sign
            negativeSprite.setPosition(minusXPosition, yPosition);
            window.draw(negativeSprite);

            xPosition += negativeSprite.getGlobalBounds().width;

            counter = abs(counter);
        }

        drawDigits(counter, xPosition, yPosition);
    }

    void drawUI() {
        int yPos = window.getSize().y - UI_HEIGHT;

        window.draw(smileyFace);
        window.draw(debugButton);
        window.draw(pauseButton);

        displayTimer();
        drawMineCounter();
        window.draw(leaderboardButton);

    }

    bool alreadyPausedGameOver = false;

    void togglePause() {
        auto now = steady_clock::now();

        if (gameOver) {
            paused = true;
            pauseButton.setTexture(*texturePause);
            pauseTime = now;
            return;
        }


        if (paused) {
            // Resume game
            paused = false;
            pauseButton.setTexture(*texturePause);
            pausedDuration += duration_cast<seconds>(now - pauseTime);
        } else {
            // Pause game
            paused = true;
            pauseButton.setTexture(*texturePlay);
            pauseTime = now;
        }

        for (auto& row : grid) {
            for (auto& tile : row) {
                tile.togglePausedState();
            }
        }
    }

    void displayTimer() {
        auto now = steady_clock::now();
        duration<int> effectiveDuration = duration_cast<seconds>(now - startTime) - pausedDuration;

        if (paused) {
            effectiveDuration = duration_cast<seconds>(pauseTime - startTime) - pausedDuration;
        }

        int gameTime = effectiveDuration.count();
        gameTime = max(gameTime, 0);  // Ensure gameTime is non-negative

        //cout << "Game duration: " << gameTime / 60 << " minutes, " << gameTime % 60 << " seconds" << endl;

//        drawDigits(gameTime / 60, (cols * 32) - 97, window.getSize().y - 70);  // Minutes
//        drawDigits(gameTime % 60, (cols * 32) - 54, window.getSize().y - 70);  // Seconds

        // x: (num of columns * 32) - 54
        // y: 32 * (num of rows + 0.5) + 16
        drawDigits(gameTime / 60, (cols * 32) - 97, 32 * (rows + 0.5f) + 16);  // Minutes
        drawDigits(gameTime % 60, (cols * 32) - 54, 32 * (rows + 0.5f) + 16);  // Seconds

    }

    void drawDigits(int time, int xPosition, int yPosition) {
        int tens = time / 10;
        int ones = time % 10;
        digitSprites[tens].setPosition(xPosition, yPosition);
        window.draw(digitSprites[tens]);
        digitSprites[ones].setPosition(xPosition + 21, yPosition); // Next digit position
        window.draw(digitSprites[ones]);
    }

    void drawDigits(int time, int xPosition) {
        int tens = time / 10;
        int ones = time % 10;
        digitSprites[tens].setPosition(xPosition, 32 * (rows + 0.5f) + 16);
        window.draw(digitSprites[tens]);
        digitSprites[ones].setPosition(xPosition + 21, 32 * (rows + 0.5f) + 16);
        window.draw(digitSprites[ones]);
    }

    void placeMines() {
        int minesPlaced = 0;
        while (minesPlaced < mineCount) {
            int x = rand() % cols;
            int y = rand() % rows;
            if (!grid[y][x].getIsMine()) {
                grid[y][x].setMine(true);
                minesPlaced++;
            }
        }
    }

    void calculateAdjacency() {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (grid[i][j].getIsMine()) {
                    continue;
                }
                int mineCount = 0;
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && grid[ni][nj].getIsMine()) {
                            mineCount++;
                        }
                    }
                }
                grid[i][j].setAdjacentMines(mineCount);
            }
        }
    }

    void handleInput(sf::Event event) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed) {

            handleMouse(event.mouseButton.x, event.mouseButton.y, event.mouseButton.button);
        }

    }

//    void handleMouse(int x, int y, sf::Mouse::Button button) {
//        int col = x / 32;
//        int row = y / 32;
//        if (row >= 0 && row < rows && col >= 0 && col < cols) {
//            Tile& tile = grid[row][col];
//            cout << "Clicked on tile at row: " << row << ", col: " << col << endl;
//            if (button == sf::Mouse::Left) {
//                tile.reveal();
//                if (tile.getIsMine()) {
//                    gameOver = true;
//                }
//            } else if (button == sf::Mouse::Right) {
//                tile.toggleFlag();
//            }
//        }
//    }

    void revealMines() {
        for (auto& row : grid) {
            for (auto& tile : row) {
                if (tile.getIsMine()) {
                    tile.reveal();
                }
            }
        }
    }

    void hideMines() {
        for (auto& row : grid) {
            for (auto& tile : row) {
                if (tile.getIsMine()) {
                    tile.hide();
                }
            }
        }
    }

    void showAllMines() {
        for (auto& row : grid) {
            for (auto& tile : row) {
                if (tile.getIsMine()) {
                    tile.revealOver();
                }
            }
        }
    }

    void pauseGame() {
        if (!paused) {
            togglePause();
        }
    }

    void togglePausedStateTiles(){

        for (auto& row : grid) {
            for (auto& tile : row) {
                if (tile.getIsRevealed()) {
                    tile.togglePausedState();
                }
            }
        }

    }

    void handleMouse(int x, int y, sf::Mouse::Button button) {

        if(showingLeaderboard){
            return;
        }

        if (pauseButton.getGlobalBounds().contains(x, y)) {
            togglePause();
            return;
        }

        if (smileyFace.getGlobalBounds().contains(x, y)) {
            resetGame();
            return;
        }

        if (leaderboardButton.getGlobalBounds().contains(x, y)) {
            showLeaderboard(false);
        }

        if (paused) return;

        if (debugButton.getGlobalBounds().contains(x, y) && !gameOver) {
            if (debugMode) {
                debugMode = false;
                hideMines();
            } else {
                debugMode = true;
                revealMines();
            }
            return;
        }

        int col = x / 32;
        int row = y / 32;

//        if (row >= 0 && row < rows && col >= 0 && col < cols) {
//            Tile& tile = grid[row][col];
//            if (button == sf::Mouse::Left && !tile.getIsFlagged()) {
//                if (!tile.getIsRevealed()) {
//                    tile.reveal();
//                    if (tile.getIsMine()) {
//                        gameOver = true;
//                        showAllMines();
//                        pauseGame(); // You may want to call pauseGame instead of togglePause to ensure the game is paused
//                        showSadFace();
//                    }
//                }
//            }else if (button == sf::Mouse::Right) {
//                cout << "Toggling flag on tile at row: " << row << ", col: " << col << endl;
//                tile.toggleFlag();
//            }
//        }

        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            Tile& tile = grid[row][col];
            if (button == sf::Mouse::Left) {

                if (tile.getIsMine()) {
                    if(!tile.getIsFlagged()){
                        gameOver = true;
                        showAllMines();
                        pauseGame();
                        showSadFace();
                        revealAdjacentTiles(row, col);
                    }
                }else if (tile.getAdjacentMines() == 0) {
                    cout << "Revealing adjacent blanks" << endl;
                    revealAdjacentBlanks(row, col); // If no adjacent mines, start revealing process
                }else{
                    tile.reveal();
                }
            } else if (button == sf::Mouse::Right) {
                tile.toggleFlag();
            }
        }
    }

    void revealAdjacentBlanks(int row, int col) {
        cout << "1" << endl;
        // Base case: if the tile is out of bounds or already revealed, return
        if (row < 0 || row >= rows || col < 0 || col >= cols || grid[row][col].getIsRevealed())
            return;

        cout << "2" << endl;

        // Reveal the current tile
        grid[row][col].reveal();

        cout << "3" << endl;

        // If the tile is not blank (it has adjacent mines), just reveal it and return
        if (grid[row][col].getAdjacentMines() > 0)
            return;

        cout << "4" << endl;

        // Recursively reveal all eight neighboring tiles
        for (int di = -1; di <= 1; ++di) {
            for (int dj = -1; dj <= 1; ++dj) {
                if (di == 0 && dj == 0) continue; // Skip the current tile itself
                cout << "Revealing adjacent blank at row: " << row + di << ", col: " << col + dj << endl;
                revealAdjacentBlanks(row + di, col + dj);
            }
        }
    }

    void showLeaderboard(bool won) {
        showingLeaderboard = true;

        for (auto& row : grid) {
            for (auto& tile : row) {
                tile.togglePausedState();
            }
        }

        const int WIDTH = 450;
        const int HEIGHT = 300;
        sf::RenderWindow leaderboardWindow(sf::VideoMode(WIDTH, HEIGHT), "Leaderboard", sf::Style::Titlebar | sf::Style::Close);

        sf::Font font;
        if (!font.loadFromFile("files/font.ttf")) {
            std::cerr << "Could not load font" << std::endl;
            showingLeaderboard = false;
            return;
        }

        sf::Text title("LEADERBOARD", font, 20);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        // underlined
        title.setStyle(sf::Text::Underlined);
        title.setPosition(WIDTH / 3, (HEIGHT / 2) - 120);

        sf::Text entryText;
        entryText.setFont(font);
        entryText.setCharacterSize(18);
        entryText.setFillColor(sf::Color::White);
        entryText.setPosition(80, 80);

        std::string entriesString;
        std::vector<std::string> leaderboardEntries = loadLeaderboardEntries(won);
        for(int i = 0; i < leaderboardEntries.size(); i++){
            entriesString += to_string(i + 1) + ". " + leaderboardEntries[i] + "\n";
        }
//        for (const auto& entry : leaderboardEntries) {
//            entriesString += entry + "\n";
//        }
        entryText.setString(entriesString);

        while (leaderboardWindow.isOpen()) {
            sf::Event event;
            while (leaderboardWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    leaderboardWindow.close();
                }
            }

            leaderboardWindow.clear(sf::Color::Blue);
            leaderboardWindow.draw(title);
            leaderboardWindow.draw(entryText);
            leaderboardWindow.display();
        }

        sf::Event eventToDiscard;
        while (window.pollEvent(eventToDiscard)) {
        }

        showingLeaderboard = false;
        for (auto& row : grid) {
            for (auto& tile : row) {
                tile.togglePausedState();
            }
        }
    }

    std::vector<std::string> loadLeaderboardEntries(bool won) {
        std::vector<std::string> entries;
        std::ifstream file("files/leaderboard.txt");
        std::string line;
        while (getline(file, line)) {
            entries.push_back(line);
        }

        sort(entries.begin(), entries.end());

        // shrink to top 5
        if(entries.size() > 5){
            entries.resize(5);
        }

        if(won){
            auto now = steady_clock::now();
            auto winningDuration = duration_cast<seconds>(now - startTime) - pausedDuration;
            int winningTime = winningDuration.count();
            for (int i = 0; i < entries.size(); i++) {
                string entry = entries[i];
                string timeString = entry.substr(0, 5);
                int minutes = stoi(timeString.substr(0, 2));
                int seconds = stoi(timeString.substr(3, 2));
                int time = minutes * 60 + seconds;
                if (winningTime < time) {
                    string minutes = to_string(winningTime / 60);
                    string seconds = to_string(winningTime % 60);
                    if (minutes.length() == 1) {
                        minutes = "0" + minutes;
                    }
                    if (seconds.length() == 1) {
                        seconds = "0" + seconds;
                    }
                    string totalTime = minutes + ":" + seconds;
                    for (int j = 0; j < entries.size(); j++) {
                        if (entries[j].find(playerName) != std::string::npos) {
                            entries.erase(entries.begin() + j);
                            break;
                        }
                    }
                    entries.insert(entries.begin() + i, totalTime + ", " + playerName + "*");

                    // shrink to top 5
                    entries.resize(5);
                    break;
                }
            }
        }

        for (int i = 0; i < entries.size(); i++) {
            string entry = entries[i];
            string timeString = entry.substr(0, 5);
            string playerName = entry.substr(7);
            entries[i] = string(10 - timeString.length(), ' ')+ timeString + string(10 - timeString.length(), ' ') + playerName + "\n";
        }

        return entries;
    }

    void showSadFace() {
        smileyFace.setTexture(*textureFaceLose);
    }

    void revealAllTiles() {
        for (auto& row : grid) {
            for (auto& tile : row) {
                tile.revealOver();
            }
        }

//        // reveal all adjacent tiles
//        for (int i = 0; i < rows; i++) {
//            for (int j = 0; j < cols; j++) {
//                if (grid[i][j].getIsMine()) {
//                    revealAdjacentTiles(i, j);
//                }
//            }
//        }

    }

    void resetGame() {
        gameOver = false;
        victory = false;
        paused = false;
        smileyFace.setTexture(*textureFaceHappy);

        pausedDuration = duration<int>::zero();

        // clear the grid
        for (auto& row : grid) {
            for (auto& tile : row) {
                tile.reset();
            }
        }

        grid.clear();

        startTime = steady_clock::now();

        initializeGame();
    }

    void appendToLeaderboard(const std::string& playerName, const std::string& time) {
        std::ifstream infile("files/leaderboard.txt", std::ios::ate);
        bool needsNewline = false;

        if (infile.is_open()) {

            if (infile.tellg() > 0) {
                infile.seekg(-1, std::ios_base::end);

                char lastChar;
                infile.get(lastChar);
                if (lastChar != '\n') {
                    needsNewline = true;
                }
            }
            infile.close();
        } else {
            std::cerr << "Unable to open leaderboard file for reading." << std::endl;
            return;
        }

        std::ofstream outfile("files/leaderboard.txt", std::ios::app);

        if (outfile.is_open()) {
            if (needsNewline) {
                outfile << std::endl;
            }
            outfile << time << ", " << playerName << std::endl;
            outfile.close();
        } else {
            std::cerr << "Unable to open leaderboard file for appending." << std::endl;
        }
    }

    void recordVictory() {
        if(victory) {
            auto now = steady_clock::now();
            auto winningDuration = duration_cast<seconds>(now - startTime) - pausedDuration;
            int winningTime = winningDuration.count();
            int minutes = winningTime / 60;
            int seconds = winningTime % 60;

            string timeString = to_string(minutes) + ":" + to_string(seconds);
            if (minutes < 10) {
                timeString = "0" + timeString;
            }
            if (seconds < 10) {
                timeString = timeString.substr(0, 3) + "0" + timeString.substr(3);
            }

            appendToLeaderboard(playerName, timeString);

            showLeaderboard(true);

//            // Open the leaderboard file in append mode
//            ofstream leaderboardFile("files/leaderboard.txt", ios::app);
//            if (!leaderboardFile.is_open()) {
//                cerr << "Unable to open leaderboard file!" << endl;
//                return;
//            }
//
//            // Write the new entry
//            leaderboardFile << setfill('0') << setw(2) << minutes << ':'
//                            << setfill('0') << setw(2) << seconds << ", "
//                            << playerName << endl;
//
//            // Close the file
//            leaderboardFile.close();
//

        }
    }

    void checkVictory() {
        victory = true;
        for ( auto& row : grid) {
            for ( auto& tile : row) {
                if (!tile.getIsMine() && !tile.getIsRevealed()) {
                    victory = false;
                    return;
                }
            }
        }
        if (victory) {

            gameOver = true;
            togglePause();
            recordVictory();
        }
    }

    bool isGameOver()  {
        return gameOver;
    }

    bool isVictory()  {
        return victory;
    }
};