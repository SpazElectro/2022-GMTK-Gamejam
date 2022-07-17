#ifndef GLOBAL_H
#define GLOBAL_H

#include "pch.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 430;
const char* WINDOW_TITLE = "Dice Clicker";

const int DICE_SIZE = 96;

Font fonts[20] = { 0 };

Texture2D backgroundImage;
Audio* selectSound;

Upgrade upgrades[3];

long long prestiges = 0;
long long prestigePrice = 10000;

long long prestigeMultiplier = 1;

std::string formatPrice(long input)
{
    const std::vector<std::string> suffixes = {"K", "Mil", "Bil", "Tril", "Quad", "Quint"};

    if(input < 1000)
        return std::to_string (input);

    long num = input, wholePart, fracPart;

    unsigned int i;

    for (i = 0; i < suffixes.size(); i++)
    {
            wholePart = num / 1000;
            fracPart = (num / 100) % 10;

            if (wholePart < 1000)
            break;

            num = wholePart;
    }

    return std::to_string(wholePart) + "." + std::to_string(fracPart) + " " + suffixes[i];
}

static void AudioProcessEffectLPF(void *buffer, unsigned int frames)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula

    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        float l = ((float *)buffer)[i], r = ((float *)buffer)[i + 1];
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        ((float *)buffer)[i] = low[0];
        ((float *)buffer)[i + 1] = low[1];
    }
}

static float *delayBuffer = NULL;
static unsigned int delayBufferSize = 0;
static unsigned int delayReadIndex = 2;
static unsigned int delayWriteIndex = 0;

// Audio effect: delay
static void AudioProcessEffectDelay(void *buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        float leftDelay = delayBuffer[delayReadIndex++];    // ERROR: Reading buffer -> WHY??? Maybe thread related???
        float rightDelay = delayBuffer[delayReadIndex++];

        if (delayReadIndex == delayBufferSize) delayReadIndex = 0;

        ((float *)buffer)[i] = 0.5f*((float *)buffer)[i] + 0.5f*leftDelay;
        ((float *)buffer)[i + 1] = 0.5f*((float *)buffer)[i + 1] + 0.5f*rightDelay;

        delayBuffer[delayWriteIndex++] = ((float *)buffer)[i];
        delayBuffer[delayWriteIndex++] = ((float *)buffer)[i + 1];
        if (delayWriteIndex == delayBufferSize) delayWriteIndex = 0;
    }
}

#endif