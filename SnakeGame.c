//------------------------------------------------------------------
// Snake game
// 31.1.2024 Juha Koivukorpi
// Opikoodia course c-project      
//------------------------------------------------------------------
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>

#define ToggleBoolean(b) ((b) ? (b = false) : (b = true));

const int FIELD_X_MIN = 2;  // movable area left side 
const int FIELD_X_MAX = 70;  // movable area right side
const int FIELD_Y_MIN = 2;  // movable area top side
const int FIELD_Y_MAX = 21;  // movable area bottom side
const int SNAKE_START_X = 35; // snake head start position x
const int SNAKE_START_Y = 15; // snake head start position y
const int SCORE_FACTOR = 50; // shown game score is internal counter x this 

COORD c = {0, 0}; // this is for function setCursorXY

struct GameComponent {
    int x;
    int y;
    char character;
    bool exists; // This is only used in food component.
};

struct Direction {
    int x;
    int y;
    char pressedKey;
};

struct HighScore {
    char initials[3];
    int score;
} highScoreTable[5];

// Introduction --------------------------------------------------
void setCursorXY (int, int);
void cursorVisible(bool);
bool loadHighScore();
void saveHighScore();
void cutString(char *, int);
void countDown(); // Countdown game start 
bool yesNoInput(); // ask player to press y or n
void readKeyboard(struct Direction*);
void gamePaused();
void initSnake(struct GameComponent*,int);
void drawSnake(struct GameComponent*,int);
void moveSnake(struct GameComponent*, struct Direction, int*, bool);
void setFood(struct GameComponent*, struct GameComponent*, int);
int  checkCollision(struct GameComponent*, int, struct GameComponent*);
void updateScore(int);
void checkHighScore(int);

void snakeGame();
void snakeMain();
// INT MAIN() FUNCTION IS HERE
void showMsgBox();
void highScoreMsgBox(); 
void gameOverMsgBox();
void hideMsgBox();
void drawPlayField();
void drawScoreTable();
void drawHighScoreTable();
void titleScreen();

//=================================================================
// Set cursor position (x,y) on screen. Origin (0,0) = left,top ----------
void setCursorXY (int x, int y) {
    c.X = x; c.Y = y; // Set X and Y coordinates
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
// Hide or show cursor --------------------------------------------
void cursorVisible(bool cursorVisible) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 10;
    info.bVisible = cursorVisible;
    SetConsoleCursorInfo(consoleHandle, &info);
}
//==================================================================

bool loadHighScore() {
    FILE *scoreFile;
    char line[10];
    int index = 0;
    scoreFile = fopen("snakescores.txt","r");
    if (scoreFile == NULL) {
        return false;
    }   
    while(fgets(line,sizeof line,scoreFile)!= NULL){ // read a line from a file  
        cutString(line,index); // index is for high score table 
        index++;
    }
    fclose(scoreFile);
    return true;
}

void saveHighScore() {
    FILE *scoreFile;
    scoreFile = NULL;

    scoreFile = fopen("snakescores.txt","w");
    for (int i = 0; i < 5; i++){
        fprintf(scoreFile, highScoreTable[i].initials);
        fprintf(scoreFile,"#%d\n", highScoreTable[i].score); 
    } 
    fclose(scoreFile);
}

void cutString(char *cutString, int index){ // index is for high score table
    char *retString;
    char cutCharacter[2] = "#"; // This character cuts the string

    retString = strtok(cutString, cutCharacter);
    strcpy(highScoreTable[index].initials,retString);
    retString = strtok(NULL, cutCharacter);
    highScoreTable[index].score = atoi(retString);
}

//==================================================================

void countDown(){
    setCursorXY(SNAKE_START_X-2,SNAKE_START_Y-4);
    printf("%s","Ready");
    Sleep(500);
    setCursorXY(SNAKE_START_X-2,SNAKE_START_Y-4);
    printf("%s","Steady");
    Sleep(500);
    setCursorXY(SNAKE_START_X-2,SNAKE_START_Y-4);
    printf("%s","  GO  ");
    Sleep(500);
    setCursorXY(SNAKE_START_X-2,SNAKE_START_Y-4);
    printf("%s","      ");
}


bool yesNoInput(){
    while (1) {
        char key_code = _getch(); // get pressed key
        if (key_code == 'y' || key_code == 'Y') return true; 
        if (key_code == 'n' || key_code == 'N') return false; 
    }
}

void readKeyboard(struct Direction *mDir) { 
    int x = mDir->x , y = mDir->y;
    char key_code = ' ';
    //getch();
    if (kbhit()) { 
        switch (getch()) { 
        case 75: // left arrow key
            x = - 1; y = 0; // direction = left
            break; 
        case 80: // down arrow key
            x = 0; y = 1; // direction = down
            break; 
        case 77: // right arrow key
            x = 1; y = 0; // direction = right
            break; 
        case 72: // up arrow key
            x = 0; y = -1;// direction = up
            break; 
        case 'a': 
            x = - 1; y = 0;
            break; 
        case 's': 
            x = 0; y = 1; 
            break; 
        case 'd': 
            x = 1; y = 0; 
            break; 
        case 'w': 
            x = 0; y = -1;
            break;
        case 'p':
            key_code = 'p'; // p for pause
            break; 
        case '0':
            key_code = '0'; // Developer mode
            break;
        } 
    }
    mDir->pressedKey = key_code;
    if (mDir->x == (-x) || mDir->y == (-y)) return; // if direction is inverted (backwards) do not change direction 
    mDir->x = x; mDir->y = y; 
} 

// game paused actions
void gamePaused(){
    char scrollText[12] = "GAME PAUSED ";
    char tempChar = ' ';
    while (!kbhit()) {  
        for (int i = 0;i < 12;i++){
            setCursorXY(77+i,14);
            printf("%c",scrollText[i]); // Print text. This works better than printf.
        }
        tempChar = scrollText[0]; // Save first character
        for (int i = 0; i < 12; i++){
            scrollText[i] = scrollText[i+1]; // Move characters left.
        }
        scrollText[11]= tempChar; // Move first character to last
        Sleep(200);
    };
    setCursorXY(77,14);
    printf("            "); // clear line
}

//================================================================================================================= 
// Functions for snake. .exists is never
//-----------------------------------------------------------------------------------------------------------------
// Set snakes coordinates and body characters
void initSnake(struct GameComponent *mySnake, int snakeL){
    for (int i = 0; i <= snakeL-1; i++) { // This loop allows to change snakes initial lenght
        mySnake[i].x = SNAKE_START_X; mySnake[i].y = SNAKE_START_Y+i; mySnake[i].character = 'O';
    } 
    // Last component of snake is empty space which also clears characters after snake
    mySnake[snakeL-1].character = ' ';
    // Last visible components of snake are smaller
    mySnake[snakeL-2].character = '.';
    mySnake[snakeL-3].character = 'o';
    mySnake[snakeL-4].character = 'o';
    mySnake[0].character = 153; // snakes head 
}

// Draw snake on screen 
void drawSnake(struct GameComponent *mySnake, int snakeL){
    for (int i = 0; i < snakeL; i++){
        setCursorXY(mySnake[i].x,mySnake[i].y);
        printf("%c", mySnake[i].character);
    }
}

// Move snakes body parts and finally head to new coordinates
void moveSnake(struct GameComponent *mySnake, struct Direction mDir, int *snakeL, bool extend){
    int lenght = *snakeL;
    // normal movement without extension
    if (extend == false) {  
        for (int i = lenght-1; i > 0; i--){
            mySnake[i].x = mySnake[i-1].x; // move snake parts from back to front
            mySnake[i].y = mySnake[i-1].y;
        }
        mySnake[0].x += mDir.x; mySnake[0].y += mDir.y;
    } 
    else {
        // extend snake and move only head
        lenght++;       
        for (int i = lenght-1; i > 1; i--){
            mySnake[i].x = mySnake[i-1].x; // move snake parts from back to front
            mySnake[i].y = mySnake[i-1].y;
            mySnake[i].character = mySnake[i-1].character;
        }
        int x = mySnake[0].x, y = mySnake[0].y; // snakes head position before moving
        mySnake[0].x += mDir.x; mySnake[0].y += mDir.y; // move head to a new position
        mySnake[1].x = x; mySnake[1].y = y; // bodypart before head gets its old position.
        *snakeL = lenght;  
    }
}

// Set food on random coordinates
void setFood(struct GameComponent *food, struct GameComponent *mySnake, int SnakeL){
    bool success;
    int x = 0, y = 0; 
    do {
        x = rand() % (FIELD_X_MAX - FIELD_X_MIN) + FIELD_X_MIN;
        y = rand() % (FIELD_Y_MAX - FIELD_Y_MIN) + FIELD_Y_MIN;
        success = true;
        // check if the position is empty (no snake on these coordinates)
        for (int i = 0; i < SnakeL; i++){
            if (x == mySnake[i].x && y == mySnake[i].y) { success = false; }
        }
    } while (!success);
    food->x = x; food->y = y; food->exists = true;
    setCursorXY(x,y);
    printf("%c", food->character);
}
//----------------------------------------------------------------------
// Checks collisions with borders and food and snake
// 0 = no collision
// 1 = collision with food
// 2 = collision with border
// 3 = collision with snakes own tail 
//----------------------------------------------------------------------
int checkCollision(struct GameComponent *mySnake, int snakeL, struct GameComponent *food){
    // Check hit food
    if (mySnake[0].x == food->x && mySnake[0].y == food->y) {
        food->exists = false;
        return 1; 
    }
    // Check borders
    if (mySnake[0].x > FIELD_X_MAX || mySnake[0].x < FIELD_X_MIN || mySnake[0].y > FIELD_Y_MAX || mySnake[0].y < FIELD_X_MIN){
        return 2;
    }
    // Check hit own tail
    for (int i = 3; i < snakeL; i++){
        if (mySnake[0].x == mySnake[i].x && mySnake[0].y == mySnake[i].y) {
            return 3;
        }
    }
    return 0;
}

void updateScore(int score){
    setCursorXY(82,14);
    printf("%d",score*SCORE_FACTOR);
}

void checkHighScore(int score){
    char inits[3] = "   ";
    int flushChar = 0;
    for (int i = 0; i < 5; i++){  // check if new score is higher than any in the high score table
        if ((score*SCORE_FACTOR) > highScoreTable[i].score){ // if it's bigger move other backwards from that point
            for (int j = 3; j >= i; j--){
                highScoreTable[j+1].score = highScoreTable[j].score;
                strcpy(highScoreTable[j+1].initials, highScoreTable[j].initials);
            }
            highScoreTable[i].score = (score*SCORE_FACTOR);
            highScoreMsgBox();      // show high score box
            setCursorXY(33,13);     // move cursor into the box
            cursorVisible(true);    // show cursor
            scanf("%3s", inits);    // ask initials
            cursorVisible(false);   // hide cursor
            strcpy(highScoreTable[i].initials, inits); // move initials to high score table
            drawHighScoreTable();   // redraw high score table
            saveHighScore();
            while ((flushChar = fgetc(stdin)) != '\n' && flushChar != EOF); /* Flush stdin */
            break;
        }
    }
}

//=================================================================================
void snakeGame (){
    int snakeCollided = 0;  // 0 = no collision. 1-3 tells where collided
    int extendSize = 0;     // used when snake grows up lenght. How many times will be extended
    bool extendSnake = false; // Works with extendSize
    int gameScore = 0;      // It's a game score
    bool developerMode = false; // Shows snakes lenght, delay time and snakes head coodrs in screen
    int gameDelay = 150; // Game delay is milliseconds
    bool pauseEnabled = true; // after pause, pause is disbled until other key that 'p' is pressed 
                              // without this you can't unpause with 'p' key (couldn't flush stdin)
    //------------------------------
    // Create snake and food
    //------------------------------
    struct GameComponent snakePart;
    int snakeLenght = 6; // initial snake lenght. Minimum is 4, where's 3 visible components
    struct GameComponent *snake;
    snake = malloc(snakeLenght * sizeof(snakePart)); 
    initSnake(snake, snakeLenght);

    struct GameComponent snakeFood;
    snakeFood.x = 0; snakeFood.y = 0; snakeFood.character = '@', snakeFood.exists = false;
    //------------------------------   
    // Draw play field
    //------------------------------   
    drawPlayField();
    drawSnake(snake,snakeLenght);
    struct Direction movingDir;
    movingDir.x = 0; movingDir.y = -1; movingDir.pressedKey = ' ';
    // Wait player ready
    showMsgBox(); 
    getch();
    hideMsgBox();
    countDown();
     
    while (1){ // Game loop
        readKeyboard(&movingDir);
            if (movingDir.pressedKey == 'p' && pauseEnabled == true) { pauseEnabled = false; gamePaused(); updateScore(gameScore);} // pause game
            if (movingDir.pressedKey != 'p') pauseEnabled = true;
            if (movingDir.pressedKey == '0') { ToggleBoolean(developerMode); movingDir.pressedKey = ' ';} // Developer mode
        
        moveSnake(snake,movingDir,&snakeLenght,extendSnake);
            if (extendSize > 0 ) {
                extendSize--;
                if (extendSize == 0) { extendSnake = false; } 
            }
        
        if (!snakeFood.exists){ setFood(&snakeFood, snake, snakeLenght);} // Add food if not exists

        snakeCollided = checkCollision(snake,snakeLenght,&snakeFood);       
            switch (snakeCollided)
            {
            case 0: // no collision
                break;
            case 1: // food
                snake = realloc(snake, (snakeLenght + 2) * sizeof(snakePart)); // Change the number here...
                extendSize += 2; extendSnake = true; // ...and also here. Both MUST be the same.
                if (gameDelay > 11) gameDelay -= 2;
                gameScore += 1;
                updateScore(gameScore);
                break;
            case 2: // border
                goto EndGame;
                break;  
            case 3: // own tail
                goto EndGame;
                break;  
            default:
                break;
            } 
        drawSnake(snake,snakeLenght);
        Sleep(gameDelay);

        // print information on screen
        if (developerMode){
            setCursorXY(93,1);
            printf("Lenght: %d. Delay(ms): %d", snakeLenght, gameDelay);
            setCursorXY(93,2);
            printf("Snakes head: %d,%d \n", snake[0].x,snake[0].y);
        }
    }
    EndGame:
    if (gameScore > 0) checkHighScore(gameScore);      
    free(snake);
}

//===============================================================================
// MAIN PROGRAM
//===============================================================================
void snakeMain() {
    bool newGame = true;
    int flushChar = 0;
    srand(time(NULL));
    system("cls"); // clear screen
    cursorVisible(false); // Hide cursor

    // Change console color to green
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
    
    // Show title screen
    titleScreen();
    getch();
    
    // Initialize high score
    for (int i = 0; i <= 5; i++){
        strcpy(highScoreTable[i].initials, "AAA");
        highScoreTable[i].score = 0;
    }
    if (!loadHighScore()){ // load saved high scores from a file, 
        saveHighScore();   // if file not found make a new one
    };

    do {
        system("cls"); // clear screen 
        snakeGame();
        gameOverMsgBox();
        newGame = yesNoInput();
    } while (newGame);
    saveHighScore();
    cursorVisible(true);
    setCursorXY(0,0);
    system("cls");
    // Change console color to white
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

int main() {
    
    snakeMain();

    return 0;
}
//===============================================================================

//-------------------------------------------------------------------------------
// Graphics are here
//-------------------------------------------------------------------------------
void showMsgBox() {
    int x = 22, y = 10;
    setCursorXY(x,y);
    printf("+--------------------------+");
    setCursorXY(x,y+1);
    printf("|   press a key to start   |");
    setCursorXY(x,y+2);
    printf("+--------------------------+");
}

void highScoreMsgBox() {
    int x = 22, y = 10;
    setCursorXY(x,y);
    printf("+--------------------------+");
    setCursorXY(x,y+1);
    printf("|        HIGH SCORE        |");
    setCursorXY(x,y+2);
    printf("|    ENTER YOU INITIALS    |");
    setCursorXY(x,y+3);
    printf("|                          |");
    setCursorXY(x,y+4);
    printf("+--------------------------+");
}

void gameOverMsgBox() {
    int x = 22, y = 10;
    setCursorXY(x,y);
    printf("+--------------------------+");
    setCursorXY(x,y+1);
    printf("|        GAME OVER         |");
    setCursorXY(x,y+2);
    printf("|      NEW GAME (Y/N)      |");
    setCursorXY(x,y+3);
    printf("+--------------------------+");
    setCursorXY(x,y+4);
    printf("                            ");
}

void hideMsgBox(){
    int x = 22, y = 10;
    setCursorXY(x,y);
    printf("                            ");
    setCursorXY(x,y+1);
    printf("                            ");
    setCursorXY(x,y+2);
    printf("                            ");
}

void drawPlayField(){
    setCursorXY(0,1);  
    printf(" #######################################################################\n");
        for (int i = 0; i < 20; i++){
    printf(" #                                                                     #\n");
    }
    printf(" #######################################################################\n");
    int x = 75, y = 17;
    setCursorXY(x,y);
    printf("#################");
    setCursorXY(x,y+1);
    printf("| Use arrow keys|");
    setCursorXY(x,y+2);
    printf("|  or WASD to   |");
    setCursorXY(x,y+3);
    printf("| control snake |");
    setCursorXY(x,y+4);
    printf("| P pauses game |");
    setCursorXY(x,y+5);
    printf("+---------------+");
    drawHighScoreTable();
    drawScoreTable();
}

void drawHighScoreTable() {
        int x = 75, y = 1;
        setCursorXY(x,y);
        printf("#################");
        setCursorXY(x,y+1);
        printf("|  HIGH SCORES  |");
        setCursorXY(x,y+2);
        printf("+---------------+");
        for (int i = 0; i <= 5; i++){
            setCursorXY(x,y+3+i);
            printf("|               |");
        }
        for (int i = 0; i <= 5; i++){
            setCursorXY(x+3,y+3+i);
            printf("%s",highScoreTable[i].initials);
            setCursorXY(x+9,y+3+i);
            printf("%d",highScoreTable[i].score);
        }
        setCursorXY(x,y+8);
        printf("+---------------+");
}

void drawScoreTable() {
        int x = 75, y=11;
        setCursorXY(x,y);
        printf("#################");
        setCursorXY(x,y+1);
        printf("|     SCORE     |");
        setCursorXY(x,y+2);
        printf("+---------------+");
        setCursorXY(x,y+3);
        printf("|      0        |");
        setCursorXY(x,y+4);
        printf("+---------------+");
}

void titleScreen(){
    // https://www.asciiart.eu/image-to-ascii
    system("cls"); // clear screen
    setCursorXY(0,0);
    printf("..............................................................................................\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf(".=============:................:=:....:==:......:==:............................-============.\n");
    printf(".============:.@@@@@@@@@.@@@@@@...@@@@.=:.@@@@@@.:=.@@@@@@@@.:@@@@@@@@@@@@@@@@@@-============.\n");
    printf(".============.@@#######@.@####@@@.@##@.=.@@####@@.=..@#####@.@@##@@.#@########@.-============.\n");
    printf(".============.@########@.@######@@@##@.=.@######@.:=.@####@+@@##@@...@########@.=============.\n");
    printf(".============.@@####@@@@.@###########@.:.@######@@.=.@####@.@##@@.:=.@####@#@@@.=============.\n");
    printf(".============:.@@@##@....@##########@@..@@@@@####@.=.@####@@@#@@.:==.@####@.....=============.\n");
    printf(".=============:..@@#@@.:.#@#@@@@####@-..@#@.@@###@.:.@########@..===.@####@@@@@:=============.\n");
    printf(".=============-....##@@.-.@#@..@@@@#@..@@#@..@###@@..@####@@@#@@..:=.@####@.....=============.\n");
    printf(".=============-@@@@##@@.=.@#@.:...@@@..@#@@@@@@###@..@####@.@@#@@@.=.@####@@@@@.=============.\n");
    printf(".=============-.@@@@@@.:=.@@@.===:..@.@@@@.@@.@@@@@@.@@@@@@..@@@@@.=.@@@@@@@@@@.=============.\n");
    printf(".==============:......:==:...:=====::......................::.....:=:..........:=============.\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf(".================================ PRESS A KEY TO CONTINUE ===================================.\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf(".============================================================================================.\n");
    printf("..............................................................................................\n");
}

