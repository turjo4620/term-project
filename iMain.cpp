#include "iGraphics.h"
#include<stdio.h>
#include<math.h>
#include<stdbool.h>
#include "iSound.h"  // if iGraphics has it separately or use proper setup
#include <mmsystem.h>
#include <fstream>   // For file input/output
#include <string>    // For string manipulation
#include <vector>    // For dynamic array of scores
#include <algorithm> // For sorting
#include <iomanip>   // For output formatting
Image bgPlayingImage;


int musicChannel = -1;

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
    STATE_GAME_SUMMARY,
     STATE_SETTINGS     
};
int gameState = STATE_ADVENTURE_SPLASH; // The game will now start in the menu
int ballLives = 2;
bool isMusicEnabled = true;
bool isSoundEffectsEnabled = true;

// --- Player Score and Leaderboard ---
struct PlayerScore {
    char name[50]; // Fixed size for simplicity
    int score;

    // Operator for sorting (descending by score)
    bool operator<(const PlayerScore& other) const {
        return score > other.score;
    }
};

std::vector<PlayerScore> leaderboard_level1;
std::vector<PlayerScore> leaderboard_level2;
std::vector<PlayerScore> leaderboard_level3;
char playerNameInput[50];
int playerNameIndex = 0;
char currentPlayerName[50]; 
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
#define PLAY_BUTTON_Y (SCREEN_HEIGHT / 2 + 120)

#define HELP_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)
#define HELP_BUTTON_Y (SCREEN_HEIGHT / 2 + 40)

#define EXIT_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)
#define EXIT_BUTTON_Y (SCREEN_HEIGHT / 2 - 200)

#define LEADERBOARD_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2) // New button for leaderboard
#define LEADERBOARD_BUTTON_Y (SCREEN_HEIGHT / 2 - 40) // Position it below EXIT

#define MENU_BUTTON_X (SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2)

#define SETTINGS_BUTTON_Y (SCREEN_HEIGHT / 2 - 120)

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
#define SPIKE1 2
#define SPIKE2 20
#define THING 27
#define MOVING_ARROW 21
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
float frictionX = 0.95;    // Rate at which speed decreases when no key is pressed
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
float lastScoringPositionX = 0.0f;
int mouse_x = 0;
int mouse_y = 0;

#define MAX_SPEED_X 8.0f

// NEW FUNCTION: Draws the initial splash screen
void drawAdventureSplashScreen() {
    iShowImage(0, 0, "images/front.png");

    // START Button with hover effect
    if (mouse_x >= SPLASH_START_BUTTON_X && mouse_x <= SPLASH_START_BUTTON_X + SPLASH_START_BUTTON_WIDTH &&
        mouse_y >= SPLASH_START_BUTTON_Y && mouse_y <= SPLASH_START_BUTTON_Y + SPLASH_START_BUTTON_HEIGHT) {
        iSetColor(50, 200, 50); // Brighter green for hover
    } else {
        iSetColor(0, 150, 0);   // Normal green
    }
    iFilledRectangle(SPLASH_START_BUTTON_X, SPLASH_START_BUTTON_Y, SPLASH_START_BUTTON_WIDTH, SPLASH_START_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SPLASH_START_BUTTON_X + 65, SPLASH_START_BUTTON_Y + 30, "START", GLUT_BITMAP_TIMES_ROMAN_24);
}         // A 60x60 pixel button
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





struct MovingArrow {
    float tileIndex;         // Index in levelTiles[]
    float baseY;           // Starting Y position (center point)
    float moveRange;       // How far it moves up and down from baseY
    float speed;
    bool movingUp;
};

#define MAX_ARROWS 50
MovingArrow movingArrows[MAX_ARROWS];
int numMovingArrows = 0;


// ... (previous drawing functions like drawLevelSelect, drawSpinner, etc.) ...

// NEW FUNCTION: Draws a simple help screen

// ... (iDraw() function follows) ...
// Draws the quit confirmation dialog over the main menu

// Draws the quit confirmation dialog over the main menu

void drawGameOverScreen() {
    iShowImage(0, 0, "images/gameover.png");

    // iSetColor(255, 255, 255); // White text
    // iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, "You ran out of lives!", GLUT_BITMAP_HELVETICA_18);

    char finalScoreText[50];
    sprintf(finalScoreText, "Your Score: %d", currentScore);
    iSetColor(255, 255, 0); // Yellow for the score
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, finalScoreText, GLUT_BITMAP_TIMES_ROMAN_24);


    // Return to Main Menu Button
    iSetColor(100, 100, 100); // Gray
    iFilledRectangle(SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2, 100, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 70, 125, "MAIN MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}
void resetBallPosition() {
    // For Level 1, always go back to the absolute start.
    if (currentLevel == 1) {
        ballX = 100.0f;
        ballY = BASE_TILE_HEIGHT + 1;
    } 
    // For Level 2, move the player back a safe distance near the floor.
    else if (currentLevel == 2) {
        // We move the ball back 10 tile widths. You can change this value.
        float respawnX = ballX - (10 * BASE_TILE_WIDTH);
        float levelStartX = 7 * BASE_TILE_WIDTH;

        if (respawnX < levelStartX) {
            ballX = levelStartX;
        } else {
            ballX = respawnX;
        }
        // Place the ball safely above the ground at the new X position.
        ballY = BASE_TILE_HEIGHT + 1;
    }
    // NEW: Specific logic for Level 3's high platforms.
    else if (currentLevel == 3) {
        // Move the ball back 10 tile widths.
        float respawnX = ballX - (10 * BASE_TILE_WIDTH);
        float levelStartX = 54 * BASE_TILE_WIDTH;

        if (respawnX < levelStartX) {
            ballX = levelStartX;
        } else {
            ballX = respawnX;
        }
        // CORRECTED: Place the ball on Level 3's main floor (which is at height 29).
        ballY = BASE_TILE_HEIGHT * 30; 
    }

    // Reset the ball's velocity and scoring tracker for all levels
    speedX = 0;
    vy = 0;
    lastScoringPositionX = ballX;
}

// NEW FUNCTION: Draws the Level Completed screen
void drawLevelCompletedScreen() {
    iShowImage(0, 0, "images/levelcomplete.png");
    // iSetColor(255, 255, 255);
    // iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 150, "LEVEL COMPLETED!", GLUT_BITMAP_TIMES_ROMAN_24);
    char finalScoreText[50];
    sprintf(finalScoreText, "Your Score: %d", currentScore);
    iSetColor(255, 255, 0);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 200, finalScoreText, GLUT_BITMAP_TIMES_ROMAN_24);

    float buttonX = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
    float nextLevelY = SCREEN_HEIGHT / 2 + 20;
    float restartY = SCREEN_HEIGHT / 2 - 100;
    float mainMenuY = SCREEN_HEIGHT / 2 - 220;

    // NEXT LEVEL Button
    if (mouse_x >= buttonX && mouse_x <= buttonX + MENU_BUTTON_WIDTH && mouse_y >= nextLevelY && mouse_y <= nextLevelY + MENU_BUTTON_HEIGHT) {
        iSetColor(50, 220, 100);
    } else {
        iSetColor(0, 180, 50);
    }
    iFilledRectangle(buttonX, nextLevelY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 70, nextLevelY + 25, "NEXT LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

    // RESTART Button
    if (mouse_x >= buttonX && mouse_x <= buttonX + MENU_BUTTON_WIDTH && mouse_y >= restartY && mouse_y <= restartY + MENU_BUTTON_HEIGHT) {
        iSetColor(240, 190, 50);
    } else {
        iSetColor(200, 150, 0);
    }
    iFilledRectangle(buttonX, restartY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 80, restartY + 25, "RESTART", GLUT_BITMAP_TIMES_ROMAN_24);

    // MAIN MENU Button
    if (mouse_x >= buttonX && mouse_x <= buttonX + MENU_BUTTON_WIDTH && mouse_y >= mainMenuY && mouse_y <= mainMenuY + MENU_BUTTON_HEIGHT) {
        iSetColor(220, 100, 50);
    } else {
        iSetColor(180, 50, 0);
    }
    iFilledRectangle(buttonX, mainMenuY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(buttonX + 70, mainMenuY + 25, "MAIN MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}




void addMovingArrow(float startRow, float startcol, float baseRow, float range, float speed, bool startMovingUp) {
    levelTiles[numActualTiles].id = MOVING_ARROW;
    levelTiles[numActualTiles].x = startRow * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = startcol * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = false;

    int idx = numActualTiles;
    numActualTiles++;

    movingArrows[numMovingArrows].tileIndex = idx;
    movingArrows[numMovingArrows].baseY = baseRow * BASE_TILE_HEIGHT;
    movingArrows[numMovingArrows].moveRange = range;
    movingArrows[numMovingArrows].speed = speed;
    movingArrows[numMovingArrows].movingUp = startMovingUp;
    numMovingArrows++;
}



void addBrickBlock(float startCol, float endCol, float startRow,float endRow) {
    for (int i = startCol; i <= endCol; i++) {
        for (int j = startRow; j <= endRow; j++) {
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

void addRing(float col,float row ,int ringType) {
    if (numActualTiles < MAX_LEVEL_TILES) {
        levelTiles[numActualTiles].id = ringType;
        levelTiles[numActualTiles].x = col * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y = row * BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = false;
        numActualTiles++;
    }
}

void addSpike(float col, float row, int spikeType) {
    if (numActualTiles < MAX_LEVEL_TILES) {
        levelTiles[numActualTiles].id = spikeType;
        levelTiles[numActualTiles].x = col * BASE_TILE_WIDTH;
        levelTiles[numActualTiles].y = row * BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
        levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
        levelTiles[numActualTiles].isSolid = false;
        numActualTiles++;
    }
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
    
   addBrickBlock(0, NUM_FLOOR_TILES - 1, 0, 0);  // Row 0, all columns


    // Add top roof tiles
    
 addBrickBlock(0, NUM_FLOOR_TILES - 1, 9, 9);


    // Add left vertical wall
   addBrickBlock(0, 0, 0, 9);

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
  addBrickBlock(4, 9, 4, 9);

addBrickBlock(13, 14, 0, 5);


    addBrickBlock(19, 21, 3, 9);

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
addBrickBlock(30, 33, 3, 4);

    // Middle support brick

   addBrickBlock(31, 32, 2, 2);

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
            if(j==8 || (j<=7 && (i==42 || i==43)))
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

   addBrickBlock(69, 70, 0, 5);

   addBrickBlock(73, 74, 4, 9);

   addBrickBlock(77, 78, 0, 5);

addBrickBlock(81, 82, 4, 9);

   addBrickBlock(85, 86, 0, 5);


addBrickBlock(91, 92, 0, 2);

   addBrickBlock(93, 93, 0, 1);



  addBrickBlock(94, 95, 0, 3);

    addBrickBlock(97, 98, 4, 9);

   addBrickBlock(100 ,101, 0, 3);

  addBrickBlock(102, 102, 0, 1);
  
    
    addBrickBlock(103, 104, 0, 2);

    // drawing rings

   

    addRing(7,1,RING1);

  
    addRing(31, 5 ,RING1);

     addRing(73, 1 ,RING1);
    

     addRing(81, 1 ,RING1);

    //horizontal rings

    
     addRing(96, 3 ,RING2);

    
 addRing(99, 3 ,RING2);


    //drawing SPIKE1s


    addSpike(16,1,SPIKE1);

   
    addSpike(28,1,SPIKE1);
   
    
    addSpike(34,1,SPIKE1);

    
    
    addSpike(49,1,SPIKE1);

   
    
    addSpike(52,1,SPIKE1);

    
    addSpike(56,1,SPIKE1);

    
    addSpike(59,1,SPIKE1);

    addSpike(93,2,SPIKE1);
    
    addSpike(102,2,SPIKE1);
    countTotalRings();
}
void initializeLevel2(){
    numActualTiles = 0;
    ballX = 7* BASE_TILE_WIDTH; // The wall ends at x=200, so 250 is a safe start
    ballY = 7*BASE_TILE_HEIGHT ; // Just above the floor
    speedX = 0;
    vy = 0;
    NUM_FLOOR_TILES = 134;
    // -------- LEVEL 2 FLOOR --------
   addBrickBlock(0, NUM_FLOOR_TILES - 1, 0, 0);

   
     addBrickBlock(0, 1, 0, ROWS - 1);
    
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

 addBrickBlock(12, 13, 0, 5);

    for (int i = 14; i <= 19; i++) {
   
        for (int j = 4; j <= 5; j++) {
         
            if ((i == 14 && j== 4) || (i == 15 && j== 4) || (i == 16 && j== 4) || (i == 17 && j== 4))  continue;
           {

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

  

      addBrickBlock(22, 23, 4,8);

   

     addBrickBlock(26, 27, 1, 5);



 addBrickBlock(30, 31, 6, 9);

  
 addBrickBlock(37, 41, 1, 5);

   

 addBrickBlock(61,62, 4, 8);


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


     addBrickBlock(85, 86, 4,  11);


   addBrickBlock(96, 97, 6, 9);

 
       addBrickBlock(96, 97, 1, 3);


           addBrickBlock(107,108, 1, 5);

           addBrickBlock(111, 112, 4, 8);

           addBrickBlock(115, 118, 4, 4);



           addBrickBlock(117, 118, 1, 5);


           addBrickBlock(122, 123, 5, 8);
 

           addBrickBlock(126, 128, 5, 5);
    
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
   
           addBrickBlock(103, 104, 10, 30);



           addBrickBlock(109, 128, 19, 19);


           addBrickBlock(105, 107, 15, 15);
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

           addBrickBlock(105, 120, 23, 23);
   

           addBrickBlock(105, 128, 28, 29);

// drawing vertical rings

    

    addRing(18,6,RING1);

    
    addRing(37,6,RING1);

    addRing(61,1,RING1);

    
addRing(85,1,RING1);
    
    addRing(96,4,RING1);


   
addRing(107,6,RING1);

     
    addRing(119,24,RING1);



   //drawing horizontal ring
    

addRing(127.5,9,RING2);









    
    //drawing SPIKE1s



     addSpike(10,1,SPIKE1);
    
     addSpike(127,1,SPIKE1);

 
         addSpike(117,10,SPIKE1);


     // drawing moving arrow    

addMovingArrow(3.5, 2, 4, 180, 1.2, false);  // col, startYRow, baseYRow, range, speed, dir
addMovingArrow(2, 2, 4, 180, 1.2, false);

addMovingArrow(46, 2, 4, 180, 1.2, false);
addMovingArrow(51, 2, 4, 180, 1.2, false);
addMovingArrow(56, 2, 4, 180, 1.2, false);


addMovingArrow(88, 2, 4, 180, 1.2, false);




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
            levelTiles[numActualTiles].y = BASE_TILE_HEIGHT*29;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


           addBrickBlock(0, 1,20, 69);

    //top roof
 

           addBrickBlock(0, 132, 47, 47);

   

           addBrickBlock(6, 7,35, 35);

   

           addBrickBlock(2, 4, 39, 39);

    for (int i = 10; i <= 13; i++) {
        for (int j = 29; j <=32; j++)
            if((i==10 || i==11) ||(i==12 && (j==31 || j==30)) || (i==13 && j==30))
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




    for (int i = 20; i <= 25; i++) {
        for (int j = 29; j <=32; j++)
            if((i==22 || i==23 || i==24 || i==25) ||(i==21 && (j==31 || j==30)) || (i==20 && j==30))
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




           addBrickBlock(10, 11, 39, 46);

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



           addBrickBlock(131, 132, 40, 47);

                addBrickBlock(111,112, 44, 44);


                addBrickBlock(113, 114, 42, 42);

 

                addBrickBlock(115, 116, 44, 44);

 


                addBrickBlock(118, 119, 41, 41);
          
                addBrickBlock(121, 122, 40, 40);
 

                addBrickBlock(122, 123, 43, 43);

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
        for (int j = 29; j <=38; j++)

        { if(j == 32 || j==33 ) continue;
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

    
    

           addBrickBlock(22, 36, 25, 25);

   
       addBrickBlock(22, 46, 21, 21);

    for (int i = 37; i <= 38; i++) {
        for (int j = 29; j <=38; j++)

        { if(j == 32 || j==33  ) continue;
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
        for (int j = 29; j <=32; j++)

        { if((i==46 && j== 32) || (i == 47 && (j==31 || j==32))) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }



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


   for (int i = 66; i <= 68; i++) {
        for (int j =14 ; j <=29; j++)
        
        { if(i==68 && j== 29) continue;
            levelTiles[numActualTiles].id = ID_BRICK;
            levelTiles[numActualTiles].x = LEVEL_START_X + i * BASE_TILE_WIDTH;
            levelTiles[numActualTiles].y = LEVEL_START_Y + j * BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
            levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
            levelTiles[numActualTiles].isSolid = true;
            numActualTiles++;
        }
    }


  
       addBrickBlock(68, 132,  14, 14);

     

     addBrickBlock(74, 91, 17, 39);
    
   

         addBrickBlock(94, 111, 17, 39);




         addBrickBlock(112, 132, 22, 22);
    


         addBrickBlock(132, 133, 14, 22);


     addBrickBlock(123, 124, 15, 15);

// drawing vertical rings

    

    addRing(45,42,RING1);


    
    addRing(31,32,RING1);

    
    addRing(37,32,RING1);

    
    addRing(74,15,RING1);

     
    addRing(110,15,RING1);


        
    addRing(60,43.4,RING1);

    
        
    addRing(68,42,RING1);


        
    addRing(77,42,RING1);




    // drawing horizontal rings

    
    addRing(16.5,39,RING2);


    //drawing spike1

    

      addSpike(39,30,SPIKE1);   

            addSpike(43,30,SPIKE1);

      addSpike(123,16,SPIKE1);

         addSpike(123,40,SPIKE1);

     addSpike(124,40,SPIKE1);
        
     addSpike(129,40,SPIKE1);

    //drawing spike2

    addSpike(6,33.4,SPIKE2);
     addSpike(4,45.4,SPIKE2);

    
    
 
 addSpike(2,45.4,SPIKE2);
    
     addSpike(53,37.4,SPIKE2);

   
 addSpike(58,37.4,SPIKE2);
    

    addSpike(70,37.4,SPIKE2);


    //draw things

    
    levelTiles[numActualTiles].id = THING;
    levelTiles[numActualTiles].x = LEVEL_START_X + 24 * BASE_TILE_WIDTH;
    levelTiles[numActualTiles].y = LEVEL_START_Y + 26 * BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].width = BASE_TILE_WIDTH;
    levelTiles[numActualTiles].height = BASE_TILE_HEIGHT;
    levelTiles[numActualTiles].isSolid = true;  
    numActualTiles++;

 // drawing moving arrow

 addMovingArrow(41, 43, 42.5, 145, 1.2, false);
 addMovingArrow(39.5,43, 42.5, 145, 1.2, false);
addMovingArrow(38,43, 42.5, 145, 1.2, false);
addMovingArrow(36.5,43, 42.5, 145, 1.2, false);
addMovingArrow(35, 43, 42.5, 145, 1.2, false);
addMovingArrow(33.5, 43, 42.5, 145, 1.2, false);


addMovingArrow(26, 33, 33.5, 205, 1.2, false);

addMovingArrow(117, 17, 17.5, 145, 1.2, false);



  

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

    char scoreText[50];
    sprintf(scoreText, "Score: %d", currentScore);
    // Positioned 200 pixels from the right edge and 40 from the top.
    iText(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 40, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
}
// Global array to store all tiles in the level
int ctualTiles = 0; // Number of tiles currently in use
void loadLevel(int level) {
    ballLives = 2;
    ringsCollected = 0;

    // ADD THESE TWO LINES to reset scoring for the new level
    currentScore = 0;
    lastScoringPositionX = (level == 1) ? 100.0f : ((level == 2) ? 7 * BASE_TILE_WIDTH : 54 * BASE_TILE_WIDTH); // Reset to level's start X

    if (level == 1) {
        initializeLevel1();
    } else if (level == 2) {
        initializeLevel2();
    } else if (level == 3) {
        initializelevel3();
    }
    countTotalRings(); // You had this, but it's good to ensure it's after initialization
}





void updateAllMovingArrows() {
    for (int i = 0; i < numMovingArrows; i++) {
        int idx = movingArrows[i].tileIndex;
        float& y = levelTiles[idx].y;

        if (movingArrows[i].movingUp) {
            y += movingArrows[i].speed;
            if (y >= movingArrows[i].baseY + movingArrows[i].moveRange) {
                movingArrows[i].movingUp = false;
            }
        } else {
            y -= movingArrows[i].speed;
            if (y <= movingArrows[i].baseY - movingArrows[i].moveRange) {
                movingArrows[i].movingUp = true;
            }
        }
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
    leaderboard_level1.clear();
    leaderboard_level2.clear();
    leaderboard_level3.clear();

    std::ifstream file("D:\\Projects\\Modern-iGraphics-0.4.0\\Modern-iGraphics-0.4.0\\leaderboard.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string levelStr, nameStr, scoreStr;
            
            std::getline(ss, levelStr, ',');
            std::getline(ss, nameStr, ',');
            std::getline(ss, scoreStr, ',');

            if (!levelStr.empty() && !nameStr.empty() && !scoreStr.empty()) {
                int level = std::stoi(levelStr);
                PlayerScore ps;
                strncpy(ps.name, nameStr.c_str(), sizeof(ps.name) - 1);
                ps.name[sizeof(ps.name) - 1] = '\0';
                ps.score = std::stoi(scoreStr);

                if (level == 1) leaderboard_level1.push_back(ps);
                else if (level == 2) leaderboard_level2.push_back(ps);
                else if (level == 3) leaderboard_level3.push_back(ps);
            }
        }
        file.close();
        
        // Sort all three lists after loading
        std::sort(leaderboard_level1.begin(), leaderboard_level1.end());
        std::sort(leaderboard_level2.begin(), leaderboard_level2.end());
        std::sort(leaderboard_level3.begin(), leaderboard_level3.end());
    }
}

void saveLeaderboard() {
    std::ofstream file("D:\\Projects\\Modern-iGraphics-0.4.0\\Modern-iGraphics-0.4.0\\leaderboard.txt");
    if (file.is_open()) {
        for (const auto& score : leaderboard_level1) file << "1," << score.name << "," << score.score << "\n";
        for (const auto& score : leaderboard_level2) file << "2," << score.name << "," << score.score << "\n";
        for (const auto& score : leaderboard_level3) file << "3," << score.name << "," << score.score << "\n";
        file.close();
    }
}

// NEW: Adds a score to the correct level's list.
void addScoreToLeaderboard(int level, const char* name, int score) {
    // Choose the correct vector based on the level
    std::vector<PlayerScore>* currentLeaderboard = nullptr;
    if (level == 1) currentLeaderboard = &leaderboard_level1;
    else if (level == 2) currentLeaderboard = &leaderboard_level2;
    else if (level == 3) currentLeaderboard = &leaderboard_level3;
    else return; // Do nothing for invalid levels

    bool playerExists = false;
    for (auto& entry : *currentLeaderboard) {
        if (strcmp(entry.name, name) == 0) {
            playerExists = true;
            if (score > entry.score) entry.score = score;
            break;
        }
    }

    if (!playerExists) {
        PlayerScore newScore;
        strncpy(newScore.name, name, sizeof(newScore.name) - 1);
        newScore.name[sizeof(newScore.name) - 1] = '\0';
        newScore.score = score;
        currentLeaderboard->push_back(newScore);
    }

    std::sort(currentLeaderboard->begin(), currentLeaderboard->end());
    saveLeaderboard();
}

void drawEnterNameScreen() {
    // First, draw the main menu in the background for the overlay effect
    iShowImage(0, 0, "images/landingpage.png");

    // --- Define and Draw the Dialog Box ---
    int dialogWidth = 400;
    int dialogHeight = 250;
    int dialogX = SCREEN_WIDTH / 2 - dialogWidth / 2;
    int dialogY = SCREEN_HEIGHT / 2 - dialogHeight / 2;

    iSetColor(50, 50, 70); // Dark blue-gray background
    iFilledRectangle(dialogX, dialogY, dialogWidth, dialogHeight);
    iSetColor(200, 200, 220); // Light border
    iRectangle(dialogX, dialogY, dialogWidth, dialogHeight);

    iSetColor(255, 255, 255);
    iText(dialogX + 100, dialogY + 200, "ENTER YOUR NAME", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Name Input Field (Positioned inside the dialog) ---
    float fieldWidth = 300;
    float fieldHeight = 40;
    float fieldX = dialogX + (dialogWidth - fieldWidth) / 2; // Center horizontally
    float fieldY = dialogY + 110;

    iSetColor(255, 255, 255);
    iFilledRectangle(fieldX, fieldY, fieldWidth, fieldHeight);
    
    // Hover effect for the text field border
    if (mouse_x >= fieldX && mouse_x <= fieldX + fieldWidth &&
        mouse_y >= fieldY && mouse_y <= fieldY + fieldHeight) {
        iSetColor(0, 150, 255); // Bright blue border
    } else {
        iSetColor(100, 100, 100); // Normal gray border
    }
    iRectangle(fieldX, fieldY, fieldWidth, fieldHeight);

    // Draw the typed name
    iSetColor(0, 0, 0);
    iText(fieldX + 10, fieldY + 10, playerNameInput, GLUT_BITMAP_HELVETICA_18);

    // --- Submit Button (Positioned inside the dialog) ---
    float buttonWidth = 150;
    float buttonHeight = 50;
    float buttonX = dialogX + (dialogWidth - buttonWidth) / 2; // Center horizontally
    float buttonY = dialogY + 40;

    // Hover effect for the submit button
    if (mouse_x >= buttonX && mouse_x <= buttonX + buttonWidth &&
        mouse_y >= buttonY && mouse_y <= buttonY + buttonHeight) {
        iSetColor(50, 220, 100); // Brighter green
    } else {
        iSetColor(0, 180, 50);   // Normal green
    }
    iFilledRectangle(buttonX, buttonY, buttonWidth, buttonHeight);
    iSetColor(255, 255, 255);
    iText(buttonX + 45, buttonY + 15, "SUBMIT", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawLeaderboardScreen() {
    iShowImage(0, 0, "images/landingpage.png");
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 60, "LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24);

    int startX = 100;
    int startY = SCREEN_HEIGHT - 120;
    char scoreEntry[100];

    // --- Draw Level 1 Scores ---
    iSetColor(255, 255, 0); // Yellow
    iText(startX, startY, "--- LEVEL 1 ---", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255); // White
    for (size_t i = 0; i < leaderboard_level1.size() && i < 5; ++i) { // Show top 5
        sprintf(scoreEntry, "%2d. %-15s %d", (int)i + 1, leaderboard_level1[i].name, leaderboard_level1[i].score);
        iText(startX, startY - ( (i + 1) * 25 ), scoreEntry, GLUT_BITMAP_HELVETICA_18);
    }

    // --- Draw Level 2 Scores ---
    startX = 400; // Move to middle column
    iSetColor(255, 255, 0);
    iText(startX, startY, "--- LEVEL 2 ---", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255);
    for (size_t i = 0; i < leaderboard_level2.size() && i < 5; ++i) {
        sprintf(scoreEntry, "%2d. %-15s %d", (int)i + 1, leaderboard_level2[i].name, leaderboard_level2[i].score);
        iText(startX, startY - ( (i + 1) * 25 ), scoreEntry, GLUT_BITMAP_HELVETICA_18);
    }

    // --- Draw Level 3 Scores ---
    startX = 700; // Move to right column
    iSetColor(255, 255, 0);
    iText(startX, startY, "--- LEVEL 3 ---", GLUT_BITMAP_HELVETICA_18);
    iSetColor(255, 255, 255);
    for (size_t i = 0; i < leaderboard_level3.size() && i < 5; ++i) {
        sprintf(scoreEntry, "%2d. %-15s %d", (int)i + 1, leaderboard_level3[i].name, leaderboard_level3[i].score);
        iText(startX, startY - ( (i + 1) * 25 ), scoreEntry, GLUT_BITMAP_HELVETICA_18);
    }

    // Back to Main Menu Button
    iSetColor(100, 100, 100);
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
    iShowImage(0, 0, "images/landingpage.png");

    // --- PLAY Button ---
    if (mouse_x >= MENU_BUTTON_X && mouse_x <= MENU_BUTTON_X + MENU_BUTTON_WIDTH &&
        mouse_y >= PLAY_BUTTON_Y && mouse_y <= PLAY_BUTTON_Y + 50) {
        iSetColor(50, 220, 100); // Hover
    } else {
        iSetColor(0, 180, 50);   // Normal
    }
    iFilledRectangle(MENU_BUTTON_X, PLAY_BUTTON_Y, MENU_BUTTON_WIDTH, 50);
    iSetColor(255, 255, 255);
    iText(MENU_BUTTON_X + 95, PLAY_BUTTON_Y + 18, "PLAY", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- HELP Button ---
    if (mouse_x >= MENU_BUTTON_X && mouse_x <= MENU_BUTTON_X + MENU_BUTTON_WIDTH &&
        mouse_y >= HELP_BUTTON_Y && mouse_y <= HELP_BUTTON_Y + 50) {
        iSetColor(50, 150, 220); // Hover
    } else {
        iSetColor(0, 100, 180);   // Normal
    }
    iFilledRectangle(MENU_BUTTON_X, HELP_BUTTON_Y, MENU_BUTTON_WIDTH, 50);
    iSetColor(255, 255, 255);
    iText(MENU_BUTTON_X + 95, HELP_BUTTON_Y + 18, "HELP", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- LEADERBOARD Button ---
    if (mouse_x >= MENU_BUTTON_X && mouse_x <= MENU_BUTTON_X + MENU_BUTTON_WIDTH &&
        mouse_y >= LEADERBOARD_BUTTON_Y && mouse_y <= LEADERBOARD_BUTTON_Y + 50) {
        iSetColor(220, 220, 50); // Hover
    } else {
        iSetColor(180, 180, 0);   // Normal
    }
    iFilledRectangle(MENU_BUTTON_X, LEADERBOARD_BUTTON_Y, MENU_BUTTON_WIDTH, 50);
    iSetColor(255, 255, 255);
    iText(MENU_BUTTON_X + 60, LEADERBOARD_BUTTON_Y + 18, "LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- SETTINGS Button ---
    if (mouse_x >= MENU_BUTTON_X && mouse_x <= MENU_BUTTON_X + MENU_BUTTON_WIDTH &&
        mouse_y >= SETTINGS_BUTTON_Y && mouse_y <= SETTINGS_BUTTON_Y + 50) {
        iSetColor(150, 150, 150); // Hover (Light Gray)
    } else {
        iSetColor(100, 100, 100);   // Normal (Dark Gray)
    }
    iFilledRectangle(MENU_BUTTON_X, SETTINGS_BUTTON_Y, MENU_BUTTON_WIDTH, 50);
    iSetColor(255, 255, 255);
    iText(MENU_BUTTON_X + 75, SETTINGS_BUTTON_Y + 18, "SETTINGS", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- EXIT Button ---
    if (mouse_x >= MENU_BUTTON_X && mouse_x <= MENU_BUTTON_X + MENU_BUTTON_WIDTH &&
        mouse_y >= EXIT_BUTTON_Y && mouse_y <= EXIT_BUTTON_Y + 50) {
        iSetColor(220, 100, 50); // Hover
    } else {
        iSetColor(180, 50, 0);    // Normal
    }
    iFilledRectangle(MENU_BUTTON_X, EXIT_BUTTON_Y, MENU_BUTTON_WIDTH, 50);
    iSetColor(255, 255, 255);
    iText(MENU_BUTTON_X + 95, EXIT_BUTTON_Y + 18, "EXIT", GLUT_BITMAP_TIMES_ROMAN_24);
}
void drawSettingsScreen() {
    // First, draw the main menu in the background to create an overlay effect
    drawTitleScreen(); 

    // --- Draw the Settings Dialog Box ---
    int dialogWidth = 400;
    int dialogHeight = 250;
    int dialogX = SCREEN_WIDTH / 2 - dialogWidth / 2;
    int dialogY = SCREEN_HEIGHT / 2 - dialogHeight / 2;

    // Draw the box background and border
    iSetColor(50, 50, 70); // Dark blue-gray
    iFilledRectangle(dialogX, dialogY, dialogWidth, dialogHeight);
    iSetColor(200, 200, 220); // Light lavender border
    iRectangle(dialogX, dialogY, dialogWidth, dialogHeight);

    iSetColor(255, 255, 255);
    iText(dialogX + 150, dialogY + 200, "SETTINGS", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Music ON/OFF Button ---
    int musicButtonX = dialogX + 50;
    int musicButtonY = dialogY + 120;
    iSetColor(0, 150, 200);
    iFilledRectangle(musicButtonX, musicButtonY, 300, 50);
    iSetColor(255, 255, 255);
    // Display "MUSIC ON" or "MUSIC OFF" based on the isMusicEnabled variable
    const char* musicText = isMusicEnabled ? "MUSIC: ON" : "MUSIC: OFF";
    iText(musicButtonX + 100, musicButtonY + 18, musicText, GLUT_BITMAP_HELVETICA_18);

    // --- Sound Effects ON/OFF Button ---
    int soundButtonX = dialogX + 50;
    int soundButtonY = dialogY + 50;
    iSetColor(0, 150, 200);
    iFilledRectangle(soundButtonX, soundButtonY, 300, 50);
    iSetColor(255, 255, 255);
    // Display "SOUNDS ON" or "SOUNDS OFF" based on the isSoundEffectsEnabled variable
    const char* soundText = isSoundEffectsEnabled ? "SOUNDS: ON" : "SOUNDS: OFF";
    iText(soundButtonX + 95, soundButtonY + 18, soundText, GLUT_BITMAP_HELVETICA_18);
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
    iShowImage(0, 0, "images/landingpage.png");

    // --- Screen Title ---
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT - 100, "SELECT A LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Define Button Layout Locally ---
    float button_x = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
    float button_height = 50;
    
    float level1_y = SCREEN_HEIGHT / 2 + 80;
    float level2_y = SCREEN_HEIGHT / 2;
    float level3_y = SCREEN_HEIGHT / 2 - 80;
    float back_y = 50;

    // --- Level 1 Button ---
    if (mouse_x >= button_x && mouse_x <= button_x + MENU_BUTTON_WIDTH && mouse_y >= level1_y && mouse_y <= level1_y + button_height) {
        iSetColor(50, 220, 100); // Hover
    } else {
        iSetColor(0, 180, 50);   // Normal
    }
    iFilledRectangle(button_x, level1_y, MENU_BUTTON_WIDTH, button_height);
    iSetColor(255, 255, 255);
    iText(button_x + 85, level1_y + 18, "LEVEL 1", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Level 2 Button ---
    if (mouse_x >= button_x && mouse_x <= button_x + MENU_BUTTON_WIDTH && mouse_y >= level2_y && mouse_y <= level2_y + button_height) {
        iSetColor(50, 150, 220); // Hover
    } else {
        iSetColor(0, 100, 180);   // Normal
    }
    iFilledRectangle(button_x, level2_y, MENU_BUTTON_WIDTH, button_height);
    iSetColor(255, 255, 255);
    iText(button_x + 85, level2_y + 18, "LEVEL 2", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Level 3 Button ---
    if (mouse_x >= button_x && mouse_x <= button_x + MENU_BUTTON_WIDTH && mouse_y >= level3_y && mouse_y <= level3_y + button_height) {
        iSetColor(220, 100, 50); // Hover
    } else {
        iSetColor(180, 50, 0);    // Normal
    }
    iFilledRectangle(button_x, level3_y, MENU_BUTTON_WIDTH, button_height);
    iSetColor(255, 255, 255);
    iText(button_x + 85, level3_y + 18, "LEVEL 3", GLUT_BITMAP_TIMES_ROMAN_24);

    // --- Back to Menu Button ---
    if (mouse_x >= button_x && mouse_x <= button_x + MENU_BUTTON_WIDTH && mouse_y >= back_y && mouse_y <= back_y + button_height) {
        iSetColor(150, 150, 150); // Hover
    } else {
        iSetColor(100, 100, 100);   // Normal
    }
    iFilledRectangle(button_x, back_y, MENU_BUTTON_WIDTH, button_height);
    iSetColor(255, 255, 255);
    iText(button_x + 55, back_y + 18, "BACK TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);
}
void drawPauseMenu() {
    iShowImage(0, 0, "images/landingpage.png");
    iSetColor(255, 255, 255);
    iText(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 90, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

    float resume_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
    float resume_btn_y = SCREEN_HEIGHT / 2;
    float menu_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
    float menu_btn_y = SCREEN_HEIGHT / 2 - 150;

    // Resume Button
    if (mouse_x >= resume_btn_x && mouse_x <= resume_btn_x + BUTTON_WIDTH && mouse_y >= resume_btn_y && mouse_y <= resume_btn_y + BUTTON_HEIGHT) {
        iSetColor(50, 150, 220);
    } else {
        iSetColor(0, 100, 180);
    }
    iFilledRectangle(resume_btn_x, resume_btn_y, BUTTON_WIDTH, BUTTON_HEIGHT);
    iSetColor(255, 255, 255);
    iText(resume_btn_x + 100, resume_btn_y + 40, "Resume", GLUT_BITMAP_TIMES_ROMAN_24);

    // Main Menu Button
    if (mouse_x >= menu_btn_x && mouse_x <= menu_btn_x + BUTTON_WIDTH && mouse_y >= menu_btn_y && mouse_y <= menu_btn_y + BUTTON_HEIGHT) {
        iSetColor(220, 100, 50);
    } else {
        iSetColor(180, 50, 0);
    }
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
        case STATE_SETTINGS: // <-- ADD THIS
            drawSettingsScreen();
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
                else if (currentTile.id == SPIKE1) {
                    iShowImage(drawX, drawY, "images/spike_1.png");
                }
                else if (currentTile.id == SPIKE2) {
                    iShowImage(drawX, drawY, "images/spike_2.png");
                }
                else if (currentTile.id == RING1) {
                    iShowImage(drawX, drawY, "images/ring1.png");
                }
                else if (currentTile.id == RING2) {
                    iShowImage(drawX, drawY, "images/ring2.png");
                }
                else if (currentTile.id == THING) {
                    iShowImage(drawX, drawY, "images/thing.png");
                }
              else if (levelTiles[i].id == MOVING_ARROW) {
                   iShowImage(levelTiles[i].x - cameraX, levelTiles[i].y - cameraY, "images/arrow.png");

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

                else if (currentTile.id == SPIKE1) {
                    iShowImage(drawX, drawY, "images/spike_1.png");
                }
                
                else if (currentTile.id == SPIKE2) {
                    iShowImage(drawX, drawY, "images/spike_2.png");
                }
                else if (currentTile.id == RING1) {
                    iShowImage(drawX, drawY, "images/ring1.png");
                }
                else if (currentTile.id == RING2) {
                    iShowImage(drawX, drawY, "images/ring2.png");
                }
                 else if (currentTile.id == THING) {
                    iShowImage(drawX, drawY, "images/thing.png");
                }
                else if (levelTiles[i].id == MOVING_ARROW) {
                  iShowImage(levelTiles[i].x - cameraX, levelTiles[i].y - cameraY, "images/arrow.png");

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
    // This will update the global variables every time the mouse moves
    mouse_x = mx;
    mouse_y = my;
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
            
            gameState = STATE_TITLE;
            // Start music only if it's enabled and not already playing
            if (isMusicEnabled && musicChannel == -1) {
                musicChannel = iPlaySound("music/game_music.wav", true, 80);
            }
        } 
        else if (gameState == STATE_TITLE) {
            // --- UPDATED: Logic for the 5 main menu buttons ---
            if (mx >= MENU_BUTTON_X && mx <= MENU_BUTTON_X + MENU_BUTTON_WIDTH && my >= PLAY_BUTTON_Y && my <= PLAY_BUTTON_Y + 50) {
                if(isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
                gameState = STATE_ENTER_NAME;
                playerNameInput[0] = '\0';
                playerNameIndex = 0;
            } 
            else if (mx >= MENU_BUTTON_X && mx <= MENU_BUTTON_X + MENU_BUTTON_WIDTH && my >= HELP_BUTTON_Y && my <= HELP_BUTTON_Y + 50) {
                if(isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
                gameState = STATE_HELP_ADVENTURE;
            } 
            else if (mx >= MENU_BUTTON_X && mx <= MENU_BUTTON_X + MENU_BUTTON_WIDTH && my >= LEADERBOARD_BUTTON_Y && my <= LEADERBOARD_BUTTON_Y + 50) {
                if(isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
                loadLeaderboard();
                gameState = STATE_LEADERBOARD;
            } 
            else if (mx >= MENU_BUTTON_X && mx <= MENU_BUTTON_X + MENU_BUTTON_WIDTH && my >= SETTINGS_BUTTON_Y && my <= SETTINGS_BUTTON_Y + 50) {
                if(isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
                gameState = STATE_SETTINGS;
            }
            else if (mx >= MENU_BUTTON_X && mx <= MENU_BUTTON_X + MENU_BUTTON_WIDTH && my >= EXIT_BUTTON_Y && my <= EXIT_BUTTON_Y + 50) {
                if(isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
                gameState = STATE_QUIT_CONFIRM;
            }
        }
        else if (gameState == STATE_SETTINGS) {
    // Define the settings dialog box area
    int dialogWidth = 400;
    int dialogHeight = 250;
    int dialogX = SCREEN_WIDTH / 2 - dialogWidth / 2;
    int dialogY = SCREEN_HEIGHT / 2 - dialogHeight / 2;

    // --- NEW: Check if the click is OUTSIDE the dialog box ---
    if (mx < dialogX || mx > dialogX + dialogWidth || my < dialogY || my > dialogY + dialogHeight) {
        // If the click is outside, close the settings by returning to the main menu
        gameState = STATE_TITLE;
    } 
    else {
        // --- The click was INSIDE the box, so check which button was pressed ---
        int musicButtonX = dialogX + 50;
        int musicButtonY = dialogY + 120;
        int soundButtonX = dialogX + 50;
        int soundButtonY = dialogY + 50;

        // Music ON/OFF Button
        if (mx >= musicButtonX && mx <= musicButtonX + 300 && my >= musicButtonY && my <= musicButtonY + 50) {
            isMusicEnabled = !isMusicEnabled;
            if (isMusicEnabled) {
                iResumeSound(musicChannel);
            } else {
                iPauseSound(musicChannel);
            }
        }
        // Sound Effects ON/OFF Button
        else if (mx >= soundButtonX && mx <= soundButtonX + 300 && my >= soundButtonY && my <= soundButtonY + 50) {
            isSoundEffectsEnabled = !isSoundEffectsEnabled;
        }
    }
}


else if (gameState == STATE_LEVEL_SELECT) {
    // --- Define Button Layout Locally to Match Drawing ---
    float button_x = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
    float button_height = 50;

    float level1_y = SCREEN_HEIGHT / 2 + 80;
    float level2_y = SCREEN_HEIGHT / 2;
    float level3_y = SCREEN_HEIGHT / 2 - 80;
    float back_y = 50;

    // Level 1 Button Click
    if (mx >= button_x && mx <= button_x + MENU_BUTTON_WIDTH && my >= level1_y && my <= level1_y + button_height) {
        currentLevel = 1;
        loadLevel(currentLevel);
        gameState = STATE_PLAYING;
        if (isMusicEnabled) iResumeSound(musicChannel);
    }
    // Level 2 Button Click
    else if (mx >= button_x && mx <= button_x + MENU_BUTTON_WIDTH && my >= level2_y && my <= level2_y + button_height) {
        currentLevel = 2;
        loadLevel(currentLevel);
        gameState = STATE_PLAYING;
        if (isMusicEnabled) iResumeSound(musicChannel);
    }
    // Level 3 Button Click
    else if (mx >= button_x && mx <= button_x + MENU_BUTTON_WIDTH && my >= level3_y && my <= level3_y + button_height) {
        currentLevel = 3;
        loadLevel(currentLevel);
        gameState = STATE_PLAYING;
        if (isMusicEnabled) iResumeSound(musicChannel);
    }
    // Back to Menu Button Click
    else if (mx >= button_x && mx <= button_x + MENU_BUTTON_WIDTH && my >= back_y && my <= back_y + button_height) {
        gameState = STATE_TITLE;
        if (isSoundEffectsEnabled) iPlaySound("music/click.wav", false);
    }
}
        else if (gameState == STATE_PLAYING) {
            if (mx >= PAUSE_BUTTON_X && mx <= PAUSE_BUTTON_X + PAUSE_BUTTON_SIZE &&
                my >= PAUSE_BUTTON_Y && my <= PAUSE_BUTTON_Y + PAUSE_BUTTON_SIZE) {
                gameState = STATE_PAUSED;
                // iPauseSound(musicChannel);
            }
        }
        else if (gameState == STATE_PAUSED) {
            float resume_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
            float resume_btn_y = SCREEN_HEIGHT / 2;
            float menu_btn_x = SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2;
            float menu_btn_y = SCREEN_HEIGHT / 2 - 150;

            if (mx >= resume_btn_x && mx <= resume_btn_x + BUTTON_WIDTH && my >= resume_btn_y && my <= resume_btn_y + BUTTON_HEIGHT) {
                gameState = STATE_PLAYING;
                if (isMusicEnabled) iResumeSound(musicChannel);
            }
            if (mx >= menu_btn_x && mx <= menu_btn_x + BUTTON_WIDTH && my >= menu_btn_y && my <= menu_btn_y + BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
        else if (gameState == STATE_HELP_ADVENTURE) {
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 && mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 50 && my <= 50 + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
        else if (gameState == STATE_QUIT_CONFIRM) {
            int dialogWidth = 600, dialogHeight = 250;
            int dialogX = (SCREEN_WIDTH / 2) - (dialogWidth / 2);
            int dialogY = (SCREEN_HEIGHT / 2) - (dialogHeight / 2);
            int buttonWidth = 150, buttonHeight = 60, buttonPadding = 50;
            int yesButtonX = dialogX + (dialogWidth / 2) - buttonWidth - (buttonPadding / 2);
            int yesButtonY = dialogY + 40;
            int noButtonX = dialogX + (dialogWidth / 2) + (buttonPadding / 2);
            int noButtonY = dialogY + 40;

            if (mx >= yesButtonX && mx <= yesButtonX + buttonWidth && my >= yesButtonY && my <= yesButtonY + buttonHeight) {
                exit(0);
            }
            else if (mx >= noButtonX && mx <= noButtonX + buttonWidth && my >= noButtonY && my <= noButtonY + buttonHeight) {
                gameState = STATE_TITLE;
            }
        }
        else if (gameState == STATE_GAME_OVER) {
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 &&
                mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 100 && my <= 100 + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
        else if (gameState == STATE_LEVEL_COMPLETED) {
            float buttonX = SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2;
            float nextLevelY = SCREEN_HEIGHT / 2 + 20;
            float restartY = SCREEN_HEIGHT / 2 - 100;
            float mainMenuY = SCREEN_HEIGHT / 2 - 220;

            if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH && my >= nextLevelY && my <= nextLevelY + MENU_BUTTON_HEIGHT) {
                currentLevel++;
                int maxLevels = 3;
                if (currentLevel > maxLevels) {
                    gameState = STATE_TITLE;
                } else {
                    loadLevel(currentLevel);
                    gameState = STATE_PLAYING;
                    if(isMusicEnabled) iResumeSound(musicChannel);
                }
            }
            else if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH && my >= restartY && my <= restartY + MENU_BUTTON_HEIGHT) {
                loadLevel(currentLevel);
                gameState = STATE_PLAYING;
                if(isMusicEnabled) iResumeSound(musicChannel);
            }
            else if (mx >= buttonX && mx <= buttonX + MENU_BUTTON_WIDTH && my >= mainMenuY && my <= mainMenuY + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
            }
        }
       else if (gameState == STATE_ENTER_NAME) {
    // --- Define coordinates that match the new drawing function ---
    int dialogWidth = 400;
    int dialogX = SCREEN_WIDTH / 2 - dialogWidth / 2;
    int dialogY = SCREEN_HEIGHT / 2 - 125;
    
    float buttonWidth = 150;
    float buttonHeight = 50;
    float buttonX = dialogX + (dialogWidth - buttonWidth) / 2;
    float buttonY = dialogY + 40; // This is an estimate, adjust if needed based on the Y from draw function

    // Check for "Submit" button click using the new coordinates
    if (mx >= buttonX && mx <= buttonX + buttonWidth &&
        my >= buttonY && my <= buttonY + buttonHeight) {
        
        // if (isSoundEffectsEnabled) iPlaySound("music/click.wav", false);

        if (playerNameIndex > 0) {
            strcpy(currentPlayerName, playerNameInput);
        } else {
            strcpy(currentPlayerName, "Guest");
        }
        gameState = STATE_LEVEL_SELECT;
    }
}
        else if (gameState == STATE_LEADERBOARD) {
            if (mx >= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 && mx <= SCREEN_WIDTH / 2 - MENU_BUTTON_WIDTH / 2 + MENU_BUTTON_WIDTH &&
                my >= 50 && my <= 50 + MENU_BUTTON_HEIGHT) {
                gameState = STATE_TITLE;
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
    // if (gameState == STATE_PLAYING && (state == 0 || state == 2)) {
    //     if (key == 'a') {
    //         speedX -= accelerationX;
    //         rotationFrame = (rotationFrame - 3 + TOTAL_FRAMES) % TOTAL_FRAMES;
    //     }
    //     if (key == 'd') {
    //         speedX += accelerationX;
    //         rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES;
    //     }
    //     if ((key == 'w' || key == ' ') && onGround) {
    //         vy = 13;
    //         rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES;
    //         onGround = false;
    //     }
    // }

    // Typing name should allow only state == 0 (first press)
    if (gameState == STATE_ENTER_NAME && state == 0) {
    if (key == '\b') { // Backspace
        if (playerNameIndex > 0) {
            playerNameIndex--;
            playerNameInput[playerNameIndex] = '\0';
        }
    } 
    else if (key == '\r') { // --- THIS IS THE CORRECTED LOGIC FOR THE ENTER KEY ---
        if (playerNameIndex > 0) {
            // Just like the mouse click, copy the input to our player name variable
            strcpy(currentPlayerName, playerNameInput);
        } else {
            // Use a default name if nothing was typed
            strcpy(currentPlayerName, "Guest");
        }

        // Clear the input field for the next time the game starts
        playerNameInput[0] = '\0';
        playerNameIndex = 0;
        
        // Proceed to the level selection screen
        gameState = STATE_LEVEL_SELECT;
    } 
    else if (playerNameIndex < sizeof(playerNameInput) - 1) {
        // Allow typing letters, numbers, and spaces
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
                
            }
        }

        if (key == 27) { // Escape
            exit(0);
        }
    }
}




void updateBall() {
    // --- 1. Apply basic physics (gravity and friction) ---
    if (isKeyPressed('a')) {
        speedX -= accelerationX;
        rotationFrame = (rotationFrame - 3 + TOTAL_FRAMES) % TOTAL_FRAMES;
    }
    if (isKeyPressed('d')) {
        speedX += accelerationX;
        rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES;
    }
    if ((isKeyPressed('w') || isKeyPressed(' ')) && onGround) {
        vy = 13; // or jumpStrength
        onGround = false;
        // You can keep the rotation on jump if you like
        // rotationFrame = (rotationFrame + 3) % TOTAL_FRAMES; 
    }

    if (speedX > MAX_SPEED_X) {
        speedX = MAX_SPEED_X;
    }
    if (speedX < -MAX_SPEED_X) { // Don't forget the negative direction!
        speedX = -MAX_SPEED_X;
    }

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
        if (!tile.isSolid) continue; // Skip non-solid tiles

        // Check for an overlap between the ball and the solid tile
        if (ballX + ballWidth > tile.x && ballX < tile.x + tile.width &&
            ballY + ballHeight > tile.y && ballY < tile.y + tile.height) {

            // CORRECTED: Check for level completion with consistent logic
            if (tile.id == RED_FINISH_POINT) {
                int requiredRings = (int)ceil(totalRingsInLevel * 0.70);
                if (ringsCollected >= requiredRings) {
                    printf("Level %d Completed! (%d/%d rings)\n", currentLevel, ringsCollected, totalRingsInLevel);
                    addScoreToLeaderboard(currentLevel, currentPlayerName, currentScore);
                    gameState = STATE_LEVEL_COMPLETED;
                    speedX = 0;
                    vy = 0;
                    return; // Exit the function early
                }
            }

            // If it wasn't the finish line, treat it as a normal wall
            if (speedX > 0) ballX = tile.x - ballWidth;
            else if (speedX < 0) ballX = tile.x + tile.width;
            
            speedX = 0;
        }
    }
    float score_interval = 20 * BASE_TILE_WIDTH;
    if (ballX > lastScoringPositionX + score_interval) {
        currentScore += 50;
        lastScoringPositionX = ballX;
        printf("Score increased to: %d\n", currentScore);
    }

    // --- 3. Handle Vertical Movement and Collision ---
    onGround = false;
    ballY += vy;

    for (int i = 0; i < numActualTiles; ++i) {
        struct Tile tile = levelTiles[i];
        if (!tile.isSolid) continue;

        if (ballX + ballWidth > tile.x && ballX < tile.x + tile.width &&
            ballY + ballHeight > tile.y && ballY < tile.y + tile.height) {
            
            // CORRECTED: Check for level completion with the same consistent logic
            if (tile.id == RED_FINISH_POINT) {
                int requiredRings = (int)ceil(totalRingsInLevel * 0.70);
                if (ringsCollected >= requiredRings) {
                    printf("Level %d Completed! (%d/%d rings)\n", currentLevel, ringsCollected, totalRingsInLevel);
                    addScoreToLeaderboard(currentLevel, currentPlayerName, currentScore);
                    gameState = STATE_LEVEL_COMPLETED;
                    speedX = 0;
                    vy = 0;
                    return; // Exit the function early
                }
            }

            // If not the finish line, resolve vertical collision
            if (vy > 0) ballY = tile.y - ballHeight;
            else {
                ballY = tile.y + tile.height;
                onGround = true;
            }
            vy = 0;
        }
    }

    // --- 4. Ring and Spike Collision ---
    for (int i = 0; i < numActualTiles; i++) {
        if (levelTiles[i].id == RING1 || levelTiles[i].id == RING2 || levelTiles[i].id == SPIKE1 || levelTiles[i].id == SPIKE2 || levelTiles[i].id == MOVING_ARROW) {
            if (ballX + ballWidth > levelTiles[i].x && ballX < levelTiles[i].x + levelTiles[i].width &&
                ballY + ballHeight > levelTiles[i].y && ballY < levelTiles[i].y + levelTiles[i].height) {

                if (levelTiles[i].id == RING1 || levelTiles[i].id == RING2) {
                    levelTiles[i].id = EMPTY;
                    ringsCollected++;
                    currentScore += 100; // Give points for collecting a ring
                    printf("Ring collected! Total: %d/%d\n", ringsCollected, totalRingsInLevel);
                    if (isSoundEffectsEnabled){
                        iPlaySound("music\\ring_collect.wav", false);
                    }
                } 
                else if (levelTiles[i].id == SPIKE1 || levelTiles[i].id == SPIKE2 || levelTiles[i].id == MOVING_ARROW) {
                    printf("Ouch! Hit a spike.\n");
                    if (isSoundEffectsEnabled){
                        iPlaySound("music\\spikehit.wav", false);
                    }
                    ballLives--;

                    if (ballLives > 0) {
                        resetBallPosition(); 
                    } 
                    else {
                        addScoreToLeaderboard(currentLevel, currentPlayerName, currentScore);
                        gameState = STATE_GAME_OVER;
                        iPlaySound("music\\gameover.wav", false);
                    }
                }
            }
        }
    }

    // --- 5. World Boundary and Camera Logic ---
    float levelActualWidth = NUM_FLOOR_TILES * BASE_TILE_WIDTH;
    if (ballX < 0) { ballX = 0; if (speedX < 0) speedX = 0; }
    if (ballX + ballWidth > levelActualWidth) { ballX = levelActualWidth - ballWidth; if (speedX > 0) speedX = 0; }

    if (ballY < -100) { // If the ball falls out of the world
        ballLives--;
        if(ballLives > 0) {
            resetBallPosition();
        } else {
            addScoreToLeaderboard(currentLevel, currentPlayerName, currentScore);
            gameState = STATE_GAME_OVER;
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

    // In updateBall(), at the end...
if (ballY < -100) { // If the ball falls out of the world
    ballLives--;
    if (isSoundEffectsEnabled) iPlaySound("music/hurt.wav", false);

    if (ballLives > 0) {
        // --- THIS IS THE CHANGE: Go to the absolute start of the level ---
        if (currentLevel == 1) {
            ballX = 100.0f;
            ballY = BASE_TILE_HEIGHT + 1;
        } else if (currentLevel == 2) {
            ballX = 7 * BASE_TILE_WIDTH;
            ballY = 7 * BASE_TILE_HEIGHT;
        } else if (currentLevel == 3) {
            ballX = 54 * BASE_TILE_WIDTH;
            ballY = BASE_TILE_HEIGHT * 44;
        }
        // Reset velocity and scoring tracker
        speedX = 0;
        vy = 0;
        lastScoringPositionX = ballX;
    } else {
        // Game Over logic
        addScoreToLeaderboard(currentLevel, currentPlayerName, currentScore);
        gameState = STATE_GAME_OVER;
        iStopSound(musicChannel);
        if (isSoundEffectsEnabled) iPlaySound("music/game_over.wav", false);
    }
}
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
int main(int argc, char *argv[]) {
    // --- All setup and loading should happen BEFORE the game window opens ---
    glutInit(&argc, argv);
    // 1. Initialize the sound system (This is the missing step)
    iInitializeSound(); 

    // 2. Initialize player name buffers
    playerNameInput[0] = '\0';
    playerNameIndex = 0;
    currentPlayerName[0] = '\0';

    // 3. Load game assets and data
    iLoadImage(&bgPlayingImage, "images/landingpage.png");
    loadBallImageNames();
    loadLeaderboard(); 

    // 4. Set the game's timers
    iSetTimer(10, updateAllMovingArrows);
    iSetTimer(10, gameLoop);

    // 5. Start the iGraphics engine. This creates the window and starts the main loop.
    iOpenWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Adventure Ball");

    return 0;
}