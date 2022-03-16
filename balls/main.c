#include <math.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"

#include "pico/stdlib.h"

#include "external/fonts.h"
#include "external/st7735.h"
#include "external/ICM20948.h"

#include "balls.c"

int main() {
    ScreenLoop();
}

void setup() {
  // Screen
  stdio_init_all();
  setvbuf ( stdout , NULL , _IONBF , 0 );
  sleep_ms(1000);
  ST7735_Init();
  ST7735_DrawImage(0, 0, 80, 160, plaao_logo);
  sleep_ms(2000);
  printf("%s\n", "Initialising");

  // Accelerometer
  i2c_init(i2c0, 400 * 1000);
  gpio_set_function(4, GPIO_FUNC_I2C);
  gpio_set_function(5, GPIO_FUNC_I2C);
  gpio_pull_up(4);
  gpio_pull_up(5);
  sleep_ms(1000);
  
  IMU_EN_SENSOR_TYPE enMotionSensorType;

  imuInit(&enMotionSensorType);
  if (IMU_EN_SENSOR_TYPE_ICM20948 != enMotionSensorType) {
    printf("Failed to initialise IMU...");
  }

  printf("IMU initialised!");

  ST7735_FillScreen(ST7735_BLACK);
}

// https://forums.raspberrypi.com/viewtopic.php?t=302960
uint32_t rnd() {
  int k, random=0;
  volatile uint32_t *rnd_reg=(uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
  
  for(k=0; k<32; k++){
  
  random = random << 1;
  random = random + (0x00000001 & (*rnd_reg));

  }
  return random;
}

void ScreenLoop() {
  setup();

  Ball playerBall = {
    {40, 80}, 
    {
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}
    },
    0, 0, 0.94, ST7735_CYAN, ST7735_BLUE
  };

  Ball enemyBall = {
    {40, 80}, 
    {
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
      {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}
    },
    0, 0, 0.94, ST7735_RED, ST7735_COLOR565(0xdd, 0x9a, 0)
  };

  Ball crazyBalls[6];

  for (int i=0; i<6; i++) {
    Ball crazyBall = {
      {40, 80}, 
      {
        {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
        {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
        {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}, 
        {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80},
        {40, 80}, {40, 80}, {40, 80}, {40, 80}, {40, 80}
      },
      0, 0, 0.94, ST7735_COLOR565(0x88, 0x88, 0x88), ST7735_COLOR565(0x33, 0x33, 0x33)
    };

    crazyBalls[i] = crazyBall;
  }

  float curX = 0;
  float curY = 0;
  float curZ = 0;

  float firstX = 0;
  float firstY = 0;
  float firstZ = 0;

  sleep_ms(1000);
  icm20948AccelRead(&firstX, &firstY, &firstZ);
  sleep_ms(1000);

  while (true) {
    icm20948AccelRead(&curX, &curY, &curZ);
    
    float diffX = firstX - curX;
    float diffY = firstY - curY;

    //printf("x %f, y %f\n", diffX, diffY);

    /* Static movement code
    if (diffX > 0.15) {
      curPos.x -= 1;
    } else if (diffX < -0.15) {
      curPos.x += 1;
    }

    if (diffY > 0.15) {
      curPos.y -= 1;
    } else if (diffY < -0.15) {
      curPos.y += 1;
    }
    */

    // Player ball - move with tilt.
    // Amplify friction if the board is (generally) stable.
    if (fabs(diffX) > 0.015) {
      playerBall.speedX += diffX * 0.33;
    } else {
      playerBall.speedX *= playerBall.friction * playerBall.friction;
    }
    
    if (fabs(diffY) > 0.015) {
      playerBall.speedY += diffY * 0.33;
    } else {
      playerBall.speedY *= playerBall.friction * playerBall.friction;
    }

    // Enemy ball - move opposite from tilt
    if (fabs(diffX) > 0.015) {
      enemyBall.speedX -= diffX * 0.33;
    } else {
      enemyBall.speedX *= enemyBall.friction * enemyBall.friction;
    }
    
    if (fabs(diffY) > 0.015) {
      enemyBall.speedY -= diffY * 0.33;
    } else {
      enemyBall.speedY *= enemyBall.friction * enemyBall.friction;
    }

    // Crazy balls - move randomly
    for (int i=0; i<6; i++) {
      uint32_t roll = rnd();
      if (roll < (0xffffffff / 6)) { // About 1/6 chance
        crazyBalls[i].speedX += (((float)rnd() / (float)0xffffffff) - 0.5) * 3;
        crazyBalls[i].speedY += (((float)rnd() / (float)0xffffffff) - 0.5) * 3;
      }

      ballPhysicsStep(&crazyBalls[i]);
      finishBallMovement(&crazyBalls[i]);
    }

    ballPhysicsStep(&playerBall);
    ballPhysicsStep(&enemyBall);

    finishBallMovement(&playerBall);
    finishBallMovement(&enemyBall);
    
    sleep_ms(10);
  }
}
