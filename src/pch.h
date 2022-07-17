#include <iostream>
#include <string>
#include "global.h"

extern "C"
{
#include "raylib.h"
}

#ifndef PCH_H
#define PCH_H

// im so sorry for 2 variables with the same name forgive me
const int DICE_SIZE_PCH = 96;

double cash = 0;
double diceNumber = 1;
double diceGain = 1;
double diceLuck = 1;
double diceConstantNumber = 0;

Texture2D diceTextures[6];

Vector2 quickVec(int x, int y) {
    Vector2 pos = Vector2();
    pos.x = x;
    pos.y = y;

    return pos;
}

Rectangle quickRect(Vector2 position, Vector2 scale) {
    Rectangle rect;
    rect.x = position.x; rect.y = position.y;
    rect.width = scale.x; rect.height = scale.y;

    return rect;
}

Texture2D getTextureFromPath(const char* path) {
    Image image = LoadImage(path);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

Texture2D getResizedTextureFromPath(const char* path, int newWidth, int newHeight) {
    Image image = LoadImage(path);

    ImageResize((&image), newWidth, newHeight);

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

void prepareDiceTextures() {
    for(int i = 0; i < 6; i++) {
        diceTextures[i] = getResizedTextureFromPath(
            ("resources/dice/" + std::to_string(i + 1) + ".png").c_str(),
            DICE_SIZE_PCH, DICE_SIZE_PCH
        );

        SetTextureFilter(diceTextures[i], TEXTURE_FILTER_POINT);
    }
}

Texture2D getDiceTexture(int num) {
    return diceTextures[num - 1];
}

class Audio {
public:
    int randomRange;
    Sound sounds[50];

    Audio(std::string path, int randomRange) {        
        for (int i = 0; i < randomRange; i++)
        {
            this->sounds[i] = LoadSound((path + std::to_string(i) + ".wav").c_str());
        }
        
        this->randomRange = randomRange - 1;
    }

    void Play() {
        PlaySound(this->sounds[GetRandomValue(0, this->randomRange)]);
    }

    void Stop() {
        StopSound(this->sounds[GetRandomValue(0, this->randomRange)]);
    }

    void PlayMulti() {
        int randval = GetRandomValue(0, this->randomRange);

        PlaySoundMulti(this->sounds[randval]);
    }

    void StopMulti() {
        StopSoundMulti();
    }
};

class Upgrade {
public:
    const char* name;
    
    int price;
    int level;
    int levels;

    Upgrade() {}

    Upgrade(const char* name, int price, int levels) {
        this->name = name;
        this->price = price;
        this->levels = levels;
    }

    void onPurchase() {
        if(this->name == "Multiplier") {
            diceGain *= 2;
        } else if(this->name == "Luck") {
            diceLuck += 0.6;
        } else if(this->name == "Constant Dice Multiplier") {
            diceConstantNumber += 1;
        }

        this->price *= 2;
    }
};

#endif