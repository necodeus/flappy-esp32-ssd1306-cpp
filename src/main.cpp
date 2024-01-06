#include <Wire.h>
#include "SSD1306Wire.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

SSD1306Wire display(0x3C, SDA, SCL);

int points = 0;
int velocity = 2;

class Bird
{
public:
  int x, y, width, height, velocity;
  Bird()
  {
    x = 50;
    y = 20;
    width = 5;
    height = 5;
    velocity = 0;
  }
  void update()
  {
    y += velocity;
    velocity += 1;
    if (y < 0)
    {
      y = 0;
    }
    else if (y + height > SCREEN_HEIGHT)
    {
      y = SCREEN_HEIGHT - height;
    }
  }
  void draw(SSD1306Wire *display)
  {
    display->setColor(WHITE);
    display->fillRect(x, y, width, height);
  }
};

class Pipe
{
public:
  int x, width, gap, topHeight, bottomHeight;
  bool passed;
  Pipe(int xPos)
  {
    x = xPos;
    width = 20;
    gap = 40;
    topHeight = random(5, 20);
    bottomHeight = SCREEN_HEIGHT - gap - topHeight;
    passed = false;
  }
  void update()
  {
    x -= velocity;

    if (x + width < 0) {
      x = SCREEN_WIDTH;
      topHeight = random(5, 20);
      bottomHeight = SCREEN_HEIGHT - gap - topHeight;
      passed = false;
    }
  }
  void draw(SSD1306Wire *display)
  {
    display->setColor(WHITE);
    display->fillRect(x, 0, width, topHeight);
    display->fillRect(x, SCREEN_HEIGHT - bottomHeight, width, bottomHeight);
  }
};

Bird bird;
Pipe pipes[] = { Pipe(SCREEN_WIDTH), Pipe(SCREEN_WIDTH), Pipe(SCREEN_WIDTH), Pipe(SCREEN_WIDTH) };
const int pipeDistance = 40;

bool checkCollision(Bird bird, Pipe pipe)
{
  if (bird.y < 0 || bird.y + bird.height > SCREEN_HEIGHT)
  {
    return true;
  }
  if (bird.x + bird.width >= pipe.x && bird.x <= pipe.x + pipe.width)
  {
    if (bird.y <= pipe.topHeight || bird.y + bird.height >= SCREEN_HEIGHT - pipe.bottomHeight)
    {
      return true;
    }
  }
  return false;
}

bool checkIfBirdPassed(Bird bird, Pipe pipe)
{
  if (bird.x > pipe.x + pipe.width && !pipe.passed)
  {
    pipe.passed = true;
    return true;
  }
  return false;
}

void setup()
{
  display.init();
}


int lockY = 0;

bool isJoystickUp(int gpio)
{
  int y = analogRead(gpio);

  return y > 3000 && !lockY;
}

bool isJoystickDown(int gpio)
{
  int y = analogRead(gpio);
  return y < 1000 && !lockY;
}

bool isJoystickIdle(int gpio)
{
  int y = analogRead(gpio);
  return y < 3000 && y > 1000;
}

void lockJoystick()
{
  lockY = 1;
}

void unlockJoystick()
{
  lockY = 0;
}

void loop()
{
  display.clear();

  if (isJoystickIdle(34)) {
    unlockJoystick();
  }

  if (isJoystickUp(34) || isJoystickDown(34)) {
    lockJoystick();
    bird.velocity = -5;
  }

  bird.update();
  bird.draw(&display);

  for (int i = 0; i < sizeof(pipes) / sizeof(pipes[0]); i++)
  {
    pipes[i].update();
    pipes[i].draw(&display);

    if (checkCollision(bird, pipes[i]))
    {
      display.clear();
      display.setColor(WHITE);
      display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      display.setFont(ArialMT_Plain_16);
      display.drawString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "Game Over");
      display.display();
      delay(500);
      ESP.restart();
    }
    if (checkIfBirdPassed(bird, pipes[i]))
    {
      points++;
    }
  }

  display.drawString(0, 0, String(points));

  display.display();

  delay(50);
}
