#include "global.h"
#include "Timer/Timer.h"
#include <math.h>
#include <vector>
#include <thread>
#include <chrono>

float dicePositionY = 0;
Timer* mainDiceTimer;

float diceTimer = 0.0f;

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

typedef struct
{
  int luck;
  int max;
  bool isGolden;
} Dice_t;

long rollDice(Dice_t *dice, long gain, long prestigeMult)
{
    int rolled = GetRandomValue(dice->luck, dice->max);

    mainDiceTimer = new Timer();
	dicePositionY=53;

    diceNumber = (dice->isGolden ? 6 : rolled);

    return (dice->isGolden ? 6 : rolled) * gain * prestigeMult;
}

Color getUpgradeColor(Upgrade* upgrade) {
    if(upgrade->level == upgrade->levels) return YELLOW;

    if(upgrade->price <= cash) {
        return GREEN;
    } else {
        return RED;
    }
}

void drawUpgrades() {
    int posX = 160;
    int posY = 45;
    int namePosX = posX;

    for(int i = 0; i < 3; i++) {
        posX += 100;
        namePosX += 100;

        DrawRectangle(posX, posY, 50, 50, WHITE);
        DrawRectangleLines(posX, posY, 50, 50, BLACK);
        DrawText("Upgrade", posX + 6.25, posY + 12.5, 10, BLACK);

        if(upgrades[i].name == "Multiplier") {
            namePosX -= 20;
        } else if(upgrades[i].name == "Constant Dice Multiplier") {
            namePosX -= 10;
        }

        DrawTextEx(
            fonts[0],
            upgrades[i].name,
            quickVec(namePosX + 15, posY - 30), 15, 1, getUpgradeColor(&upgrades[i])
        );

        DrawTextEx(
            fonts[0],
            ("$" + formatPrice(upgrades[i].price)).c_str(),
            quickVec(posX + 10, posY + 75), 20, 1, WHITE
        );

        DrawTextEx(
            fonts[0],
            (
                (std::to_string(upgrades[i].level)
                + "/" +
                std::to_string(upgrades[i].levels))
            ).c_str(),
            quickVec(posX + 10, posY + 50), 20, 1, WHITE
        );
    }
}

void buyUpgrade(Upgrade* item) {
    if(item->level < item->levels) {        
        if (cash >= item->price) {
            std::cout << "upgraded " << item->name << std::endl;

            item->level++;
            cash -= item->price;
            item->onPurchase();

            std::cout << "bought" << std::endl;
        } else {
            std::cout << "not enough cash" << std::endl;
        }
    } else {
        std::cout << "max level" << std::endl;
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    InitAudioDevice();

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    
    // essentials

    Dice_t dice = {0, 6, false};
    
    // music code is NOT mine
    Music music = LoadMusicStream("resources/music.mp3");
    music.looping = true;

    // Allocate buffer for the delay effect
    delayBufferSize = 48000 * 2;
    delayBuffer = (float *)RL_CALLOC(delayBufferSize, sizeof(float));   // 1 second delay (device sampleRate*channels)

    PlayMusicStream(music);

    backgroundImage = getResizedTextureFromPath("resources/background.png", WINDOW_WIDTH, WINDOW_HEIGHT);
    selectSound = new Audio("sounds/dice", 4);

    Texture2D background = LoadTexture("resources/cyberpunk_street_background.png");
    Texture2D midground = LoadTexture("resources/cyberpunk_street_midground.png");
    Texture2D foreground = LoadTexture("resources/cyberpunk_street_foreground.png");
    Texture2D goldenDice = getResizedTextureFromPath("resources/dice/golden.png", DICE_SIZE, DICE_SIZE);
    SetTextureFilter(goldenDice, TEXTURE_FILTER_POINT);

    float scrollingBack = 0;
    float scrollingMid = 0;
    float scrollingFore = 0;

    fonts[0] = LoadFont("fonts/nokiafc22.ttf");
    fonts[1] = LoadFont("fonts/monsterrat.ttf");
    fonts[2] = LoadFont("fonts/arial.ttf");

    upgrades[0] = *(new Upgrade("Multiplier", 500, 3));
    upgrades[1] = *(new Upgrade("Luck", 800, 5));
    upgrades[2] = *(new Upgrade("Constant Dice Multiplier", 3000, 6));

    upgrades[0].level = 0;
    upgrades[1].level = 0;
    upgrades[2].level = 0;

    prepareDiceTextures();
    
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    bool musicPaused = false;
    bool inMenu = true;

    while(!WindowShouldClose()) {
        UpdateMusicStream(music);

        scrollingBack -= 0.1f * GetFrameTime() * 1000;
        scrollingMid -= 0.5f * GetFrameTime() * 1000;
        scrollingFore -= 1.0f * GetFrameTime() * 1000;

        if (scrollingBack <= -background.width*2) scrollingBack = 0;
        if (scrollingMid <= -midground.width*2) scrollingMid = 0;
        if (scrollingFore <= -foreground.width*2) scrollingFore = 0;

        if(IsKeyPressed(KEY_SPACE)) {
            inMenu = false;
            break;
        }

        BeginDrawing();
            ClearBackground(GRAY);

            DrawTextureEx(background, quickVec(scrollingBack, 0), 0, 2, WHITE);
            DrawTextureEx(background, quickVec(background.width * 2 + scrollingBack, 0), 0, 2, WHITE);

            // Draw midground image twice
            DrawTextureEx(midground, quickVec(scrollingMid, 0), 0, 2, WHITE);
            DrawTextureEx(midground, quickVec(midground.width * 2 + scrollingMid, 0), 0, 2, WHITE);

            // Draw foreground image twice
            DrawTextureEx(foreground, quickVec(scrollingFore, 50), 0, 2, WHITE);
            DrawTextureEx(foreground, quickVec(foreground.width * 2 + scrollingFore, 50), 0, 2, WHITE);

            DrawFPS(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 20);
            DrawText("Dice Clicker", WINDOW_WIDTH / 2 - MeasureText("Dice Clicker", 30) / 2, 20, 30, WHITE);
            DrawText("Press Space to start", WINDOW_WIDTH / 2 - MeasureText("Press Space to start", 20) / 2, WINDOW_HEIGHT - 40, 20, WHITE);
        EndDrawing();
		
    }

    if(inMenu) {
        UnloadTexture(backgroundImage);
        UnloadTexture(background);
        UnloadTexture(midground);
        UnloadTexture(foreground);
        UnloadFont(fonts[0]);
        UnloadFont(fonts[1]);
        UnloadFont(fonts[2]);
        UnloadSound(selectSound->sounds[0]);
        UnloadSound(selectSound->sounds[1]);
        UnloadSound(selectSound->sounds[2]);
        UnloadSound(selectSound->sounds[3]);
        UnloadMusicStream(music);

        CloseAudioDevice();
        CloseWindow();

        RL_FREE(delayBuffer);
        
        return 0;
    }

    while(!WindowShouldClose()) {
        if(mainDiceTimer) {
            if(mainDiceTimer->ElapsedMillisecs() > 100 and mainDiceTimer->ElapsedMillisecs() < 200) {
                dicePositionY = -53;
            }

            if(mainDiceTimer->ElapsedMillisecs() > 200) {
                dicePositionY = 0;
                mainDiceTimer = nullptr;
            }
        }

        dice.luck = diceLuck;
        
        UpdateMusicStream(music);

        if(IsKeyPressed(KEY_M)) {
            if(!musicPaused) {
                PauseMusicStream(music);
            } else {
                ResumeMusicStream(music);
            }

            musicPaused = !musicPaused;
        }

        scrollingBack -= 0.1f * GetFrameTime() * 1000;
        scrollingMid -= 0.5f * GetFrameTime() * 1000;
        scrollingFore -= 1.0f * GetFrameTime() * 1000;

        if (scrollingBack <= -background.width*2) scrollingBack = 0;
        if (scrollingMid <= -midground.width*2) scrollingMid = 0;
        if (scrollingFore <= -foreground.width*2) scrollingFore = 0;

        for(int i = 0; i < 3; i++) {
            if((upgrades[i].level > upgrades[i].levels) || (upgrades[i].level < 0)) {
                upgrades[i].level = 0;

                printf("Executed/killed bad value\n");
            }
        }

        if(IsWindowFocused()) {
            if(IsMouseButtonPressed(0)) {
                if(CheckCollisionRecs(
                    quickRect(
                        GetMousePosition(), quickVec(25, 25)
                    ),
                    quickRect(
                        quickVec(650, 340), quickVec(100, 50)
                    )
                )) {
                    if(cash >= prestigePrice) {
                        cash = 0;
                        diceConstantNumber = 0;
                        diceLuck = 0;
                        prestigeMultiplier += 0.3;
                        prestigePrice *= 2;
                        prestiges += 1;

                        upgrades[0].level = 0;
                        upgrades[1].level = 0;
                        upgrades[2].level = 0;
                    } else {
                        printf("Not enough cash \n");
                    }
                } else {
                    Upgrade* item = nullptr;
                    bool foundUpgrade = false;

                    int posX = 160;
                    int posY = 45;

                    for(int i = 0; i < 3; i++) {
                        posX += 100;

                        if(CheckCollisionRecs(
                            quickRect(
                                GetMousePosition(), quickVec(50, 50)
                            ),
                            quickRect(
                                quickVec(posX, posY), quickVec(50, 50)
                            )
                        )) {
                            item = &upgrades[i];
                            foundUpgrade = true;
                            break;
                        }
                    }
                    
                    selectSound->PlayMulti();

                    if(foundUpgrade) {
                        printf("upgrade found %s\n", item->name);
                        buyUpgrade(item);
                    } else {
                        cash += rollDice(&(dice), diceGain, prestigeMultiplier);
                        dice.isGolden = GetRandomValue(0, 1);
                    }
                }
            }
        }
        
        if(GetMusicTimePlayed(music) >= GetMusicTimeLength(music)) {
            SeekMusicStream(music, 0);
        }
        
        if(dicePositionY > 0) {
            dicePositionY -= GetFrameTime()/10.0f;
        }
		
        BeginDrawing();
            ClearBackground(GRAY);

            DrawTextureEx(background, quickVec(scrollingBack, 0), 0, 2, WHITE);
            DrawTextureEx(background, quickVec(background.width * 2 + scrollingBack, 0), 0, 2, WHITE);

            // Draw midground image twice
            DrawTextureEx(midground, quickVec(scrollingMid, 0), 0, 2, WHITE);
            DrawTextureEx(midground, quickVec(midground.width * 2 + scrollingMid, 0), 0, 2, WHITE);

            // Draw foreground image twice
            DrawTextureEx(foreground, quickVec(scrollingFore, 50), 0, 2, WHITE);
            DrawTextureEx(foreground, quickVec(foreground.width * 2 + scrollingFore, 50), 0, 2, WHITE);

            DrawTexture(backgroundImage, 0, 0, WHITE);
            DrawFPS(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 20);

            if(dice.isGolden == false) {
                DrawTexture(getDiceTexture(diceNumber), WINDOW_WIDTH / 2 - DICE_SIZE + 32, WINDOW_HEIGHT / 2 - DICE_SIZE + 50 - dicePositionY, WHITE);
            } else {
                diceNumber = 6;

                DrawTexture(goldenDice, WINDOW_WIDTH / 2 - DICE_SIZE + 32, WINDOW_HEIGHT / 2 - DICE_SIZE + 50 - dicePositionY, WHITE);
            }

            DrawTextEx(fonts[0], formatPrice(cash).c_str(), quickVec(160, 325), 34, 1, BLACK);

            DrawRectangle(600, 320, 200, 50, WHITE);
            DrawRectangleLines(600, 320, 200, 50, BLACK);
            DrawTextEx(
                fonts[0],
                ("Prestige ($" + formatPrice(prestigePrice) + ")").c_str(),
                quickVec(600+12.5, 320+20), 15, 1, BLACK
            );

            DrawTextEx(
                fonts[0],
                ("Prestiges: " + std::to_string(prestiges)).c_str(),
                quickVec(50, 50), 15, 1, WHITE
            );

            drawUpgrades();

        EndDrawing();
    }

    UnloadTexture(backgroundImage);
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadFont(fonts[0]);
    UnloadFont(fonts[1]);
    UnloadFont(fonts[2]);
    UnloadSound(selectSound->sounds[0]);
    UnloadSound(selectSound->sounds[1]);
    UnloadSound(selectSound->sounds[2]);
    UnloadSound(selectSound->sounds[3]);
    UnloadMusicStream(music);

    CloseAudioDevice();
    CloseWindow();

    RL_FREE(delayBuffer);

    return 0;
}