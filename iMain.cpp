#include "iGraphics.h"
#include<stdio.h>
#include<math.h>
#include<stdbool.h>
#include "iSound.h"  // if iGraphics has it separately or use proper setup
#include <windows.h>
#include <mmsystem.h>
#include <fstream>   // For file input/output
#include <string>    // For string manipulation
#include <vector>    // For dynamic array of scores
#include <algorithm> // For sorting
#include <iomanip>   // For output formatting
Image bgPlayingImage;
#pragma comment(lib, "winmm.lib")

bool musicPlaying = false;
bool musicEnabled = true;
wchar_t backgroundMusic[] = L"music/game_music.wav"; // Place your audio file in a "music" folder

// --- Game State Management ---
enum GameState {
    STATE_ADVENTURE_SPLASH,
    STATE_TITLE,
    STATE_LEVEL_SELECT,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_HELP_ADVENTURE,
    STATE_LEVEL_COMPLETED,
    STATE_QUIT_CONFIRM,
    STATE_GAME_OVER,
    STATE_ENTER_NAME,      // New state for entering player name
    STATE_LEADERBOARD,     // New state for displaying leaderboard
    STATE_GAME_SUMMARY     // NEW: State to show game summary after score entry
};
int gameState = STATE_ADVENTURE_SPLASH; // The game will now start in the menu
int ballLives = 2;

// --- Player Score and Leaderboard ---
struct PlayerScore {
    char name[50]; // Fixed size for simplicity
    int score;

    // Operator for sorting (descending by score)
    bool operator<(const PlayerScore& other) const {
        return score > other.score;
    }
};

std::vector<PlayerScore> leaderboard;
char playerNameInput[50];
int playerNameIndex = 0;
int currentScore = 0; // Score for the current game
bool gameEndedForScoreEntry = false; // NEW: Flag to know if we entered name screen from game end

// --- Screen and Button Dimensions ---
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 563
#define BUTTON_WIDTH 300
#define BUTTON_HEIGHT 100
#define SPLASH_START_BUTTON_WIDTH 200
#define SPLASH_START_BUTTON_HEIGHT 80
#define SPLASH_START_BUTTON_X (SCREEN_WIDTH / 2 - SPLASH_START_BUTTON_WIDTH / 2)
#define SPLASH_START_BUTTON_Y 30 // Adjusted lower position for START button
#define MENU_BUTTON_WIDTH 250
#define MENU_BUTTON_HEIGHT 70

#define PLAY_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)
#define PLAY_BUTTON_Y (SCREEN_HEIGHT / 2 + 50)

#define HELP_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)
#define HELP_BUTTON_Y (SCREEN_HEIGHT / 2 - 50)

#define EXIT_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)
#define EXIT_BUTTON_Y (SCREEN_HEIGHT / 2 - 150)

#define LEADERBOARD_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2) // New button for leaderboard
#define LEADERBOARD_BUTTON_Y (SCREEN_HEIGHT / 2 - 250) // Position it below EXIT

#define SLOPE_LEFT 10   // Slope from top-left to bottom-right
#define SLOPE_RIGHT 11  // Slope from bottom-left to top-right
#define SLOPE_RIGHT_FLIPPED 12
#define SLOPE_LEFT_FLIPPED 13
#define SPECIAL_BLOCK_RED 14
#define SPECIAL_BLOCK_GREEN 15
#define SPECIAL_BLOCK_BLUE 16
#define SPECIAL_BLOCK_YELLOW 17
#define RING1 4 //vertical ring
#define RING2 8  //horizontal ring
#define BUTTON 5
#define GATE 6
#define FINISH_POINT 7
#define EMPTY 0
#define BRICK 1
#define SPIKE 2
#define SPINNER 3
#define TILE_SIZE 150
#define CHARACTER_TILE 9
#define ROWS 60
#define COLS 320  // long horizontal worldint ballX = 60;
float tileHeight = 60;
float tileWidth = 80;
float ballX;
float ballY;// the starting position of the ball.
int ballWidth = 70, ballHeight =72;
float gravity = 0.2; // the ball have to come to the grounf after jumping
float vy = 0;    // vertical speed
float speedX = 0; // horizontal speed
bool onGround = false;
int platformX = 0, platformY = 50;
int platformwidth = 800, platformHeight = 20;
float accelerationX = 4; // Rate at which speed changes
float frictionX = 0.92;    // Rate at which speed decreases when no key is pressed
float jumpStrength = 15.0;
int cameraX = 0;
int cameraY = 0;
float startX = 0;
float startY = 0;
float groundY = 50;
int numActualTiles;
#define TOTAL_FRAMES 24
char ballImageNames[TOTAL_FRAMES][30];
int rotationFrame = 0;
#define ID_BRICK 1
#define MAX_LEVEL_TILES 100000
const float BASE_TILE_WIDTH = 80.0f;
const float BASE_TILE_HEIGHT = 60.0f;
const float LEVEL_START_X = 0.0f;
const float LEVEL_START_Y = 0.0f;
int NUM_FLOOR_TILES ;
#define PAUSE_BUTTON_X (SCREEN_WIDTH - 80)  // 80 pixels from the right edge
#define PAUSE_BUTTON_Y (SCREEN_HEIGHT - 80)  // 80 pixels from the top edge
#define PAUSE_BUTTON_SIZE 60
#define RED_FINISH_POINT 18

// NEW FUNCTION: Draws the initial splash screen
void drawAdventureSplashScreen() {
    // Background image for the splash screen
    // Make sure you have 'images/splash_adventure.png' in your project!
    iShowImage(0, 0, "images/front.png"); // You might change this to "images/landingpage.png" if you prefer

    // START Button
    iSetColor(0, 150, 0); // Green color for the button
    iFilledRectangle(SPLASH_START_BUTTON_X, SPLASH_START_BUTTON_Y, SPLASH_START_BUTTON_WIDTH, SPLASH_START_BUTTON_HEIGHT);
    iSetColor(255, 255, 255); // White text
    iText(SPLASH_START_BUTTON_X + 65, SPLASH_START_BUTTON_Y + 30, "START", GLUT_BITMAP_TIMES_ROMAN_24);
}            // A 60x60 pixel button
int currentLevel = 1; // Starts with level 1
bool levelCompleted = false; // Optional flag for level switching
struct Tile {
    int id;
    float x, y;
    float width, height;
    bool isSolid;
    bool hitByBall;
};

struct Tile levelTiles[MAX_LEVEL_TILES];
// Function to count the total rings in the current level
int ringsCollected = 0;
int totalRingsInLevel = 0;
void countTotalRings() {
    totalRingsInLevel = 0;
    for (int i = 0; i < numActualTiles; i++) {
        if (levelTiles[i].id == RING1 || levelTiles[i].id == RING2) {
            totalRingsInLevel++;
        }
    }
}
// ... (previous drawing functions like drawLevelSelect, drawSpinner, etc.) ...

// NEW FUNCTION: Draws a simple help screen

// ... (iDraw() function follows) ...
// Draws the quit confirmation dialog over the main menu

// Draws the quit confirmation dialog over the main menu

void drawGameOverScreen() {
    iSetColor(0, 0, 0); // Black background
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    iSetColor(255, 0, 0); // Red text
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    iSetColor(255, 255, 255); // White text
    iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, "You ran out of lives!", GLUT_BITMAP_HELVETICA_18);

    // Return to Main Menu Button
    iSetColor(100, 100, 100); // Gray
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, 100, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, 125, "MAIN MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}
// --- END ADDITION ---

// NEW FUNCTION: Draws the Level Completed screen
void drawLevelCompletedScreen() {
    iSetColor(0, 150, 200); // Blue background
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png"); // Using a consistent background

    iSetColor(255, 255, 255); // White text
    iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 150, "LEVEL COMPLETED!", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Button Positions ---
    float buttonX = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
    float nextLevelY = SCREEN_HEIGHT / 2 + 20;
    float restartY = SCREEN_HEIGHT / 2 - 100;
    float mainMenuY = SCREEN_HEIGHT / 2 - 220;

    // --- Draw Next Level Button ---
    iSetColor(0, 180, 50); // Green
    iFilledRectangle(buttonX, nextLevelY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 70, nextLevelY + 25, "NEXT LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw Restart Button ---
    iSetColor(200, 150, 0); // Yellow/Orange
    iFilledRectangle(buttonX, restartY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 80, restartY + 25, "RESTART", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw Main Menu Button ---
    iSetColor(180, 50, 0); // Red
    iFilledRectangle(buttonX, mainMenuY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 70, mainMenuY + 25, "MAIN MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}

void initializeLevel1() {
    ballX = 100.0f; // A safe distance from the 1-tile thick wall
    ballY = BASE_TILE_HEIGHT + 1; // Just above the floor
    speedX = 0;
    vy = 0;
    NUM_FLOOR_TILES = 110;
    // Create the 100 base floor tiles
    numActualTiles = 0;

    // --- Reset Ring Score for the Level ---
    ringsCollected = 0;
    for (int i = 0; i < NUM_FLOOR_TILES; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    // Add top roof tiles
    for (int i = 0; i < NUM_FLOOR_TILES; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = 9*BASE_TILE_HEIGHT; // Top of screen (adjust as needed)
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    // Add left vertical wall
    for (int i = 0; i < 10; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = 0;
            levelTiles[numActualTiles].y = LEVEL_START_Y + i * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    // Add right vertical wall
    // Add right vertical wall
        float rightEdgeX = NUM_FLOOR_TILES * BASE_TILE_WIDTH - BASE_TILE_WIDTH;
    for (int i = 0; i < 10; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK; // Default to a normal brick

            // The red finish line is now at the very bottom (ground level).
            if (i == 0 || i == 1) {
                levelTiles[numActualTiles].id = RED_FINISH_POINT;
            }

            levelTiles[numActualTiles].x = rightEdgeX;
            levelTiles[numActualTiles].y = LEVEL_START_Y + i * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true; 
            levelTiles[numActualTiles].hitByBall = false; // Initialize to not hit
            numActualTiles++;
        }
    }
    // Add tiles from column 4 to 9 and row 4 to top
    for (int i = 4; i <= 9; i++) {
        for (int j = 4; j < 10; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 13; i <= 14; i++) {
        for (int j = 0; j <=5; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 19; i <= 21; i++) {
        for (int j = 3; j <10; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 24; i <= 27; i++) {
        for (int j = 0; j <=4; j++)
            if(j==4 || (j<=3 && (i==24 || i==25)))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    for (int i = 30; i <= 33; i++) {
        for (int j = 3; j <= 4; j++) {
            // Full row at j = 3 and top at j = 4
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }
    // Middle support brick
    for (int i = 31; i <= 32; i++) {
        levelTiles[numActualTiles].id = ID_BRICK;
        levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y = LEVEL_START_Y + 2 * BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = true;
        numActualTiles++;
    }

    for (int i = 36; i <= 39; i++) {
        for (int j = 0; j <=4; j++)
            if(j==4 || (j<=3 && (i==38 || i==39)))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }


    for (int i = 42; i <= 44; i++) {
        for (int j = 4; j <9; j++)
            if(j==9-1 || (j<=9-2 && (i==42 || i==43)))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }


    for (int i = 64; i <= 66; i++) {
        for (int j = 4; j <9; j++)
            if(j==9-1 || (j<=9-2 && (i==65 || i==66)))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    for (int i =69; i <= 70; i++) {
        for (int j = 0; j < 6; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 73; i <= 74; i++) {
        for (int j = 4; j < 10; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i =77; i <= 78; i++) {
        for (int j = 0; j < 6; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 81; i <= 82; i++) {
        for (int j = 4; j < 10; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i =85; i <= 86; i++) {
        for (int j = 0; j < 6; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


    for (int i =91; i <= 92; i++) {
        for (int j = 0; j < 3; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }
    for (int j = 0; j < 2; j++) {
        levelTiles[numActualTiles].id = ID_BRICK;
        levelTiles[numActualTiles].x = LEVEL_START_X + 93 * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = true;
        numActualTiles++;
    }



    for (int i =94; i <= 95; i++) {
        for (int j = 0; j < 4; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }

    for (int i = 97; i <= 98; i++) {
        for (int j = 4; j < 10; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }


    for (int i =100; i <= 101; i++) {
        for (int j = 0; j < 4; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }
    for (int j = 0; j < 2; j++) {
        levelTiles[numActualTiles].id = ID_BRICK;
        levelTiles[numActualTiles].x = LEVEL_START_X + 102 * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = true;
        numActualTiles++;
    }



    for (int i =103; i <=104; i++) {
        for (int j = 0; j < 3; j++) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }

    // drawing rings

    levelTiles[numActualTiles].id = RING1;
    levelTiles[numActualTiles].x = 7 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y =30+ BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;


    levelTiles[numActualTiles].id = RING1;
    levelTiles[numActualTiles].x = 31 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 5 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = RING1;
    levelTiles[numActualTiles].x = 73 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = RING1;
    levelTiles[numActualTiles].x = 81* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    //horizontal rings

    levelTiles[numActualTiles].id = RING2;
    levelTiles[numActualTiles].x = 96* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = 3*BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = RING2;
    levelTiles[numActualTiles].x = 99* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = 3*BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;



    //drawing spikes

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 16* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 28* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 34* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;


    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x =49* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 52* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 56* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 59* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;


    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 93* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y =2*BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;

    levelTiles[numActualTiles].id = SPIKE;
    levelTiles[numActualTiles].x = 102* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = 2*BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;
    numActualTiles++;
    countTotalRings();
}
void initializeLevel2(){
    numActualTiles = 0;
    ballX = 210.0f; // The wall ends at x=200, so 250 is a safe start
    ballY = BASE_TILE_HEIGHT ; // Just above the floor
    speedX = 0;
    vy = 0;
    NUM_FLOOR_TILES = 134;
    // -------- LEVEL 2 FLOOR --------
    for (int i = 0; i < NUM_FLOOR_TILES; i++) {
        levelTiles[numActualTiles].id     = ID_BRICK;
        levelTiles[numActualTiles].x      = LEVEL_START_X + i * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y      = LEVEL_START_Y;        // same Y as Level 1 floor
        levelTiles[numActualTiles].width  = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = true;
        numActualTiles++;
    }
    for (int i = 0; i < ROWS; i++) {
        // The new inner loop creates the thickness of the wall (X position)
        for (int j = 0; j < 2; j++) { // This loop runs twice for a 2-tile wide wall
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + j * BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + i * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }
    // --- Creates a 2-tile thick right wall ---
  float rightEdgeX = (NUM_FLOOR_TILES - 4) * BASE_TILE_WIDTH;

    // The outer loop controls the height of the wall (Y position, 'j')
    for (int j = 0; j < ROWS + 1; j++) {
        // The inner loop creates the 2-tile thickness (X position)
        for (int i = 0; i < 2; i++) {
            if (numActualTiles < MAX_LEVEL_TILES) {
                
                // Set the 22nd and 23rd vertical bricks to be the finish point
                if (j == 20 || j == 21) {
                    levelTiles[numActualTiles].id = RED_FINISH_POINT;
                } else {
                    levelTiles[numActualTiles].id = ID_BRICK;
                }
                
                levelTiles[numActualTiles].x = rightEdgeX - (i * BASE_TILE_WIDTH);
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 0; i < NUM_FLOOR_TILES - 7; i++) {
        levelTiles[numActualTiles].id = ID_BRICK;
        levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH; // X increases
        levelTiles[numActualTiles].y = 540; // A fixed Y position, same as Level 1's ceiling
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = true;
        numActualTiles++;
    }
    for (int i = 12; i <= 13; i++) {
        for (int j = 0; j < 6; j++) { // j=0 to 7 is 8 tiles
            if (numActualTiles < MAX_LEVEL_TILES) {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 14; i <= 19; i++) {
        // The structure is now 4 steps lower, from row 2 to 5.
        for (int j = 2; j <= 5; j++) {
            // The top bar is now at j = 5 to keep the L-shape.
            if (j == 5 || i == 14 || i == 18 || i == 19) {

                if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                    levelTiles[numActualTiles].id = ID_BRICK;
                    levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].isSolid = true;
                    numActualTiles++;
                }
            }
        }
    }
    for (int i = 22; i <= 23; i++) {
        for (int j = 4; j <= 8; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 26; i <= 27; i++) {
        for (int j = 1; j <= 5; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 30; i <= 31; i++) {
        for (int j = 6; j <= 9; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 37; i <= 41; i++) {
        for (int j = 1; j <= 5; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 61; i <= 62; i++) {
        for (int j = 4; j <= 8; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    const int pyramidHeight = 6;      // Height of the pyramid (10 rows)
    const int centerCol = 72;          // Center column for symmetry

    for (int row = 0; row < pyramidHeight; row++) {
        int tilesInRow = 2 * row + 1;              // Number of tiles in this row (1, 3, 5, ..., 19)
        int startCol = centerCol - row;            // Starting column for this row (shifts left each row)

        for (int col = 0; col < tilesInRow; col++) {
            if (numActualTiles < MAX_LEVEL_TILES) {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + (startCol + col) * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + (pyramidHeight - row - 1) * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    //     for (int i = 67; i <= 68; i++) {
    //     for (int j = 4; j <= 11; j++) {

    //         if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
    //             levelTiles[numActualTiles].id = ID_BRICK;
    //             levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
    //             levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
    //             levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    //             levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    //             levelTiles[numActualTiles].isSolid = true;
    //             numActualTiles++;
    //         }
    //     }
    // }
    for (int i = 85; i <= 86; i++) {
        for (int j = 4; j <= 11; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 96; i <= 97; i++) {
        for (int j = 6; j <= 9; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 96; i <= 97; i++) {
        for (int j = 1; j <= 3; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 107; i <= 108; i++) {
        for (int j = 1; j <= 5; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 111; i <= 112; i++) {
        for (int j = 4; j <= 8; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 115; i <= 118; i++) {
        for (int j = 4; j <= 4; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 117; i <= 118; i++) {
        for (int j = 1; j <= 5; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 122; i <= 123; i++) {
        for (int j = 5; j <= 8; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 126; i <= 128; i++) {
        for (int j = 5; j <= 5; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    // i have to edit this part now.
    // Creates a 5-step staircase with each step 2 tiles wide and 2 tiles tall
    for (int step = 0; step < 6; step++) {
        int start_i = 125 - step;   // Moves left with each step
        int start_j = 9 + step;    // Moves up with each step

        for (int i = start_i; i >= start_i - 1; i--) {         // 2 tiles wide (horizontally)
            for (int j = start_j; j <= start_j + 1; j++) {     // 2 tiles tall (vertically)
                if (numActualTiles < MAX_LEVEL_TILES) {
                    levelTiles[numActualTiles].id = ID_BRICK;
                    levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].isSolid = true;
                    numActualTiles++;
                }
            }
        }
    }
    // Creates a mirrored 8-step staircase (each step is 2×2 tiles, going left-up)
    for (int step = 0; step < 6; step++) {
        int start_i = 110 + step;   // Moves RIGHT each step (mirrored)
        int start_j = 9 + step;    // Moves UP each step

        for (int i = start_i; i <= start_i + 1; i++) {         // 2 tiles wide (rightward)
            for (int j = start_j; j <= start_j + 1; j++) {     // 2 tiles tall (upward)
                if (numActualTiles < MAX_LEVEL_TILES) {
                    levelTiles[numActualTiles].id = ID_BRICK;
                    levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                    levelTiles[numActualTiles].isSolid = true;
                    numActualTiles++;
                }
            }
        }
    }
    for (int i = 103; i <= 104; i++) {
        for (int j = 10; j <= 30; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 109; i <= 128; i++) {
        for (int j = 19; j <= 19; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 105; i <= 107; i++) {
        for (int j = 15; j <= 15; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    // for (int i = 115; i <= 135; i++) {
    //     for (int j = 22; j <= 22; j++) {

    //         if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
    //             levelTiles[numActualTiles].id = ID_BRICK;
    //             levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
    //             levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
    //             levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    //             levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    //             levelTiles[numActualTiles].isSolid = true;
    //             numActualTiles++;
    //         }
    //     }
    // }
    for (int i = 105; i <= 120; i++) {
        for (int j = 23; j <= 23; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
    for (int i = 105; i <= 128; i++) {
        for (int j = 28; j <= 29; j++) {

            if (numActualTiles < MAX_LEVEL_TILES) { // Safety check
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
        }
    }
}
void initializelevel3(){

    ballX = 54*BASE_TILE_WIDTH; // A safe distance from the 1-tile thick wall
    ballY = BASE_TILE_HEIGHT*44; // Just above the floor
    speedX = 0;
    vy = 0;
    NUM_FLOOR_TILES = 200;
    // Create the 200 base floor tiles
    numActualTiles = 0;

    // --- Reset Ring Score for the Level ---
    ringsCollected = 0;

    //base tiles
    for (int i = 0; i < 68; i++) {
        if(i==33 || i==34 || i==35 || i==36) continue;
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*30;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 0; i <2; i++) {
        // The new inner loop creates the thickness of the wall (X position)
        for (int j = 20; j < 70; j++) { // This loop runs twice for a 2-tile wide wall
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    //top roof
    for (int i = 0; i < 133; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*47;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 6; i <=7; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*35;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    for (int i = 2; i <=4; i++) {
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*39;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


    for (int i = 10; i <= 13; i++) {
        for (int j = 30; j <=33; j++)
            if((i==10 || i==11) ||(i==12 && (j==32 || j==31)) || (i==13 && j==31))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }



    // Add  left slope tiles
    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 13 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 32 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 12 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 33 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 14 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 31 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 20; i <= 25; i++) {
        for (int j = 30; j <=33; j++)
            if((i==22 || i==23 || i==24 || i==25) ||(i==21 && (j==32 || j==31)) || (i==20 && j==31))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  right slope tiles
    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 19 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 31* BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 20 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y +32 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 21 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 33 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i =10; i <= 11; i++)
    {
        for (int j = 39; j < 47; j++)
        {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }

    }

    for (int i = 12; i <= 15; i++) {
        for (int j = 39; j <=42; j++)
            if((i==12 && j<=42) ||(i==13 && j<=41 )|| (i==14  && j<=40) || (i==15 && j==39))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  left slope tiles
    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 12 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 43 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 13 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 42 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 14 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 15 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 18; i <= 23; i++) {
        for (int j = 39; j <=43; j++)
            if(((i==22 || i==23)||((i==21 && j<=42) ||(i==20 && j<=41 )|| (i==19  && j<=40) || (i==18 && j==39))))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  right slope tiles
    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 18 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 19 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 20 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y +42 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 21 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y +43 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    for (int i = 20; i < 133; i++) {
        if( i==92 || i== 93) continue;
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*39;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }
    for (int i = 44; i <= 46; i++) {
        for (int j = 39; j <=41; j++)
            if((i==45 || i==46) || (i==44 && j==40))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  right slope tiles
    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 43 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 44 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 44; i <= 46; i++) {
        for (int j = 45; j <=47; j++)
            if((i==45 || i==46) || (i==44 && j==46))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    //Add  right flipped slope tiles

    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 43 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 46 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 44 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 45 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 58; i <= 61; i++)
    {
        for (int j = 40; j <=42; j++)
            if((i==58 && j==40) || (i==59 && (j==40 || j==41)) ||((i==60 || i==61)&& j==42) )
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  right slope tiles
    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 57 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 58 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 59 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 42 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 60 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 43 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    // Add  left slope tiles
    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 61 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 43 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 68; i <= 70; i++) {
        for (int j = 39; j <=41; j++)
            if((i==68 || i==69) || (i==70 && j==40))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    // Add  left slope tiles
    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 70 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 71 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y +40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 68; i <= 70; i++) {
        for (int j =45; j <=47; j++)
            if((i==68 || i==69) || (i==70 && j==46))
            {
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

    //Add  left flipped slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 70 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 45 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_LEFT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 71 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 46 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 75; i <= 79; i++) {
        for (int j = 39; j <=41; j++)
           
            {  if((i==75 && j==41) || (i==79 && j==41)) continue;
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

  //Add  right  slope tiles

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 75 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

     levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 74 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


//Add left  slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 79 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

 levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 80 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



     for (int i = 75; i <= 79; i++) {
        for (int j = 45; j <=47; j++)
           
            {  if((i==75 && j==45) || (i==79 && j==45)) continue;
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

  //Add  right  slope tiles

    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 75 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 45 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

     levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 74 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 46 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


//Add left  slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 79 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 45 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

 levelTiles[numActualTiles].id = SLOPE_LEFT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 80 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 46 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


      for (int i = 87; i <= 90; i++) {
        for (int j = 39; j <=41; j++)
           
            {  if((i==87 && j==41) || (i==90 && j==41)) continue;
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

  //Add  right  slope tiles

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 86 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

     levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 87 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


//Add left  slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 90 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 91 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;




      for (int i = 95; i <= 110; i++) {
        for (int j = 39; j <=41; j++)
           
            {  if((i==95 && j==41) ) continue;
                levelTiles[numActualTiles].id = ID_BRICK;
                levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
                levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
                levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
                levelTiles[numActualTiles].isSolid = true;
                numActualTiles++;
            }
    }

  //Add  right  slope tiles

    levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 94 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

     levelTiles[numActualTiles].id = SLOPE_RIGHT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 95 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;




    for (int i = 131; i <133; i++) {
        // The new inner loop creates the thickness of the wall (X position)
        for (int j = 40; j < 48; j++) { // This loop runs twice for a 2-tile wide wall
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

        for (int i = 111; i <=112; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 44 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }

 for (int i = 113; i <=114; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 42 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }

 for (int i = 115; i <=116; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 44 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }

 for (int i = 118; i <=119; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 41 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }

          for (int i = 121; i <=122; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 40 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }
 for (int i = 122; i <=123; i++)
         {
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + 43 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
         }

 for (int i = 126; i <=128; i++)

         { for (int j = 41; j <=42; j++)
            { if((i==126 && j==42)|| (i== 128 && j==41)) continue;
        
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
         }

         }


    for (int i = 31; i <= 32; i++) {
        for (int j = 30; j <=38; j++)

        { if(j==33 || j==34) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }



    for (int i = 22; i <= 46; i++) {
         if((i==22 || i==23) ||(i==45 || i==46))
        for (int j = 21; j <=30; j++)
        
        { 
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    
    for (int i = 22; i <= 36; i++) {
  

       
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + 25 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
    }

    for (int i = 22; i <= 46; i++) {
  

       
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + 21 * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        
    }


    for (int i = 37; i <= 38; i++) {
        for (int j = 30; j <=38; j++)

        { if(j==33 || j==34) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


    
    for (int i = 44; i <= 47; i++) {
        for (int j = 30; j <=33; j++)

        { if((i==46 && j== 33) || (i == 47 && (j==32 || j==33))) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

  //Add left slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 46 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 33 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 47 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 32 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    
    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 48 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 31 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 62; i <= 67; i++) {
        for (int j = 34; j <=38; j++)

        { if((i==62 && (j== 34 || j==35 ||j==36 || j==37)) ||(i==63 && (j== 34 || j==35 ||j==36)) || (i ==64 && (j==34 || j==35)) ||(i==65 && j== 34) ) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    // Add  right flipped slope tiles
    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 64 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 35* BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 61 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y +38 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;



    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 62 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 37 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    
    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 63* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 36 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

    levelTiles[numActualTiles].id = SLOPE_RIGHT_FLIPPED;
    levelTiles[numActualTiles].x = LEVEL_START_X + 65* BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 34 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;

   for (int i = 66; i <= 68; i++) {
        for (int j =14 ; j <=30; j++)
        
        { if(i==68 && j== 30) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    
  //Add left slope tiles

    levelTiles[numActualTiles].id = SLOPE_LEFT;
    levelTiles[numActualTiles].x = LEVEL_START_X + 68 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 30 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  // Slope should still block the ball
    numActualTiles++;


    for (int i = 68; i < 133; i++) {
       
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*14;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


     for (int i = 74; i <= 91; i++) {
        for (int j =17 ; j <=39; j++)
        
        { 
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


    
     for (int i = 94; i <= 111; i++) {
        for (int j =17 ; j <=39; j++)
        
        { 
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


  for (int i =112; i < 133; i++) {
       
    
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*22;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

    
    for (int i = 132; i <134; i++) {
        // The new inner loop creates the thickness of the wall (X position)
        for (int j = 14; j < 23; j++) { // This loop runs twice for a 2-tile wide wall
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i* BASE_TILE_WIDTH; // x will be 0, then 100
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

for (int i =123; i < 125; i++) {
       
    
        if (numActualTiles < MAX_LEVEL_TILES) {
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*15;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }

}

void drawGameUI() {
    // --- Draw UI for Ring Count ---
    iSetColor(255, 255, 255); // White text
    char ringText[50];
    sprintf(ringText, "Rings: %d / %d", ringsCollected, totalRingsInLevel);
    iText(20, SCREEN_HEIGHT - 40, ringText, GLUT_BITMAP_TIMES_ROMAN_24);

    char livesText[50];
    sprintf(livesText, "Lives: %d", ballLives);
    iText(20, SCREEN_HEIGHT - 70, livesText, GLUT_BITMAP_TIMES_ROMAN_24);
}
// Global array to store all tiles in the level
int ctualTiles = 0; // Number of tiles currently in use
void loadLevel(int level) {
    ballLives = 2; // Reset lives when a new level is loaded or started
    ringsCollected = 0; // Ensure rings are also reset for the new level
    if (level == 1) {
        initializeLevel1();
    } else if (level == 2) {
        initializeLevel2();
    }
    else if (level == 3){
        initializelevel3();
    }
}


void drawSpinner(int x, int y) {
    int size = 60; // same as tileSize

    // Example spinner: a rotating square or a cross (you can customize)
    iSetColor(0, 0, 255); // blue color

    // Draw a square centered at (x + size/2, y + size/2)
    iFilledRectangle(x + size/4, y + size/4, size/2, size/2);

    // Or draw two crossing lines to simulate a spinner
    iSetColor(255, 255, 255); // white lines
    iLine(x, y, x + size, y + size);
    iLine(x + size, y, x, y + size);
}
/*
function iDraw() is called again and again by the system.
*/
void loadBallImageNames() {
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        sprintf(ballImageNames[i], "images/ball_%d.png", i);
    }
}

// --- Leaderboard Functions ---
void loadLeaderboard() {
    leaderboard.clear(); // Clear existing data
    std::ifstream file("leaderboard.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t commaPos = line.find(',');
            if (commaPos != std::string::npos) {
                PlayerScore ps;
                std::string nameStr = line.substr(0, commaPos);
                // Ensure name fits in char array
                strncpy(ps.name, nameStr.c_str(), sizeof(ps.name) - 1);
                ps.name[sizeof(ps.name) - 1] = '\0'; // Null-terminate

                std::string scoreStr = line.substr(commaPos + 1);
                ps.score = std::stoi(scoreStr);
                leaderboard.push_back(ps);
            }
        }
        file.close();
        std::sort(leaderboard.begin(), leaderboard.end()); // Sort after loading
    } else {
        printf("Leaderboard file not found. A new one will be created on save.\n");
    }
}

void saveLeaderboard() {
    // This creates or overwrites leaderboard.txt in the program's running directory.
    std::ofstream file("leaderboard.txt");
    if (file.is_open()) {
        // Loop through the leaderboard vector and write each entry to the file.
        for (const auto& score : leaderboard) {
            file << score.name << "," << score.score << "\n";
        }
        file.close(); // Close the file to ensure data is saved.
    } else {
        // This will print to your console if the file cannot be opened for writing.
        printf("Error: Could not open leaderboard.txt for writing.\n");
    }
}

void addScoreToLeaderboard(const char* name, int score) {
    PlayerScore newScore;
    strncpy(newScore.name, name, sizeof(newScore.name) - 1);
    newScore.name[sizeof(newScore.name) - 1] = '\0';
    newScore.score = score;

    leaderboard.push_back(newScore);
    std::sort(leaderboard.begin(), leaderboard.end()); // Re-sort after adding

    // Keep only top N scores (e.g., top 10)
    if (leaderboard.size() > 10) {
        leaderboard.resize(10);
    }
    saveLeaderboard(); // Save immediately after adding/updating
}

void drawEnterNameScreen() {
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");

    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 100, "ENTER YOUR NAME", GLUT_BITMAP_TIMES_ROMAN_24);

    // Draw input box
    iSetColor(100, 100, 100);
    iFilledRectangle(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 20, 300, 40);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 - 10, playerNameInput, GLUT_BITMAP_HELVETICA_18);

    // Draw "Submit" button
    iSetColor(0, 180, 50);
    iFilledRectangle(SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 - 100, 150, 50);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 - 85, "SUBMIT", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawLeaderboardScreen() {
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");

    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100, "LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24);

    int startY = SCREEN_HEIGHT - 150;
    int lineHeight = 30;

    char scoreEntry[100];
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        sprintf(scoreEntry, "%2d. %-20s %5d", (int)i + 1, leaderboard[i].name, leaderboard[i].score);
        iText(SCREEN_WIDTH / 2 - 200, startY - (i * lineHeight), scoreEntry, GLUT_BITMAP_HELVETICA_18);
    }

    // Back to Main Menu Button
    iSetColor(100, 100, 100); // Gray
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, 50, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, 75, "BACK TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}

// NEW FUNCTION: Draws the game summary screen
void drawGameSummaryScreen() {
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");

    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100, "GAME SUMMARY", GLUT_BITMAP_TIMES_ROMAN_24);

    char summaryText[100];
    sprintf(summaryText, "%s - Your score is: %d", playerNameInput, currentScore);
    iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 30, summaryText, GLUT_BITMAP_HELVETICA_18);

    // Leaderboard Button
    iSetColor(180, 180, 0); // Yellowish for leaderboard
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, SCREEN_HEIGHT / 2 - 50, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 25, "LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24);

    // Main Menu Button
    iSetColor(100, 100, 100); // Gray
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, SCREEN_HEIGHT / 2 - 150, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 125, "MAIN MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}


// MODIFIED FUNCTION: This now serves as the Main Menu
void drawTitleScreen() {
    // Draw a background for the main menu
    iSetColor(85, 206, 255); // A nice blue
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    iShowImage(0, 0, "images/landingpage.png");
    // Draw the main game title
    // iSetColor(150, 75, 0); // A gold-like color
    // iText(SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT - 150, "BOUNCY BALL: THE GRAND ADVENTURE", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw the "Play" Button ---
    iSetColor(0, 180, 50); // A bright green for the button
    iFilledRectangle(PLAY_BUTTON_X, PLAY_BUTTON_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255); // White text
    iText(PLAY_BUTTON_X + 95, PLAY_BUTTON_Y + 25, "PLAY", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw the "Help" Button ---
    iSetColor(0, 100, 180); // A blue for the button
    iFilledRectangle(HELP_BUTTON_X, HELP_BUTTON_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255); // White text
    iText(HELP_BUTTON_X + 95, HELP_BUTTON_Y + 25, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw the "Leaderboard" Button ---
    iSetColor(180, 180, 0); // Yellowish for leaderboard
    iFilledRectangle(LEADERBOARD_BUTTON_X, LEADERBOARD_BUTTON_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255); // White text
    iText(LEADERBOARD_BUTTON_X + 60, LEADERBOARD_BUTTON_Y + 25, "LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24);


    // --- Draw the "Exit" Button ---
    iSetColor(180, 50, 0); // A red for the button
    iFilledRectangle(EXIT_BUTTON_X, EXIT_BUTTON_Y, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255); // White text
    iText(EXIT_BUTTON_X + 95, EXIT_BUTTON_Y + 25, "EXIT", GLUT_BITMAP_TIMES_ROMAN_24);
}
// NEW FUNCTION: Draws a simple help screen
void drawHelpScreen() {
    iSetColor(85, 206, 255); // Background
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");

    iSetColor(255, 255, 255); // White text
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100, "HELP / INSTRUCTIONS", GLUT_BITMAP_TIMES_ROMAN_24);
    iText(50, SCREEN_HEIGHT - 200, "- Use 'A' to move left, 'D' to move right.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 230, "- Press 'W' or 'SPACE' to jump.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 260, "- Collect all rings to complete a level.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 290, "- Avoid spikes and other hazards.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 320, "- Press 'P' to pause/unpause the game.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 350, "- Press 'M' to return to Main Menu during gameplay.", GLUT_BITMAP_HELVETICA_18);
    iText(50, SCREEN_HEIGHT - 380, "- Press 'ESC' to exit the game.", GLUT_BITMAP_HELVETICA_18); // General iGraphics exit

    // Back to Main Menu Button
    iSetColor(100, 100, 100); // Gray
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, 50, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, 75, "BACK TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}
void drawLevelSelect() {
    // Draw a blue background for the menu
    iSetColor(85, 206, 255);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");
    // Draw the game title
    // iSetColor(255, 255, 255); // White color for text
    // iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 50, "BOUNCY BALL ADVENTURE", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Define positions for the buttons ---
    float btn1_x = 140;
    float btn1_y = 320;

    float btn2_x = 140;
    float btn2_y = 150; // 150 pixels below the first button

    float btn3_x = 550;
    float btn3_y = 320;

    float btn4_x = 550;
    float btn4_y = 150;

    // --- Draw the Level 1 Button ---
    iSetColor(150, 75, 0); // A nice green color
    iFilledRectangle(btn1_x, btn1_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(btn1_x + 120, btn1_y + 40, "Level 1", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw the Level 2 Button ---
    iSetColor(150, 75, 0); // A nice red color
    iFilledRectangle(btn2_x, btn2_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(btn2_x + 120, btn2_y + 40, "Level 2", GLUT_BITMAP_TIMES_ROMAN_24);
    // Draw the level 3 Button-----
    iSetColor(150, 75, 0); // A nice red color
    iFilledRectangle(btn3_x, btn3_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(btn3_x + 120, btn3_y + 40, "Level 3", GLUT_BITMAP_TIMES_ROMAN_24);

    iSetColor(150, 75, 0); // A nice red color
    iFilledRectangle(btn4_x, btn4_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(btn4_x + 120, btn4_y + 40, "Level 4", GLUT_BITMAP_TIMES_ROMAN_24);
}
void drawPauseMenu() {
    // Draw a semi-transparent dark rectangle over the screen
    // The '150' value is the alpha/transparency. 255 is fully opaque, 0 is invisible.
    iSetColor(40, 40, 60);
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    iShowImage(0, 0, "images/landingpage.png");

    // Draw the "Paused" text
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 90, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Define button positions for the pause menu ---
    float resume_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
    float resume_btn_y = SCREEN_HEIGHT / 2;

    float menu_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
    float menu_btn_y = SCREEN_HEIGHT / 2 - 150;

    // --- Draw the "Resume" Button ---
    iSetColor(0, 100, 180); // Green
    iFilledRectangle(resume_btn_x, resume_btn_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(resume_btn_x + 100, resume_btn_y + 40, "Resume", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Draw the "Main Menu" Button ---
    iSetColor(180, 50, 0); // Red
    iFilledRectangle(menu_btn_x, menu_btn_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(menu_btn_x + 90, menu_btn_y + 40, "Main Menu", GLUT_BITMAP_TIMES_ROMAN_24);
}
void drawQuitConfirmDialog()
{
    // First, draw the main menu in the background to create an overlay effect
    // drawTitleScreen(); // You might want to skip this if you want a fully opaque dialog

    // Draw the semi-transparent overlay (optional, creates a focus effect)
    iSetColor(0, 0, 0); // Black color
    iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // A black rectangle that covers the whole screen
    iShowImage(0, 0, "images/landingpage.png");
    // Calculate dimensions for the dialog box
    int dialogWidth = 600;  // Increased width for better centering
    int dialogHeight = 250; // Increased height
    int dialogX = (SCREEN_WIDTH / 2) - (dialogWidth / 2);
    int dialogY = (SCREEN_HEIGHT / 2) - (dialogHeight / 2);

    // Draw the dialog box itself
    iSetColor(220, 220, 220); // Light gray for the box
    iFilledRectangle(dialogX, dialogY, dialogWidth, dialogHeight);

    // Draw the border
    iSetColor(0, 0, 0);
    iRectangle(dialogX, dialogY, dialogWidth, dialogHeight);

    // Draw the confirmation question - centered within the dialog box
    // You'll need to estimate text width or use a helper function if iGraphics provides one
    // For now, let's roughly center it
    char* questionText = "Are you sure to quit?";
    int textWidth = 0; // iGraphics doesn't easily provide text width, so estimate or adjust visually
    // For GLUT_BITMAP_TIMES_ROMAN_24, a rough estimate is about 12-15 pixels per character
    textWidth = strlen(questionText) * 13; // Approximate

    iSetColor(0, 0, 0);
    iText(dialogX + (dialogWidth / 2) - (textWidth / 2), dialogY + dialogHeight - 100, questionText, GLUT_BITMAP_TIMES_ROMAN_24); // Adjusted Y for question

    // --- Draw Buttons ---
    int buttonWidth = 150; // Slightly larger buttons
    int buttonHeight = 60;
    int buttonPadding = 50; // Space between buttons

    // "YES" Button
    int yesButtonX = dialogX + (dialogWidth / 2) - buttonWidth - (buttonPadding / 2);
    int yesButtonY = dialogY + 40; // Positioned near the bottom of the dialog

    iSetColor(220, 60, 60); // Red color for "YES"
    iFilledRectangle(yesButtonX, yesButtonY, buttonWidth, buttonHeight);
    iSetColor(255, 255, 255); // White text
    iText(yesButtonX + (buttonWidth / 2) - 15, yesButtonY + (buttonHeight / 2) - 8, "YES", GLUT_BITMAP_HELVETICA_18); // Center text within button

    // "NO" Button
    int noButtonX = dialogX + (dialogWidth / 2) + (buttonPadding / 2);
    int noButtonY = dialogY + 40; // Same Y as YES button

    iSetColor(60, 180, 60); // Green color for "NO"
    iFilledRectangle(noButtonX, noButtonY, buttonWidth, buttonHeight);
    iSetColor(255, 255, 255); // White text
    iText(noButtonX + (buttonWidth / 2) - 15, noButtonY + (buttonHeight / 2) - 8, "NO", GLUT_BITMAP_HELVETICA_18); // Center text within button
}
void iDraw() {
    iClear();

    switch (gameState) {
        case STATE_ADVENTURE_SPLASH: // New splash screen state
            drawAdventureSplashScreen();
            break;
        case STATE_TITLE:
            drawTitleScreen();
            break;
        case STATE_HELP_ADVENTURE: // NEW: Draw help screen when in this state
            drawHelpScreen();
            break;
        case STATE_LEVEL_SELECT:
            drawLevelSelect();
            break;
        case STATE_QUIT_CONFIRM:
            drawQuitConfirmDialog();
            break;
        case STATE_GAME_OVER:
            drawGameOverScreen();
            break;
        case STATE_LEVEL_COMPLETED: // <--- ADD THIS NEW CASE
            drawLevelCompletedScreen();
            break;
        case STATE_ENTER_NAME: // New state for name input
            drawEnterNameScreen();
            break;
        case STATE_LEADERBOARD: // New state for leaderboard display
            drawLeaderboardScreen();
            break;
        case STATE_GAME_SUMMARY: // NEW: Draw game summary screen
            drawGameSummaryScreen();
            break;
        case STATE_PLAYING:

            // CORRECTED: Draw background using the screen size constants
            iSetColor(85, 206, 255);
            iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            iShowLoadedImage2(0, 0, &bgPlayingImage, SCREEN_WIDTH, SCREEN_HEIGHT);

            // Draw all the level tiles
            for (int i = 0; i < numActualTiles; i++) {
                struct Tile currentTile = levelTiles[i];
                float drawX = currentTile.x - cameraX;
                float drawY = currentTile.y - cameraY;
                // CORRECTED: Use the screen size constant for checking boundaries
                if (drawX + currentTile.width < 0 || drawX > SCREEN_WIDTH) continue;

                if (currentTile.id == ID_BRICK) {
                    iSetColor(150, 75, 0); // Original brick color
                    iFilledRectangle(drawX, drawY, currentTile.width, currentTile.height);
                    iSetColor(0, 0, 0);
                    iRectangle(drawX, drawY, currentTile.width, currentTile.height);
                }
                else if (currentTile.id == FINISH_POINT) { // NEW: Draw finish point tiles
                    if (currentTile.hitByBall) {
                        iSetColor(160, 82, 45); // Faded brown (e.g., Sienna)
                    } else {
                        iSetColor(139, 69, 19); // Darker brown (e.g., SaddleBrown)
                    }
                    iFilledRectangle(drawX, drawY, currentTile.width, currentTile.height);
                    iSetColor(0, 0, 0);
                    iRectangle(drawX, drawY, currentTile.width, currentTile.height);
                }
                else if (currentTile.id == RED_FINISH_POINT) {
                    if (currentTile.hitByBall) {
                        iSetColor(255, 100, 100); // Faded red
                    } else {
                        iSetColor(255, 0, 0); // Bright red
                    }
                    iFilledRectangle(drawX, drawY, currentTile.width, currentTile.height);
                    iSetColor(0, 0, 0);
                    iRectangle(drawX, drawY, currentTile.width, currentTile.height);
                }
                else if (currentTile.id == SPIKE) {
                    iShowImage(drawX, drawY, "images/spike.png");
                }
                else if (currentTile.id == RING1) {
                    iShowImage(drawX, drawY, "images/ring1.png");
                }
                else if (currentTile.id == RING2) {
                    iShowImage(drawX, drawY, "images/ring2.png");
                }

            }

            // Draw the ball and the pause button
            iShowImage(ballX - cameraX, ballY - cameraY, ballImageNames[rotationFrame]);

            iSetColor(0, 0, 0); // Black button background
            iFilledRectangle(PAUSE_BUTTON_X, PAUSE_BUTTON_Y, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE);
            iSetColor(255, 255, 255); // White pause bars
            iFilledRectangle(PAUSE_BUTTON_X + 15, PAUSE_BUTTON_Y + 15, 10, 30);
            iFilledRectangle(PAUSE_BUTTON_X + 35, PAUSE_BUTTON_Y + 15, 10, 30);
            drawGameUI();
            break;
        case STATE_PAUSED:
            // CORRECTED: Draw the frozen game screen using the screen size constants
            iSetColor(85, 206, 255);
            iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            for (int i = 0; i < numActualTiles; i++) {
                struct Tile currentTile = levelTiles[i];
                float drawX = currentTile.x - cameraX;
                float drawY = currentTile.y - cameraY;
                // CORRECTED: Use the screen size constant for checking boundaries
                if (drawX + currentTile.width < 0 || drawX > SCREEN_WIDTH) continue;
                if (currentTile.id == ID_BRICK) {
                    iSetColor(150, 75, 0);
                    iFilledRectangle(drawX, drawY, currentTile.width, currentTile.height);
                    iSetColor(0, 0, 0);
                    iRectangle(drawX, drawY, currentTile.width, currentTile.height);
                }

                else if (currentTile.id == SPIKE) {
                    iShowImage(drawX, drawY, "images/spike.png");
                }
                else if (currentTile.id == RING1) {
                    iShowImage(drawX, drawY, "images/ring1.png");
                }
                else if (currentTile.id == RING2) {
                    iShowImage(drawX, drawY, "images/ring2.png");
                }


            }
            iShowImage(ballX - cameraX, ballY - cameraY, ballImageNames[rotationFrame]);

            // Draw the pause menu on top
            drawPauseMenu();
            drawGameUI();
            break;
    }
}




/*
function iMouseMove() is called when the user moves the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseMove(int mx, int my)
{
    // place your codes here
}

/*
function iMouseDrag() is called when the user presses and drags the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseDrag(int mx, int my)
{
    // place your codes here
}

/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        if (gameState == STATE_ADVENTURE_SPLASH &&
            mx >= SPLASH_START_BUTTON_X && mx <= SPLASH_START_BUTTON_X + SPLASH_START_BUTTON_WIDTH &&
            my >= SPLASH_START_BUTTON_Y && my <= SPLASH_START_BUTTON_Y + SPLASH_START_BUTTON_HEIGHT) {

            // CHANGE 1: Go to STATE_TITLE from splash screen
            gameState = STATE_TITLE;

            // Start music using PlaySoundW if enabled and not already playing
            if (musicEnabled && !musicPlaying) {
                PlaySoundW(backgroundMusic, NULL, SND_LOOP | SND_ASYNC); // Play sound, loop, and asynchronously
                musicPlaying = true;
            }
        }
        else if (gameState == STATE_TITLE) { // Main Menu
            if (mx >= PLAY_BUTTON_X && mx <= PLAY_BUTTON_X + MENU_BUTTON_WIDTH &&
                my >= PLAY_BUTTON_Y && my <= PLAY_BUTTON_Y + MENU_BUTTON_HEIGHT) {
                // CHANGE 2: Go to STATE_ENTER_NAME from Play button in Main Menu
                gameState = STATE_ENTER_NAME;
                // Clear any previous name input for a new game
                playerNameInput[0] = '\0';
                playerNameIndex = 0;
                gameEndedForScoreEntry = false; // Reset flag for new game start
            }
            else if (mx >= HELP_BUTTON_X && mx <= HELP_BUTTON_X + MENU_BUTTON_WIDTH &&
                     my >= HELP_BUTTON_Y && my <= HELP_BUTTON_Y + MENU_BUTTON_HEIGHT) {
                gameState = STATE_HELP_ADVENTURE;
            }
            else if (mx >= LEADERBOARD_BUTTON_X && mx <= LEADERBOARD_BUTTON_X + MENU_BUTTON_WIDTH && // Leaderboard button
                     my >= LEADERBOARD_BUTTON_Y && my <= LEADERBOARD_BUTTON_Y + MENU_BUTTON_HEIGHT) {
                loadLeaderboard(); // Load scores before displaying
                gameState = STATE_LEADERBOARD;
            }
            else if (mx >= EXIT_BUTTON_X && mx <= EXIT_BUTTON_X + MENU_BUTTON_WIDTH &&
                     my >= EXIT_BUTTON_Y && my <= EXIT_BUTTON_Y + MENU_BUTTON_HEIGHT) {
                // Change state to show quit confirmation dialog
                gameState = STATE_QUIT_CONFIRM;
            }
        }
        else if (gameState == STATE_LEVEL_SELECT) {
            float btn1_x = 140;
            float btn1_y = 320;

            float btn2_x = 140;
            float btn2_y = 150; // 150 pixels below the first button

            float btn3_x = 550;
            float btn3_y = 320;

            float btn4_x = 550;
            float btn4_y = 150;

            if (mx >= btn1_x && mx <= btn1_x + BUTTON_WIDTH && my >= btn1_y && my <= btn1_y + BUTTON_HEIGHT) {
                currentLevel = 1;
                loadLevel(currentLevel);
                gameState = STATE_PLAYING;
            }
            if (mx >= btn2_x && mx <= btn2_x + BUTTON_WIDTH && my >= btn2_y && my <= btn2_y + BUTTON_HEIGHT) {
                currentLevel = 2;
                loadLevel(currentLevel);
                gameState = STATE_PLAYING;
            }
            if (mx >= btn3_x && mx <= btn3_x + BUTTON_WIDTH && my >= btn3_y && my <= btn3_y + BUTTON_HEIGHT) {
                currentLevel = 3;
                loadLevel(currentLevel);
                gameState = STATE_PLAYING;
            }
            if (mx >= btn4_x && mx <= btn4_x + BUTTON_WIDTH && my >= btn4_y && my <= btn4_y + BUTTON_HEIGHT) {
                currentLevel = 4; // Assuming you have a level 4 or will add one
                loadLevel(currentLevel); // Make sure loadLevel handles currentLevel = 4
                gameState = STATE_PLAYING;
            }
        }
        else if (gameState == STATE_PLAYING) {
            // Check for Pause button click (top right)
            if (mx >= PAUSE_BUTTON_X && mx <= PAUSE_BUTTON_X + PAUSE_BUTTON_SIZE &&
                my >= PAUSE_BUTTON_Y && my <= PAUSE_BUTTON_Y + PAUSE_BUTTON_SIZE) {
                gameState = STATE_PAUSED;
                // Pause music if it's currently playing
                if (musicPlaying) {
                    // PlaySoundW(NULL, NULL, 0); // Stops all currently playing sounds
                }
            }
        }
        else if (gameState == STATE_PAUSED) {
            float resume_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
            float resume_btn_y = SCREEN_HEIGHT / 2;
            float menu_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
            float menu_btn_y = SCREEN_HEIGHT / 2 - 150;

            if (mx >= resume_btn_x && mx <= resume_btn_x + BUTTON_WIDTH && my >= resume_btn_y && my <= resume_btn_y + BUTTON_HEIGHT) {
                gameState = STATE_PLAYING;
                // Resume music if it was previously playing
                if (musicEnabled && !musicPlaying) { // Check musicEnabled to ensure it wasn't intentionally off
                    PlaySoundW(backgroundMusic, NULL, SND_LOOP | SND_ASYNC); // Restart the music
                    musicPlaying = true;
                }
            }
            if (mx >= menu_btn_x && mx <= menu_btn_x + BUTTON_WIDTH && my >= menu_btn_y && my <= menu_btn_y + BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
                // Optionally stop music when returning to title from pause
                if (musicPlaying) {
                    // PlaySoundW(NULL, NULL, 0); // Stop music
                    musicPlaying = false;
                }
            }
        }
        else if (gameState == STATE_HELP_ADVENTURE) {
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 && mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 50 && my <= 50 + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
        else if (gameState == STATE_QUIT_CONFIRM) { // Handle clicks in the quit confirmation dialog
            // Recalculate these values, just like in drawQuitConfirmDialog, or make them global/constants
            int dialogWidth = 600;
            int dialogHeight = 250;
            int dialogX = (SCREEN_WIDTH / 2) - (dialogWidth / 2);
            int dialogY = (SCREEN_HEIGHT / 2) - (dialogHeight / 2);

            int buttonWidth = 150;
            int buttonHeight = 60;
            int buttonPadding = 50;

            int yesButtonX = dialogX + (dialogWidth / 2) - buttonWidth - (buttonPadding / 2);
            int yesButtonY = dialogY + 40;

            int noButtonX = dialogX + (dialogWidth / 2) + (buttonPadding / 2);
            int noButtonY = dialogY + 40;

            // "YES" button click check
            if (mx >= yesButtonX && mx <= yesButtonX + buttonWidth && my >= yesButtonY && my <= yesButtonY + buttonHeight) {
                // Stop music before exiting if it's playing
                if (musicPlaying) {
                    // PlaySoundW(NULL, NULL, 0);
                    musicPlaying = false;
                }
                exit(0); // Confirmed quit
            }
            // "NO" button click check
            else if (mx >= noButtonX && mx <= noButtonX + buttonWidth && my >= noButtonY && my <= noButtonY + buttonHeight) {
                gameState = STATE_TITLE; // Return to Main Menu
            }
        }
        else if (gameState == STATE_GAME_OVER) {
            // Check for Main Menu button click (position 100 for Y and height 100, same as MENU_BUTTON_HEIGHT)
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 &&
                mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 100 && my <= 100 + MENU_BUTTON_HEIGHT) {

                gameState = STATE_TITLE; // Go back to Main Menu
                ballLives = 2; // Reset lives for a new game
                ringsCollected = 0; // Reset rings for a new game

                // Reset ball position and speed for a clean start when returning to the title screen
                // The actual level specific reset will happen when 'PLAY' is clicked again
                ballX = 100.0f;
                ballY = BASE_TILE_HEIGHT + 1;
                speedX = 0;
                vy = 0;
                onGround = true;

                // Stop any music playing from the game over screen
                if (musicPlaying) {
                    // PlaySoundW(NULL, NULL, 0); // Stop current sound
                    musicPlaying = false;
                }
            }
        }
        // --- NEW: Handle clicks on Level Completed screen ---
         else if (gameState == STATE_LEVEL_COMPLETED) {
            // Define button positions consistently with the drawing function
            float buttonX = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
            float nextLevelY = SCREEN_HEIGHT / 2 + 20;
            float restartY = SCREEN_HEIGHT / 2 - 100;
            float mainMenuY = SCREEN_HEIGHT / 2 - 220;

            // --- Check for "NEXT LEVEL" button click ---
            if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH &&
                my >= nextLevelY && my <= nextLevelY + MENU_BUTTON_HEIGHT) {
                
                currentLevel++;
                int maxLevels = 3; // The total number of levels you have
                
                if (currentLevel > maxLevels) {
                    // After the last level, go back to the main menu.
                    // You could also create a "You Win!" screen.
                    gameState = STATE_TITLE;
                } else {
                    loadLevel(currentLevel);
                    gameState = STATE_PLAYING;
                    // Resume music if it's enabled
                    if (musicEnabled) {
                        PlaySoundW(backgroundMusic, NULL, SND_LOOP | SND_ASYNC);
                        musicPlaying = true;
                    }
                }
            }
            // --- Check for "RESTART" button click ---
            else if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH &&
                     my >= restartY && my <= restartY + MENU_BUTTON_HEIGHT) {
                
                loadLevel(currentLevel); // Reload the same level
                gameState = STATE_PLAYING;
                // Resume music if it's enabled
                if (musicEnabled) {
                    PlaySoundW(backgroundMusic, NULL, SND_LOOP | SND_ASYNC);
                    musicPlaying = true;
                }
            }
            // --- Check for "MAIN MENU" button click ---
            else if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH &&
                     my >= mainMenuY && my <= mainMenuY + MENU_BUTTON_HEIGHT) {
                
                gameState = STATE_TITLE;
                // Reset game variables for a fresh start from the menu
                ballLives = 2;
                ringsCollected = 0;
                currentLevel = 1; 
            }
        }
        else if (gameState == STATE_ENTER_NAME) {
            // Check for "Submit" button click
            if (mx >= SCREEN_WIDTH / 2 - 75 && mx <= SCREEN_WIDTH / 2 - 75 + 150 &&
                my >= SCREEN_HEIGHT / 2 - 100 && my <= SCREEN_HEIGHT / 2 - 100 + 50) {
                // Submit name and score
                if (playerNameIndex > 0) { // Only add if name is not empty
                    addScoreToLeaderboard(playerNameInput, currentScore);
                }
                // Reset name input for next time
                playerNameInput[0] = '\0';
                playerNameIndex = 0;

                // CHANGE 3: Conditional transition based on how STATE_ENTER_NAME was reached
                if (gameEndedForScoreEntry) {
                    gameState = STATE_GAME_SUMMARY; // Go to summary if game just ended
                    gameEndedForScoreEntry = false; // Reset flag
                } else {
                    gameState = STATE_LEVEL_SELECT; // Go to level select for a new game
                }
                loadLeaderboard(); // Make sure leaderboard is updated
            }
        }
        else if (gameState == STATE_LEADERBOARD) {
            // "BACK TO MENU" button
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 && mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 50 && my <= 50 + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
        // NEW: Handle clicks on Game Summary screen
        else if (gameState == STATE_GAME_SUMMARY) {
            // Leaderboard Button
            float lb_btn_x = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
            float lb_btn_y = SCREEN_HEIGHT / 2 - 50;
            if (mx >= lb_btn_x && mx <= lb_btn_x + MENU_BUTTON_WIDTH &&
                my >= lb_btn_y && my <= lb_btn_y + MENU_BUTTON_HEIGHT) {
                loadLeaderboard(); // Ensure latest scores are loaded
                gameState = STATE_LEADERBOARD;
            }
            // Main Menu Button
            float menu_btn_x = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
            float menu_btn_y = SCREEN_HEIGHT / 2 - 150;
            if (mx >= menu_btn_x && mx <= menu_btn_x + MENU_BUTTON_WIDTH &&
                my >= menu_btn_y && my <= menu_btn_y + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
                // Reset game variables for a fresh start
                ballLives = 2;
                ringsCollected = 0;
                ballX = 100.0f;
                ballY = BASE_TILE_HEIGHT + 1;
                speedX = 0;
                vy = 0;
                onGround = true;
                cameraX = 0;
                cameraY = 0;
                // PlaySoundW(NULL, NULL, 0); // Stop music
                musicPlaying = false; // Update flag
            }
        }
    }
}
/*
function iMouseWheel() is called when the user scrolls the mouse wheel.
dir = 1 for up, -1 for down.
*/
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}

/*
function iKeyboard() is called whenever the user hits a key in keyboard.
key- holds the ASCII value of the key pressed.
*/
/*
function iKeyboard() is called whenever the user hits a key in keyboard.
key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key, int state) {
    // Movement logic should allow key repeat (state == 0 or 2)
    if (gameState == STATE_PLAYING && (state == 0 || state == 2)) {
        if (key == 'a') {
            speedX -= accelerationX;
            rotationFrame = (rotationFrame - 3 + TOTAL_FRAMES) % TOTAL_FRAMES;
        }
        if (key == 'd') {
            speedX += accelerationX;
            rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES;
        }
        if ((key == 'w' || key == ' ') && onGround) {
            vy = 13;
            rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES;
            onGround = false;
        }
    }

    // Typing name should allow only state == 0 (first press)
    else if (gameState == STATE_ENTER_NAME && state == 0) {
        if (key == '\b') {
            if (playerNameIndex > 0) {
                playerNameIndex--;
                playerNameInput[playerNameIndex] = '\0';
            }
        } else if (key == '\r') {
            if (playerNameIndex > 0) {
                addScoreToLeaderboard(playerNameInput, currentScore);
            }
            playerNameInput[0] = '\0';
            playerNameIndex = 0;

            if (gameEndedForScoreEntry) {
                gameState = STATE_GAME_SUMMARY;
                gameEndedForScoreEntry = false;
            } else {
                gameState = STATE_LEVEL_SELECT;
            }
            loadLeaderboard();
        } else if (playerNameIndex < sizeof(playerNameInput) - 1) {
            if (isalnum(key) || key == ' ') {
                playerNameInput[playerNameIndex++] = key;
                playerNameInput[playerNameIndex] = '\0';
            }
        }
    }

    // Global commands like pause, menu, and exit (only on press)
    if (state == 0) {
        if (key == 'p') {
            if (gameState == STATE_PLAYING) gameState = STATE_PAUSED;
            else if (gameState == STATE_PAUSED) gameState = STATE_PLAYING;
        }

        if (key == 'm') {
            if (gameState == STATE_PLAYING || gameState == STATE_PAUSED ||
                gameState == STATE_HELP_ADVENTURE || gameState == STATE_LEADERBOARD ||
                gameState == STATE_GAME_SUMMARY) {
                gameState = STATE_TITLE;
                ballLives = 2;
                ringsCollected = 0;
                ballX = 100.0f;
                ballY = BASE_TILE_HEIGHT + 1;
                speedX = 0;
                vy = 0;
                onGround = true;
                cameraX = 0;
                cameraY = 0;
                musicPlaying = false;
            }
        }

        if (key == 27) { // Escape
            exit(0);
        }
    }
}
void updateBall() {
    // --- 1. Apply basic physics (gravity and friction) ---
    vy -= gravity;
    speedX *= frictionX;

    if (fabs(speedX) < 0.1) {
        speedX = 0;
    }

    // --- 2. Handle Horizontal Movement and Collision ---
    ballX += speedX;

    for (int i = 0; i < numActualTiles; ++i) {
        struct Tile tile = levelTiles[i];
        if (!tile.isSolid) continue; // Skip non-solid tiles in this loop

        // Check for an overlap between the ball and the solid tile
        if (ballX + ballWidth > tile.x && ballX < tile.x + tile.width &&
            ballY + ballHeight > tile.y && ballY < tile.y + tile.height) {

            // Check if the SOLID TILE is the finish line BEFORE stopping the ball
            if (tile.id == RED_FINISH_POINT) {
                int requiredRings = (int)ceil(totalRingsInLevel * 0.70);
                if (ringsCollected >= requiredRings) {
                    // If win condition is met, trigger level completion
                    printf("Level %d Completed! (%d/%d rings)\n", currentLevel, ringsCollected, totalRingsInLevel);
                    gameState = STATE_LEVEL_COMPLETED;
                    speedX = 0;
                    vy = 0;
                    if (musicPlaying) {
                        PlaySoundW(NULL, NULL, 0);
                        musicPlaying = false;
                    }
                    // Exit the function early since the level is over
                    return;
                }
            }

            // If it wasn't the finish line (or not enough rings), treat it as a normal wall
            if (speedX > 0) {
                ballX = tile.x - ballWidth;
            } else if (speedX < 0) {
                ballX = tile.x + tile.width;
            }
            
            speedX = 0;
        }
    }

    // --- 3. Handle Vertical Movement and Collision ---
    onGround = false;
    ballY += vy;

    for (int i = 0; i < numActualTiles; ++i) {
        struct Tile tile = levelTiles[i];
        if (!tile.isSolid) continue;

        if (ballX + ballWidth > tile.x && ballX < tile.x + tile.width &&
            ballY + ballHeight > tile.y && ballY < tile.y + tile.height) {
            
            // Check for landing on a finish block
            if (tile.id == RED_FINISH_POINT) {
                if (ringsCollected >= totalRingsInLevel) {
                    printf("Level %d Completed! (%d/%d rings)\n", currentLevel, ringsCollected, totalRingsInLevel);
                    gameState = STATE_LEVEL_COMPLETED;
                    speedX = 0;
                    vy = 0;
                    if (musicPlaying) {
                        PlaySoundW(NULL, NULL, 0);
                        musicPlaying = false;
                    }
                    return;
                }
            }

            // If not the finish line, resolve vertical collision
            if (vy > 0) {
                ballY = tile.y - ballHeight;
            } else {
                ballY = tile.y + tile.height;
                onGround = true;
            }
            vy = 0;
        }
    }

    // --- 4. Ring and Spike Collision ---
    for (int i = 0; i < numActualTiles; i++) {
        if (levelTiles[i].id == RING1 || levelTiles[i].id == RING2 || levelTiles[i].id == SPIKE) {
            if (ballX + ballWidth > levelTiles[i].x && ballX < levelTiles[i].x + levelTiles[i].width &&
                ballY + ballHeight > levelTiles[i].y && ballY < levelTiles[i].y + levelTiles[i].height) {

                if (levelTiles[i].id == RING1 || levelTiles[i].id == RING2) {
                    levelTiles[i].id = EMPTY;
                    ringsCollected++;
                    printf("Ring collected! Total: %d/%d\n", ringsCollected, totalRingsInLevel);
                } 
                else if (levelTiles[i].id == SPIKE) {
                    printf("Ouch! Hit a spike.\n");

                    // Immediately reset the ball's position and stop its movement.
                    vy = 0;
                    speedX = 0;
                    onGround = true;

                    // Reset position to the EXACT start of the current level
                    if (currentLevel == 1) {
                        ballX = 100.0f;
                        ballY = BASE_TILE_HEIGHT + 1;
                    } else if (currentLevel == 2) {
                        ballX = 210.0f;
                        ballY = BASE_TILE_HEIGHT;
                    } else if (currentLevel == 3) {
                        ballX = 54 * BASE_TILE_WIDTH;
                        ballY = BASE_TILE_HEIGHT * 44;
                    }

                    // Now, decrement a life
                    ballLives--;

                    // Check if the game is truly over (lives have dropped below 0)
                    if (ballLives < 0) {
                        currentScore = ringsCollected * 100; // Example scoring
                        gameEndedForScoreEntry = true; // Set flag for the name entry screen
                        gameState = STATE_ENTER_NAME; // Go to enter name screen after game over
                    }
                }
            }
        }
    }

    // --- 5. World Boundary and Camera Logic ---
    float levelActualWidth = NUM_FLOOR_TILES * BASE_TILE_WIDTH;
    if (ballX < 0) { 
        ballX = 0; 
        if (speedX < 0) speedX = 0; 
    }
    if (ballX + ballWidth > levelActualWidth) { 
        ballX = levelActualWidth - ballWidth; 
        if (speedX > 0) speedX = 0; 
    }

    if (ballY < -100) {
        ballLives--; // NOTE: Added this to make falling off-screen cost a life, for consistency.
        if(ballLives < 0){
             currentScore = ringsCollected * 100;
             gameEndedForScoreEntry = true;
             gameState = STATE_ENTER_NAME;
        }
        else{
            vy = 0; 
            speedX = 0; 
            onGround = true;
             if (currentLevel == 1) {
                ballX = 100.0f;
                ballY = BASE_TILE_HEIGHT + 1;
            } else if (currentLevel == 2) {
                ballX = 210.0f;
                ballY = BASE_TILE_HEIGHT;
            } else if (currentLevel == 3) {
                ballX = 54 * BASE_TILE_WIDTH;
                ballY = BASE_TILE_HEIGHT * 44;
            }
        }
    }

    int worldWidthForCamera = (int)levelActualWidth;
    int maxCameraX = worldWidthForCamera - SCREEN_WIDTH;
    if (maxCameraX < 0) maxCameraX = 0;
    cameraX = (int)(ballX + ballWidth / 2.0f) - SCREEN_WIDTH / 3;
    if (cameraX < 0) cameraX = 0;
    if (cameraX > maxCameraX) cameraX = maxCameraX;

    const int LEVEL_HEIGHT_PIXELS = ROWS * BASE_TILE_HEIGHT;
    int maxCameraY = LEVEL_HEIGHT_PIXELS - SCREEN_HEIGHT;
    if (maxCameraY < 0) maxCameraY = 0;
    cameraY = (int)(ballY + ballHeight / 2.0f) - SCREEN_HEIGHT / 2;
    if (cameraY < 0) cameraY = 0;
    if (cameraY > maxCameraY) cameraY = maxCameraY;
}
void gameLoop() {
    // This function ensures the ball physics only runs when we are playing.
    if (gameState == STATE_PLAYING) {
        updateBall();
    }
}
/*
function iSpecialKeyboard() is called whenver user hits special keys likefunction
keys, home, end, pg up, pg down, arraows etc. you have to use
appropriate constants to detect them. A list is:
GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11,
GLUT_KEY_F12, GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN,
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN, GLUT_KEY_HOME, GLUT_KEY_END,
GLUT_KEY_INSERT */
void iSpecialKeyboard(unsigned char key, int state)
{
}
int main(int argc, char *argv[])
{
    glutInit(&argc, argv); // Required before iInitialize (for GLUT-based systems)
    iLoadImage(&bgPlayingImage, "images/landingpage.png");
    loadBallImageNames();
    iSetTimer(10, gameLoop);
    iOpenWindow(1000,563, "Test Window");
    PlaySound(NULL, 0, 0);  // Stops music

    // Initialize player name input buffer
    playerNameInput[0] = '\0';
    playerNameIndex = 0;

    loadLeaderboard(); // Load existing scores at startup

    return 0;

}

