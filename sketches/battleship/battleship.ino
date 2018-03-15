#include <battleshiptable.h>
#include <Wire.h>
#include "LedControlMS.h"

// used to control leds
LedControl lc=LedControl(12,11,10, 1);

// pins for buttons
const byte xButt = 7;
const byte yButt = 8;
const byte entButt = 9;

// if this arduino is waiting for the other player
boolean waiting = false;
// display guesses if true, display ships if false
boolean displayToggle = false;
// alternates between true and false
boolean blinker = true;
byte blinkerCounter = 0;
// alternates between true and false at half the rate
boolean slowBlinker = true;
byte slowBlinkerCounter = 0;

// button values when button is held
boolean entHeld = false;
boolean xHeld = false;
boolean yHeld = false;
// button values when button is initially pressed
boolean entPressed = false;
boolean xPressed = false;
boolean yPressed = false;

// location of guess cursor
byte cursorX = 3;
byte cursorY = 3;

// all the guesses locations 0=not guessed, 1=guessed, 2=hit
byte guesses[8][8];
// the last guessed location
byte lastGuessX;
byte lastGuessY;
byte response = 0;  // 0=no response, 1=miss, 2=hit

void setup() {

  pinMode(xButt, INPUT);
  pinMode(yButt, INPUT);
  pinMode(entButt, INPUT);

  Wire.begin(8);
  Wire.onReceive(receiveTurnEvent);
  
  Serial.begin (9600);
  Serial.println("Setup");
  
    lc.shutdown(0,false);
  // Set the brightness to a medium values
  lc.setIntensity(0,8);
  // and clear the display
  lc.clearDisplay(0);
  
  placeShips();

  Serial.println("Ship locations");
  for (byte i=0; i<12; i++) {
    Serial.print(readShipX(i));
    Serial.print(" ");
    Serial.println(readShipY(i));
  }

  startingAnimation();
}


void loop() {

  boolean pixels[8][8] = {{0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0}};

  boolean a = 1;

  manageButtons();

  if (response != 0) {
    Wire.beginTransmission(8);
    Wire.write(response);
    Wire.endTransmission();
    
    response = 0;
  }

  // lost detection
  boolean lost = true;
  for (byte i = 0; i<12; i++)
    if (readShipHit(i) == 0)
      lost = false;
  if (lost) {
    boolean cross[8][8] = {{1,0,0,0,0,0,0,1},
                           {0,1,0,0,0,0,1,0},
                           {0,0,1,0,0,1,0,0},
                           {0,0,0,1,1,0,0,0},
                           {0,0,0,1,1,0,0,0},
                           {0,0,1,0,0,1,0,0},
                           {0,1,0,0,0,0,1,0},
                           {1,0,0,0,0,0,0,1}};
    updateDisplay(cross);
    delay(1000);
    return;
  }
  
  // win detection
  byte hitCnt = 0;
  for (byte row=0; row<8; row++)
    for (byte col=0; col<8; col++)
      if (guesses[row][col] == 2)
        hitCnt++;
  if (hitCnt >= 12) {
    boolean circle[8][8] = {{0,0,1,1,1,1,0,0},
                            {0,1,0,0,0,0,1,0},
                            {1,0,0,0,0,0,0,1},
                            {1,0,0,0,0,0,0,1},
                            {1,0,0,0,0,0,0,1},
                            {1,0,0,0,0,0,0,1},
                            {0,1,0,0,0,0,1,0},
                            {0,0,1,1,1,1,0,0}};
    updateDisplay(circle);
    delay(1000);
    return;
  }

  // my turn
  if (!waiting) {
    displayGuesses(pixels);
    pixels[cursorY][cursorX] = slowBlinker;

    if (xPressed)
      cursorX = ++cursorX%8;
    if (yPressed)
      cursorY = ++cursorY%8;
    if (entPressed) {
      guesses[cursorY][cursorX] = 1;
      lastGuessX = cursorX;
      lastGuessY = cursorY;

      Wire.beginTransmission(8);
      Wire.write(cursorX);
      Wire.write(cursorY);
      Wire.endTransmission();

      displayToggle = false;
      waiting = true;
    }
  }
  // others turn
  else {
    // ent is toggle displays
    if (entPressed)
      displayToggle = !displayToggle;
      
    if (!displayToggle) {
      displayShips(pixels);
    }
    else {
      displayGuesses(pixels);
    }
  }

  // blinkers
  blinkerCounter++;
  if (blinkerCounter >= 2) {
    blinkerCounter = 0;
    blinker = !blinker;
  }
  slowBlinkerCounter++;
  if (slowBlinkerCounter >= 4) {
    slowBlinkerCounter = 0;
    slowBlinker = !slowBlinker;
  }

  updateDisplay(pixels);
  delay(50);
}

void placeShips() {
  randomSeed(analogRead(0));
  byte shipCnt = 0;
  
  for (byte i=0; i<4; i++) {

    // 1st ship is 4 long, 3rd and 2nd is 3, 4th is 2
    byte shipSize;
    if (i==0)
      shipSize = 4;
    else if(i==1 || i==2)
      shipSize = 3;
    else
      shipSize = 2;

    // place the ship in a random location
    boolean placed = false;
    while (!placed) {
      byte x = random(8);
      byte y = random(8);
      byte dir = random(4);

      if (canPlace(x,y,dir,shipSize)) {
        // place ship
        for (byte j=0; j<shipSize; j++) {
          // up
          if (dir==0)
            writeShip(shipCnt++, x, y+j, false);
          // right
          if (dir==1)
            writeShip(shipCnt++, x+j, y, false);
          // down
          if (dir==2)
            writeShip(shipCnt++, x, y-j, false);
          // left
          if (dir==3)
            writeShip(shipCnt++, x-j, y, false);
        }
        
        placed = true;
      }
    }
  }
}

boolean canPlace(byte x, byte y, byte dir, byte shipSize) {
  // up
  if (dir==0) {
    // check if falls off display
    if (y+shipSize > 7)
      return false;

    // check if collides with another ship
    for (byte i=0; i<shipSize; i++)
      if (isShip(x, y+i))
        return false;
  }

  // right
  else if (dir==1) {
    if (x+shipSize > 7)
      return false;

    for (byte i=0; i<shipSize; i++)
      if (isShip(x+i, y))
        return false;
  }

  // down
  else if (dir==2) {
    if (y-shipSize < 0)
      return false;

    for (byte i=0; i<shipSize; i++)
      if (isShip(x, y-i))
        return false;
  }

  // left
  else {
    if (x-shipSize < 0)
      return false;

    for (byte i=0; i<shipSize; i++)
      if (isShip(x-i, y))
        return false;
  }

  return true;
}

boolean isShip(byte x, byte y) {
  for (byte i=0; i<12; i++)
    if (readShipX(i)==x && readShipY(i)==y)
      return true;
  return false;
}

void displayShips(boolean pixels[8][8]) {
  for (byte i=0; i<12; i++) {
    if (readShipHit(i))
      pixels[readShipY(i)][readShipX(i)] = blinker;
    else
      pixels[readShipY(i)][readShipX(i)] = true;
  }
}

void startingAnimation() {
  
  for (int x=3; x>=0; x--) {
    boolean pixels[8][8] = {{0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0},
                            {0,0,0,0,0,0,0,0}};
    
    for (byte y=0; y<8; y++)
      pixels[y][x] = true;
    for (byte y=0; y<8; y++)
      pixels[y][7-x] = true;

    updateDisplay(pixels);
    delay(100);
  }
}

void manageButtons() {
  if (!entHeld && digitalRead(entButt)) {
    entHeld = true;
    entPressed = true;
  } else if (digitalRead(entButt)) {
    entHeld = true;
    entPressed = false;
  } else {
    entHeld = false;
    entPressed = false;
  }

  if (!xHeld && digitalRead(xButt)) {
    xHeld = true;
    xPressed = true;
  } else if (digitalRead(xButt)) {
    xHeld = true;
    xPressed = false;
  } else {
    xHeld = false;
    xPressed = false;
  }
  
  if (!yHeld && digitalRead(yButt)) {
    yHeld = true;
    yPressed = true;
  } else if (digitalRead(yButt)) {
    yHeld = true;
    yPressed = false;
  } else {
    yHeld = false;
    yPressed = false;
  }
}

void displayGuesses(boolean pixels[8][8]) {
  for (byte row=0; row<8; row++) {
    for (byte col=0; col<8; col++) {
      if (guesses[row][col] == 1)
        pixels[row][col] = true;
      if (guesses[row][col] == 2)
        pixels[row][col] = blinker;
    }
  }
}

void receiveTurnEvent(int bytes) {

  if (bytes == 2) {
    byte x = Wire.read();
    byte y = Wire.read();

    response = 1;

    for (byte i=0; i<12; i++) {
      // hit
      if (readShipX(i)==x && readShipY(i)==y) {
        writeShip(i,x,y,true);
        response = 2;
      }
    }
    waiting = false;
  }
  else if (bytes == 1)
    if (Wire.read() == 2)
      guesses[lastGuessY][lastGuessX] = 2;
}

void updateDisplay(boolean pixels[8][8]) {
  for (byte row=0; row<8; row++)
    for (byte col=0; col<8; col++)
      lc.setLed(0,col,row,pixels[row][col]);
}
