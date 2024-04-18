#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>

using namespace std;
class SpriteManager {
private:
    map<string, sf::Sprite> sprites;
    map<string, sf::Texture> textures;

public:
    SpriteManager() {
        loadSprites();
    }

    void loadSprites() {
        vector<string> names = {"mine", "tile_hidden", "tile_revealed", "flag", "digits",
                                "face_happy", "face_win", "face_lose", "debug", "pause", "play", "leaderboard",
                                "number_1", "number_2", "number_3", "number_4", "number_5", "number_6", "number_7", "number_8"};
        for (const auto& name : names) {
            sf::Texture texture;
            string path = "files/images/" + name + ".png";
            if (texture.loadFromFile(path)) {
                textures[name] = texture;
                sf::Sprite sprite;
                sprite.setTexture(textures[name]);
                sprites[name] = sprite;
            } else {
                cerr << "Failed to load texture from " << path << endl;
            }
        }
    }

    sf::Sprite& getSprite(const string& name) {
        return sprites[name];
    }

    sf::Texture& getTexture(const string& name) {
        return textures[name];
    }

    // New method to get digit sprites as a map
    map<int, sf::Sprite> getDigitSprites() {
        map<int, sf::Sprite> digitSprites;
        sf::Texture& digitsTexture = textures["digits"];
        for (int i = 0; i < 11; ++i) {
            sf::Sprite sprite;
            sprite.setTexture(digitsTexture);
            sprite.setTextureRect(sf::IntRect(i * 21, 0, 21, 32));
            digitSprites[i] = sprite;
        }

        return digitSprites;
    }
};