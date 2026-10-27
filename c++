#include <avr/pgmspace.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_RESET = -1;
const int SCREEN_ADDRESS = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int BUTTON_PIN = 2;


const unsigned char robot_face_normal[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x3F,0xFC,0x00,0x00,0x7F,0xFE,0x00,0x00,0xFF,0xFF,0x00,
  0x01,0xFF,0xFF,0x80,0x03,0xFF,0xFF,0xC0,0x07,0xFF,0xFF,0xE0,0x0F,0xC0,0x03,0xF0,
  0x1F,0x80,0x01,0xF8,0x1F,0x00,0x00,0xF8,0x3E,0x00,0x00,0x7C,0x3C,0x00,0x00,0x3C,
  0x78,0x00,0x00,0x1E,0x78,0x00,0x00,0x1E,0x70,0x7E,0x1F,0x0E,0x70,0x7E,0x1F,0x0E,
  0x70,0x00,0x00,0x0E,0x70,0x00,0x00,0x0E,0x78,0x00,0x00,0x1E,0x78,0x00,0x00,0x1E,
  0x3C,0x00,0x00,0x3C,0x3E,0x00,0x00,0x7C,0x1F,0x80,0x01,0xF8,0x0F,0xC0,0x03,0xF0,
  0x07,0xFF,0xFF,0xE0,0x03,0xFF,0xFF,0xC0,0x01,0xFF,0xFF,0x80,0x00,0x7F,0xFE,0x00
};

const unsigned char robot_face_blink[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x3F,0xFC,0x00,0x00,0x7F,0xFE,0x00,0x00,0xFF,0xFF,0x00,
  0x01,0xFF,0xFF,0x80,0x03,0xFF,0xFF,0xC0,0x07,0xFF,0xFF,0xE0,0x0F,0xC0,0x03,0xF0,
  0x1F,0x80,0x01,0xF8,0x1F,0x00,0x00,0xF8,0x3E,0x00,0x00,0x7C,0x3C,0x00,0x00,0x3C,
  0x78,0x00,0x00,0x1E,0x78,0x00,0x00,0x1E,0x70,0x00,0x00,0x0E,0x70,0x00,0x00,0x0E,
  0x70,0x00,0x00,0x0E,0x70,0x00,0x00,0x0E,0x78,0x00,0x00,0x1E,0x78,0x00,0x00,0x1E,
  0x3C,0x00,0x00,0x3C,0x3E,0x00,0x00,0x7C,0x1F,0x80,0x01,0xF8,0x0F,0xC0,0x03,0xF0,
  0x07,0xFF,0xFF,0xE0,0x03,0xFF,0xFF,0xC0,0x01,0xFF,0xFF,0x80,0x00,0x7F,0xFE,0x00
};


const int BIRD_WIDTH_PX = 10;
const int BIRD_HEIGHT_PX = 8;

const unsigned char epd_bitmap_bird1[] PROGMEM = {
  0x0e, 0x00, 0x15, 0x00, 0x60, 0x80, 0x81, 0x80, 0xfc, 0xc0, 0xfb, 0x80, 0x71, 0x00, 0x1e, 0x00
};
const unsigned char epd_bitmap_bird2[] PROGMEM = {
  0x0e, 0x00, 0x15, 0x00, 0x70, 0x80, 0xf9, 0x80, 0xfc, 0xc0, 0x83, 0x80, 0x71, 0x00, 0x1e, 0x00
};

const unsigned char* epd_bitmap_allArray[2] = {
  epd_bitmap_bird1,
  epd_bitmap_bird2
};


const int SCALE_FACTOR = 10;
const int GRAVITY = 5;
const int JUMP_STRENGTH = -25;
const int PIPE_WIDTH = 15 * SCALE_FACTOR;
const int PIPE_GAP = 30 * SCALE_FACTOR;
const int PIPE_SPEED = 10;
const int PIPE_SPACING = 65 * SCALE_FACTOR;

// Variabel Game
int birdY;
float velocity = 0;
bool gameOver = false;
int score = 0;
unsigned long frameCounter = 0;
unsigned long gameOverTime = 0;

enum GameState { ROBOT_FACE, STARTSCREEN, PLAYING, HIGHSCORE };
GameState gameState = ROBOT_FACE;

struct Pipe {
  int x;
  int height;
};
const int MAX_PIPES = 3;
Pipe pipes[MAX_PIPES];

long highscores[5] = {};
int newHighscoreIndex = -1;
unsigned long lastBlinkTime = 0;
bool showBlink = true;

const int FRAME_TIME = 40;
unsigned long lastFrameTime = 0;
unsigned long lastRobotBlink = 0;
bool robotEyeBlink = false;


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  randomSeed(analogRead(0));

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    while (1) delay(100);
  }
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime < FRAME_TIME) return;
  lastFrameTime = currentTime;
  frameCounter++;

  switch (gameState) {
    // Tampilan Awal: Wajah Robot
    case ROBOT_FACE:
      display.clearDisplay();
      
      // Animasi kedip mata
      if (millis() - lastRobotBlink > 2000) {
        robotEyeBlink = !robotEyeBlink;
        lastRobotBlink = millis();
      }

      if (robotEyeBlink) {
        display.drawBitmap(48, 16, robot_face_blink, 32, 32, SSD1306_WHITE);
      } else {
        display.drawBitmap(48, 16, robot_face_normal, 32, 32, SSD1306_WHITE);
      }

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(22, 52);
      display.println("mogasuka hasna :3");
      display.setCursor(30, 60);
      display.println("");
      display.display();

      // Masuk ke game jika tombol ditekan & ditahan
      if (digitalRead(BUTTON_PIN) == LOW) {
        delay(300);
        if (digitalRead(BUTTON_PIN) == LOW) {
          gameState = STARTSCREEN;
        }
      }
      break;

    // Layar Mulai Game
    case STARTSCREEN:
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(15, 20);
      display.println("hi");
      display.setTextSize(1);
      display.setCursor(25, 45);
      display.println("Tekan untuk mulai");
      display.display();

      if (digitalRead(BUTTON_PIN) == LOW) {
        resetGame();
        gameState = PLAYING;
      }
      break;

    // Permainan Berjalan
    case PLAYING:
      if (gameOver) {
        if (millis() - gameOverTime > 3000) {
          gameState = HIGHSCORE;
        }
        if ((millis() - gameOverTime > 1000) && digitalRead(BUTTON_PIN) == LOW) {
          resetGame();
          gameState = PLAYING;
        }
      } else {
        if (digitalRead(BUTTON_PIN) == LOW) {
          velocity = JUMP_STRENGTH;
        }
        velocity += GRAVITY;
        velocity = constrain(velocity, -50, 50);
        birdY += velocity;
        if (birdY < 0) birdY = 0;

        movePipes();
        checkCollision();
      }
      drawGame();
      break;

    // Layar Skor Akhir
    case HIGHSCORE:
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(30, 2);
      display.println("skor yg tinggi");

      if (millis() - lastBlinkTime > 100) {
        showBlink = !showBlink;
        lastBlinkTime = millis();
      }

      for (int i = 0; i < 5; i++) {
        if (i == newHighscoreIndex && !showBlink) continue;
        display.setCursor(25, 15 + (i * 9));
        display.print(i + 1);
        display.print(". ");
        display.print(highscores[i]);
      }
      display.display();

      if (millis() - gameOverTime > 8000) {
        gameState = ROBOT_FACE;
      }
      if (digitalRead(BUTTON_PIN) == LOW) {
        resetGame();
        gameState = PLAYING;
      }
      break;
  }
}


void resetGame() {
  birdY = SCREEN_HEIGHT * SCALE_FACTOR / 5;
  velocity = 0;
  score = 0;
  gameOver = false;

  pipes[0].x = SCREEN_WIDTH * SCALE_FACTOR;
  pipes[0].height = random(10 * SCALE_FACTOR, SCREEN_HEIGHT * SCALE_FACTOR - PIPE_GAP - 10 * SCALE_FACTOR);
  for (int i = 1; i < MAX_PIPES; i++) {
    pipes[i].x = SCREEN_WIDTH * SCALE_FACTOR + i * PIPE_SPACING + random(-20, 20) * SCALE_FACTOR;
    pipes[i].height = random(10 * SCALE_FACTOR, SCREEN_HEIGHT * SCALE_FACTOR - PIPE_GAP - 10 * SCALE_FACTOR);
  }
}

void movePipes() {
  int speed = PIPE_SPEED + score;
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x -= speed;
    if (pipes[i].x < -PIPE_WIDTH) {
      int maxX = 0;
      for (int j = 0; j < MAX_PIPES; j++) if (pipes[j].x > maxX) maxX = pipes[j].x;
      pipes[i].x = maxX + PIPE_SPACING + random(-20, 20) * SCALE_FACTOR;
      pipes[i].height = random(10 * SCALE_FACTOR, SCREEN_HEIGHT * SCALE_FACTOR - PIPE_GAP - 10 * SCALE_FACTOR);
      score++;
    }
  }
}

void checkCollision() {
  for (int i = 0; i < MAX_PIPES; i++) {
    if (pipes[i].x < 17 * SCALE_FACTOR && pipes[i].x + PIPE_WIDTH > 13 * SCALE_FACTOR) {
      if ((birdY < pipes[i].height) || (birdY > pipes[i].height + PIPE_GAP)) {
        gameOver = true;
        gameOverTime = millis();
        for (int j = 4; j > 0; j--) highscores[j] = highscores[j-1];
        highscores[0] = score;
      }
    }
  }
  if (birdY >= SCREEN_HEIGHT * SCALE_FACTOR) {
    gameOver = true;
    gameOverTime = millis();
    for (int j = 4; j > 0; j--) highscores[j] = highscores[j-1];
    highscores[0] = score;
  }
}

void drawGame() {
  display.clearDisplay();
  int frame = (frameCounter / 5) % 2;
  display.drawBitmap(10, (birdY / SCALE_FACTOR) - 4, epd_bitmap_allArray[frame], 10, 8, SSD1306_WHITE);

  for (int i = 0; i < MAX_PIPES; i++) {
    int x = pipes[i].x / SCALE_FACTOR;
    int h = pipes[i].height / SCALE_FACTOR;
    display.fillRect(x, 0, 15, h - 6, SSD1306_WHITE);
    display.fillRect(x - 2, h - 6, 19, 6, SSD1306_WHITE);
    display.fillRect(x - 2, h + 30, 19, 6, SSD1306_WHITE);
    display.fillRect(x, h + 36, 15, SCREEN_HEIGHT - (h + 36), SSD1306_WHITE);
  }

  display.setTextSize(1);
  display.setCursor(110, 2);
  display.print(score);
  if (gameOver) {
    display.fillRect(25, 15, 78, 34, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(42, 25);
    display.println("yah kalah");
    display.setCursor(38, 40);
    display.print("Skor: ");
    display.print(score);
  }
  display.display();
}
