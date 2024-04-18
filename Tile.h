using namespace std;
#include <SFML/Graphics.hpp>
#include <map>

class Tile {
private:
    bool isMine;
    bool isRevealed;
    bool isFlagged;
    int adjacentMines;
    sf::Sprite sprite;
    sf::Texture* textureRevealed;
    sf::Texture* textureHidden;
    sf::Texture* textureFlag;
    sf::Texture* textureMine;

    map<int, sf::Texture*> numberTextures;
    std::function<void(bool)> onFlagToggled;

    sf::Texture* currentTexture;

public:
    Tile(sf::Texture& hidden, sf::Texture& mine, sf::Texture& revealed, sf::Texture& flag, map<int, sf::Texture*>& numberTexture, std::function<void(bool)> onFlagToggle) :
            textureHidden(&hidden), textureMine(&mine), textureRevealed(&revealed), textureFlag(&flag), numberTextures(numberTexture),
            isMine(false), isRevealed(false), isFlagged(false), adjacentMines(0), onFlagToggled(onFlagToggle) {
        sprite.setTexture(*textureHidden);
        setCurrentTexture(*textureHidden);
    }

    void setCurrentTexture(sf::Texture& texture){
        currentTexture = &texture;
    }

    void overlayTexture(sf::Texture& overlay) {
        sf::Sprite overlaySprite(overlay);
        overlaySprite.setPosition(sprite.getPosition());
        overlaySprite.setTexture(overlay);
        overlaySprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
        overlaySprite.setColor(sf::Color(255, 255, 255, 128));
        sprite.setTexture(overlay);
    }

    bool pausedState = false;
    sf::Texture pausedTexture;

    void setPausedTexture(){
        sprite.setTexture(*currentTexture);
    }

    void togglePausedState() {
        if(!pausedState){
            pausedState = true;
            setPausedTexture();
            overlayTexture(*textureRevealed);
        }else{
            pausedState = false;
            sprite.setTexture(*currentTexture);
        }
    }

    void toggleFlag() {
//        if (!isRevealed) {
//            cout << "Toggling flag\n";
//            isFlagged = !isFlagged;
//            sprite.setTexture(isFlagged ? *textureFlag : *textureHidden);
//        }

        if(isFlagged){
            isFlagged = false;
            sprite.setTexture(*textureHidden);
            setCurrentTexture(*textureHidden);
        }else{
            isFlagged = true;
            overlayTexture(*textureFlag);
            setCurrentTexture(*textureFlag);
        }

        if(onFlagToggled) {
            onFlagToggled(isFlagged);
        }

    }

    void reveal() {
        isRevealed = true;
        if (isMine) {
            sprite.setTexture(*textureMine);
            setCurrentTexture(*textureMine);
        } else if (adjacentMines > 0 && numberTextures.find(adjacentMines) != numberTextures.end()) {
            sprite.setTexture(*numberTextures[adjacentMines]);
            setCurrentTexture(*numberTextures[adjacentMines]);
        } else {
            sprite.setTexture(*textureRevealed);
            setCurrentTexture(*textureRevealed);
        }
    }

    void hide() {
        if (isRevealed && isMine) {
            sprite.setTexture(*textureHidden);
            setCurrentTexture(*textureHidden);
        }
    }

    void revealOver() {
        isRevealed = true;

        if (adjacentMines > 0 && numberTextures.find(adjacentMines) != numberTextures.end()) {
            sprite.setTexture(*numberTextures[adjacentMines]);
            setCurrentTexture(*numberTextures[adjacentMines]);

        } else {
            sprite.setTexture(*textureRevealed);
            setCurrentTexture(*textureRevealed);
        }

        // check if flagged and mine
        if (isFlagged && !isMine) {
            // lay flag over the mine
            sprite.setTexture(*textureFlag);
            setCurrentTexture(*textureFlag);
        }

        if(isMine && !isFlagged){
            sprite.setTexture(*textureMine);
            setCurrentTexture(*textureMine);
        }
    }

    void reset() {
        isMine = false;
        isRevealed = false;
        isFlagged = false;
        adjacentMines = 0;
        sprite.setTexture(*textureHidden);
        setCurrentTexture(*textureHidden);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void setMine(bool mine) {
        isMine = mine;
    }

    bool getIsMine()  {
        return isMine;
    }

    bool getIsFlagged()  {
        return isFlagged;
    }

    bool getIsRevealed()  {
        return isRevealed;
    }

    void setAdjacentMines(int count) {
        adjacentMines = count;
    }

    int getAdjacentMines()  {
        return adjacentMines;
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    bool contains(float x, float y) {
        return sprite.getGlobalBounds().contains(x, y);
    }
};