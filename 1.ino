#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/Picopixel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 32, 2, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

/*_______แกน x___________แกน y______*/
byte xy[7][2][4] = {
  { { 3, 3, 4, 3 }, { 2, 3, 3, 4 } },
  { { 3, 4, 3, 4 }, { 2, 2, 3, 3 } },
  { { 4, 4, 3, 3 }, { 4, 3, 3, 2 } },
  { { 3, 3, 4, 4 }, { 4, 3, 3, 2 } },
  { { 3, 3, 3, 4 }, { 2, 3, 4, 4 } },
  { { 4, 4, 4, 3 }, { 2, 3, 4, 4 } },
  { { 2, 3, 4, 5 }, { 3, 3, 3, 3 } }
};
int32_t time1 = 750;
int8_t x, y, y1;
int8_t Block, Block1;
int color, data, dataCheck, colorrandom, score;
int colorchoie[] = { 0xf800, 0x07e0, 0x001f, 0xffe0, 0xf81f, 0x07ff };
bool light, shadow;

bool State() {
  for (Block = 0; Block < 7; Block++) {
    display();
    no_display();
    delay(750);
    if (Serial.available() && Serial.read() == 13) return 1;
  }
  return 0;
}

void Start() {
  display();
  for (int i = 3; i > 0; i--) {
    matrix.setTextColor(matrix.Color(255, 0, 0));
    matrix.setCursor(i == 1 ? i + 1 : 1, 23);
    matrix.print(String(i));
    matrix.show();
    matrix.setTextColor(0);
    matrix.setCursor(i == 1 ? i + 1 : 1, 23);
    matrix.print(String(i));
    delay(1000);
  }
  no_display();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Score = 0"));
  for (int i = 0; i < 10; i++)
    Serial.read();
  randomSeed(micros());
  Block1 = Block = random(0, 6);
  display();
  y = 5, Block = random(0, 6), colorrandom = random(1, 6);
  digitalWrite(4, 1);
}

void display() {
  for (int i = 0; i < 4; i++)
    matrix.drawPixel(xy[Block][0][i] + x, xy[Block][1][i] + y, light == 0 ? matrix.Color(255, 0, 0) : colorchoie[colorrandom]);
  matrix.show();
}

void no_display() {
  for (int i = 0; i < 4; i++)
    matrix.drawPixel(xy[Block][0][i] + x, xy[Block][1][i] + y, matrix.Color(0, 0, 0));
  for (int j = 0; j < 4; j++)
    matrix.drawPixel(xy[Block][0][j] + x, xy[Block][1][j] + y1, matrix.Color(0, 0, 0));
}

void Check_Color() {
  data = Serial.read();
  while (1) {
    Check_Edge();
    if (data == 13 || data == 's') time1 = millis() + 750, y++;
    for (int i = 0; i < 4; i++)
      if ((xy[Block][1][i] + y >= 32) || matrix.getPixelColor((7 - (xy[Block][0][i] + x)) * 32 + (xy[Block][1][i] + y)) > 0) {
        if ((data == 'a' || data == 'd') && (xy[Block][1][i] + y < 32) && (data == 'a' ? x++ : x--)) break;
        else y--;
        display();
        while (dataCheck != 0)
          Check_rotate();
        random_Block();
        y = 6, data = 0, x = 0;
        GameOver();
        break;
      }
    if (data != 13) {
      Check_Score();
      Check_Shadow();
      display();
      no_display();
      break;
    }
  }
}

void GameOver() {
  for (int i = 0; i < 4; i++)
    while ((xy[Block][1][i] + y == 10) && matrix.getPixelColor((7 - (xy[Block][0][i] + x)) * 32 + (xy[Block][1][i] + y)) > 0) {
      lcd.setCursor(0, 1);
      lcd.print(F("--Game Over!!!--"));
      if (millis() >= time1) time1 = millis() + 500, digitalWrite(4, 0), digitalWrite(3, !digitalRead(3));
      if (Serial.available() && Serial.read() == 32) digitalWrite(13, 1);
    }
}

void Check_Score() {
  for (int i = 233; i < 256; i++) {
    for (int j = i; j >= i - 224; j -= 32)
      if (matrix.getPixelColor(j) > 0)
        if (++color == 8) {
          lcd.setCursor(8, 0);
          lcd.print(score += 8);
          for (int k = i; k >= 233; k--)
            for (int q = k; q >= k - 224; q -= 32)
              matrix.setPixelColor(q, matrix.getPixelColor(q - 1));
          break;
        }
    color = 0;
  }
}

void Check_Edge() {
  while (data == 32) {
    if (Serial.available() && Serial.read() == 32) {
      digitalWrite(3, 0);
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      time1 = millis() + 100;
      break;
    }
    if (millis() >= time1) time1 = millis() + 500, digitalWrite(3, !digitalRead(3));
    lcd.setCursor(0, 1);
    lcd.print(F("___Stop Game!___"));
  }
  if (data == 'w' && Block != 1) {
    Check_rotate();
    Check_rotate_Edge();
  } else if (data == 'l') light = !light;
  else if (data == 'k') shadow = !shadow;
  else if (data == 'a') {
    for (int i = 0; i < 4; i++)
      if (matrix.getPixelColor((7 - (xy[Block][0][i] + (x - 1))) * 32 + (xy[Block][1][i] + y)) > 0 || xy[Block][0][i] + x == 0) return;
    x--;
  } else if (data == 'd') {
    for (int i = 0; i < 4; i++)
      if (matrix.getPixelColor((7 - (xy[Block][0][i] + (x + 1))) * 32 + (xy[Block][1][i] + y)) > 0 || xy[Block][0][i] + x == 7) return;
    x++;
  }
}

void Check_rotate_Edge() {
  for (int i = 0; i < 4; i++)
    if ((matrix.getPixelColor((7 - (xy[Block][0][i] + x)) * 32 + (xy[Block][1][i] + y)) > 0 && data != 13))
      for (int i = 0; i < 3; i++)
        Check_rotate();
    else if (matrix.getPixelColor((7 - (xy[Block][0][i] + (x + 1))) * 32 + (xy[Block][1][i] + y)) > 0 || matrix.getPixelColor((7 - (xy[Block][0][i] + (x - 1))) * 32 + (xy[Block][1][i] + y)) > 0)
      for (int i = 0; i < 4; i++)
        if (xy[Block][0][i] + x < 0 || xy[Block][0][i] + x > 7)
          for (int i = 0; i < 3; i++)
            Check_rotate();
  for (int i = 0; i < 4; i++)
    if (xy[Block][0][i] + x < 0) x++;
    else if (xy[Block][0][i] + x > 7) x--;
}

void Check_rotate() {
  if (++dataCheck == 4) dataCheck = 0;
  for (int i = 0; i < 4; i++)
    if (i == 1) continue;
    else if (Block == 6 && i == 3)
      if (xy[Block][0][i] > xy[Block][0][1]) xy[Block][1][i] += 2, xy[Block][0][i] -= 2;
      else if (xy[Block][0][i] < xy[Block][0][1]) xy[Block][0][i] += 2, xy[Block][1][i] -= 2;
      else if (xy[Block][1][i] < xy[Block][1][1]) xy[Block][0][i] += 2, xy[Block][1][i] += 2;
      else xy[Block][0][i] -= 2, xy[Block][1][i] -= 2;
    else if (xy[Block][1][i] < xy[Block][1][1] && xy[Block][0][i] != xy[Block][0][1])
      if (xy[Block][0][i] > xy[Block][0][1]) xy[Block][1][i] += 2;
      else xy[Block][0][i] += 2;
    else if (xy[Block][1][i] > xy[Block][1][1] && xy[Block][0][i] != xy[Block][0][1])
      if (xy[Block][0][i] > xy[Block][0][1]) xy[Block][0][i] -= 2;
      else xy[Block][1][i] -= 2;
    else if (xy[Block][0][i] < xy[Block][0][1] || xy[Block][1][i] < xy[Block][1][1])
      if (xy[Block][0][i] < xy[Block][0][1]) xy[Block][0][i] += 1, xy[Block][1][i] -= 1;
      else xy[Block][0][i] += 1, xy[Block][1][i] += 1;
    else if (xy[Block][0][i] > xy[Block][0][1] || xy[Block][1][i] > xy[Block][1][1])
      if (xy[Block][0][i] > xy[Block][0][1]) xy[Block][0][i] -= 1, xy[Block][1][i] += 1;
      else xy[Block][0][i] -= 1, xy[Block][1][i] -= 1;
}

void random_Block() {
  randomSeed(micros());
  for (int i = 0; i < 4; i++)
    matrix.drawPixel(xy[Block1][0][i], xy[Block1][1][i], matrix.Color(0, 0, 0));
  Block = Block1, Block1 = random(0, 7), colorrandom = random(0, 6);
  for (int i = 0; i < 4; i++)
    matrix.drawPixel(xy[Block1][0][i], xy[Block1][1][i], matrix.Color(255, 0, 0));
}

void Check_Shadow() {
  for (y1 = 5; y1 < 32; y1++)
    for (int j = 0; j < 4; j++)
      if (((xy[Block][1][j] + y1 >= 32) || matrix.getPixelColor((7 - (xy[Block][0][j] + x)) * 32 + (xy[Block][1][j] + y1)) > 0) && y1 > y) {
        y1--;
        for (int k = 0; k < 4; k++)
          matrix.drawPixel(xy[Block][0][k] + x, xy[Block][1][k] + y1, matrix.Color(shadow * 255, shadow * 255, shadow * 255));
        return;
      }
}

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.setBrightness(255);
  matrix.setFont(&Picopixel);
  matrix.setTextSize(2);
  lcd.init();
  lcd.setCursor(2, 0);
  lcd.print(F("Tetris Game!"));
  lcd.setCursor(0, 1);
  lcd.print(F("----------------"));
  Serial.println(F("______________All the rules of the game.______________\r"));
  Serial.println(F("w = Rotate block. \ra = Swipe left. \rs = Scroll down. \rd = Swipe right. \r"));
  Serial.println(F("k = Toggle block placement assist. \rl = Toggle color blocks. \rSpacar = Stop the game. \rEnter = OK, place the block. \r"));
  Serial.println(F("Click the terminal and press Enter to start the game."));
  for (int i = 0; i < 20; i++)
    Serial.println("");
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  for (int i = 0; i < 10; i++)
    Serial.read();
  for (int i = 0; i < 8; i++)
    matrix.drawPixel(i, 7, matrix.Color(255, 0, 0));
}

void loop() {
  if (State() == 1) {
    Start();
    while (true) {
      if (millis() > time1) {
        y++;
        Check_Color();
        time1 = millis() + 750;
      }
      if (Serial.available())
        Check_Color();
    }
  }
}