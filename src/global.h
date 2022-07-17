#ifndef GLOBAL_H
#define GLOBAL_H

#include "pch.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 430;
const char* WINDOW_TITLE = "Dice Clicker";

Font fonts[20] = { 0 };

Texture2D backgroundImage;
Audio* selectSound;

Upgrade upgrades[3];

int prestiges = 0;
int prestigePrice = 10000;

double prestigeMultiplier = 1;

#endif