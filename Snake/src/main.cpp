#include <TFT_eSPI.h>
#include "Petme8x8.h"
#include <deque>
#include <cstdlib>
#include <ctime>

#define PURPLE 0x780F
#define GREEN 0x07E6
using namespace std;

#define BUTTON 13
#define DEBOUNCE_TIME 200

#define JOYSTICK_Y 32
#define JOYSTICK_X 33
#define JOYSTICK_DEADZONE 300
int JOYXCENTER, JOYYCENTER;

enum GameState{START, PLAYING, OVER};
enum Directions{UP, DOWN, LEFT, RIGHT, NONE};

GameState state = START;


static const unsigned char PROGMEM image_ButtonLeftSmall_bits[] = {0x00,0xf0,0x00,0xf0,0x00,0xf0,0x00,0xf0,0x0f,0xf0,0x0f,0xf0,0x0f,0xf0,0x0f,0xf0,0xff,0xf0,0xff,0xf0,0xff,0xf0,0xff,0xf0,0x0f,0xf0,0x0f,0xf0,0x0f,0xf0,0x0f,0xf0,0x00,0xf0,0x00,0xf0,0x00,0xf0,0x00,0xf0};
static const unsigned char PROGMEM image_ButtonRightSmall_bits[] = {0xf0,0x00,0xf0,0x00,0xf0,0x00,0xf0,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0xf0,0xff,0xf0,0xff,0xf0,0xff,0xf0,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xf0,0x00,0xf0,0x00,0xf0,0x00,0xf0,0x00};
TFT_eSPI tft = TFT_eSPI(); 

struct Vector2{
  int x, y;
  Vector2(int x, int y) : x(x), y(y) {}
};

const int cellSize = 10;
const int cellYCount = 13;
const int cellXCount = 24;

double lastUpdateTime = 0;
unsigned long lastButtonPress = 0;


Directions readInput(){
  int x = analogRead(JOYSTICK_X);
  int y = analogRead(JOYSTICK_Y);

  if (x < JOYXCENTER - JOYSTICK_DEADZONE) return RIGHT;
  if (y < JOYYCENTER - JOYSTICK_DEADZONE) return DOWN;
  if (x > JOYXCENTER + JOYSTICK_DEADZONE) return LEFT;
  if (y > JOYYCENTER + JOYSTICK_DEADZONE) return UP;
  return NONE;
}
bool buttonPressed(){
  if (digitalRead(BUTTON) == LOW &&millis() - lastButtonPress >= DEBOUNCE_TIME){
    lastButtonPress = millis();
    return true;
  }
  return false;
}

bool inDeque(Vector2 pos, const std::deque<Vector2> &deque){
  for (const Vector2 &v : deque){
    if (v.x == pos.x && v.y == pos.y){
      return true;
    }
  }
  return false;
}

class Food{
private:
  Vector2 GenerateRandomCell(){
    int minY = 2;
    int maxY = 13;
    int yrange = maxY - minY;

    return Vector2(rand()%cellXCount, (rand()%yrange) + minY);
  }
public:
  Vector2 pos;
  Food() : pos(rand()%cellXCount, rand()%cellYCount){}

  void display(TFT_eSPI &tft){
    tft.fillRect(pos.x*cellSize, pos.y*cellSize, cellSize, cellSize, PURPLE);
  }

  Vector2 GenerateRandomPos(std::deque<Vector2> &snake){  // so we dont generate a fruit in the snake
    Vector2 pos = GenerateRandomCell();
    while(inDeque(pos, snake)){
      pos = GenerateRandomCell();
    }
    return pos;
  }

  ~Food() {}
};

class Snake{
public:
  std::deque<Vector2> body = {Vector2(8, 4), Vector2(7, 4), Vector2(6, 4)};
  Vector2 direction = Vector2(1, 0);
  bool grow = false;

  void display(TFT_eSPI &tft){
    for (const Vector2 &v : body){
      tft.fillRect(v.x*cellSize, v.y*cellSize, cellSize, cellSize, GREEN);
    }
  }

  void Update(){
    Vector2 tail = body.back();
    body.push_front(Vector2(body.front().x + direction.x, body.front().y + direction.y));
    if (grow){
      grow = false;
    }else{
      body.pop_back();
      tft.fillRect(tail.x*cellSize, tail.y*cellSize, cellSize, cellSize, TFT_BLACK);
    }
  }

  void Reset(){
    body = {Vector2(8, 4), Vector2(7, 4), Vector2(6, 4)};
    direction = Vector2(1, 0);
  }
};

class Game{
public:
  Snake* snake = new Snake();
  Food* fruit = new Food();
  bool run = true;
  int score = 0;

  void display(TFT_eSPI &tft){
    snake->display(tft);
    fruit->display(tft);
  }

  void Update(){
    if(run){
      display(tft);
      snake->Update();
      eatFood();
      checkEdge();
      checkColl();
    }
  }

  void eatFood(){
    if (snake->body.front().x == fruit->pos.x && snake->body.front().y == fruit->pos.y){
      snake->grow = true;
      score ++;
      fruit->pos = fruit->GenerateRandomPos(snake->body);
    }
  }

  void checkEdge(){
    if (snake->body.front().x == cellXCount || snake->body.front().x == -1 || snake->body.front().y == cellYCount || snake->body.front().y == 1){
      GameOver();
    }
  }

  void checkColl(){
    std::deque<Vector2> withouthead = snake->body;
    withouthead.pop_front();
    if (inDeque(snake->body.front(), withouthead)){
      GameOver();
    }
  }

  void GameOver(){
    snake->Reset();
    fruit->pos = fruit->GenerateRandomPos(snake->body);
    run = false;
    state = OVER;
  }

  void resetScore(){
    score = 0;
  }
  ~Game(){
    delete snake;
    delete fruit;
  }
};


bool eventTrig(double time){
  double currTime = millis();
  if (currTime - lastUpdateTime >= time){
    lastUpdateTime = currTime;
    return true;
  }
  return false;
}
Directions buffered = RIGHT;

void GameLoop(Game *gameSess){
  Directions dir = readInput();
  
  if (dir != NONE) buffered = dir;
  if (eventTrig(250)){
    switch (buffered){
    case UP:
      if (gameSess->snake->direction.y != 1){
      gameSess->snake->direction = Vector2{0, -1};}
      break;

    case DOWN:
      if (gameSess->snake->direction.y != -1){
      gameSess->snake->direction = Vector2{0, 1};}
      break;

    case LEFT:
      if (gameSess->snake->direction.x != 1){
      gameSess->snake->direction = Vector2{-1, 0};}
      break;
    
    case RIGHT:
      if (gameSess->snake->direction.x != -1){
      gameSess->snake->direction = Vector2{1, 0};}
      break;    
    }
    gameSess->Update();
  }
}

void drawScreen_1(void) {
  tft.fillScreen(0x0);
  tft.setTextColor(0x07E6);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeSerifItalic18pt7b);
  tft.drawString("Snake", 80, 32);
  tft.setTextSize(2);
  tft.setFreeFont(&Petme8x8);
  tft.setTextColor(0x435B);
  tft.drawString("Click to Begin", 8, 88);
  tft.drawBitmap(59, 38, image_ButtonLeftSmall_bits, 12, 20, 0xFC00);
  tft.drawBitmap(169, 38, image_ButtonRightSmall_bits, 12, 20, 0xFC00);
}
// [END lopaka generated]

void drawScreen_2(Game *games) {
    tft.fillScreen(0x0);
    tft.setTextColor(0xA800);
    tft.setTextSize(3);
    tft.setFreeFont(&Petme8x8);
    tft.drawString("GAMEOVER!", 12, 43);
    tft.setTextSize(1);
    tft.setFreeFont(NULL);
    tft.setTextColor(0xF7DF);
    tft.drawString("Press to Retry", 79, 100);
    char scoreSTR[16];
    snprintf(scoreSTR, sizeof(scoreSTR), "Score: %d", games->score);
    tft.drawString(scoreSTR, 79, 83);
}
// [END lopaka generated]


void setup(){
  Serial.begin(115200);

  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);

  JOYXCENTER = analogRead(JOYSTICK_X);
  JOYYCENTER = analogRead(JOYSTICK_Y);

  tft.init();
  tft.setRotation(1);

  drawScreen_1();
}

Game *games = new Game();
bool overScreen = false;

void loop(){
  switch (state){
    case START:
      if (buttonPressed()){
        tft.fillScreen(0x0000);
        lastUpdateTime = millis();
        state = PLAYING;
        buffered = NONE;
      }
      break;
    
    case PLAYING:
    
      tft.setFreeFont(&Petme8x8);
      tft.setTextSize(1);
      tft.setTextColor(0xFFE0, 0x0000);
      tft.drawRect(1, 20, 239, 115, GREEN);
      Serial.printf("X: %d | Y: %d\n", analogRead(JOYSTICK_X), analogRead(JOYSTICK_Y));

      char scoreSTR[16];
      snprintf(scoreSTR, sizeof(scoreSTR), "Score: %d", games->score);
      tft.drawString(scoreSTR, 0, 9);
      GameLoop(games);
      overScreen = false;
      break;

    case OVER:
      if (!overScreen){
        drawScreen_2(games);
        overScreen = true;
      }
      if (buttonPressed()){
        state = PLAYING;
        tft.fillScreen(0x0000);

        lastUpdateTime = millis();
        games->resetScore();
        buffered = NONE;
        games->run = true;
      }
      break;
  }
}