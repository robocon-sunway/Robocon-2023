#include <CytronMotorDriver.h>
#include <math.h> //math package
#include <SoftwareSerial.h>
#include <Cytron_PS2Shield.h>

Cytron_PS2Shield ps2(10, 11); // SoftwareSerial: Rx and Tx pin

// UNO PWM pins | 3, 5, 6, 9, 10, or 11
// MEGA PWM pins | 2 - 13, 44 - 46
// MEGA DIGITAL pins | 22 - 43, 47 - 49

// R2 device: 2 DC MOTOR & 2 SERVO MOTOR
// R2 purpose: to move front/up, back/down, left, right with 2 dc motor, and or and rotate arm and clamp with 2 servo motors.

// BASE PS2 BUTTON INPUT
int tri_up = 0;
int cross_down = 0;
int circ_right = 0;
int square_left = 0;

int up_button = 0;
int right_button = 0;
int down_button = 0;
int left_button = 0;

int L1 = 0;
int R1 = 0;

int L2 = 0;
int R2 = 0;

// FLYWHEEL
unsigned long prevTime_FlyWheelTimer = 0;
unsigned long currentTime_FlyWheelTimer = 0;
int FLYWHEEL_TIMEDIFF = 80;
unsigned long flywheel_diff = 0;

int flyMinimumLevel = 6;
int flyWheelLevel = flyMinimumLevel;
int flyMaxLevel = 25;

int flyWheelSpeedFactor = 10;
int flyWheelSpeed = 0;

// AN1 & IN1 = LEFT | AN2 & IN2 = RIGHT
//  (PWM_DIR, Pwm Pin, Dir Pin)
CytronMD leftFlyWheel(PWM_DIR, 7, 12);
CytronMD rightFlyWheel(PWM_DIR, 9, 13);
// FLYWHEEL

// Wheels
//  (PWM_DIR, Pwm Pin, Dir Pin)
CytronMD frontLMotor(PWM_DIR, 5, 50);
CytronMD backLMotor(PWM_DIR, 4, 51);

CytronMD frontRMotor(PWM_DIR, 3, 22);
CytronMD backRMotor(PWM_DIR, 6, 23);

int maxMovement = 65;
int movementMode = 1;
// Wheels

// Pneumatic
int shootingPneumatic = 47;
int reloadPneumatic = 49;
// Pneumatic

// Reload Motor
int reloadMotorPWM = 2;
int reloadMotorPin = 25;
int reloadMotorTiming = 110;

int reloadMode = 1;
// Reload Motor

// Joystick Init.
double y = 0;
double x = 0;
double rx = 0;
// Joystick Init

void setup()
{
  // PS2 Shield Initialization
  ps2.begin(9600);
  Serial.begin(9600);

  // Reload Motor Initialization
  pinMode(reloadMotorPWM, OUTPUT);
  pinMode(reloadMotorPin, OUTPUT);

  pinMode(shootingPneumatic, OUTPUT);
  pinMode(reloadPneumatic, OUTPUT);

  // Base wheels output
  frontLMotor.setSpeed(0);
  frontRMotor.setSpeed(0);
  backLMotor.setSpeed(0);
  backRMotor.setSpeed(0);

  // Base flywheel output
  leftFlyWheel.setSpeed(0);
  rightFlyWheel.setSpeed(0);

  pushReloadPneumatic();
  pullshootingPneumatic();
}

int rMotorMaxSpeed = 175;
// Reload Motor
void rMotorUp()
{
  analogWrite(reloadMotorPWM, rMotorMaxSpeed);
  digitalWrite(reloadMotorPin, HIGH);
}
void rMotorDown()
{
  analogWrite(reloadMotorPWM, rMotorMaxSpeed);
  digitalWrite(reloadMotorPin, LOW);
}
void rMotorStop()
{
  analogWrite(reloadMotorPWM, 0);
}
// Reload Motor

// Pneumatic
void pushReloadPneumatic()
{
  digitalWrite(reloadPneumatic, LOW);
}
void pullReloadPneumatic()
{
  digitalWrite(reloadPneumatic, HIGH);
}

void pushshootingPneumatic()
{
  digitalWrite(shootingPneumatic, LOW);
}
void pullshootingPneumatic()
{
  digitalWrite(shootingPneumatic, HIGH);
}
// Pneumatic

void switchOffEverything()
{
  rMotorStop();

  frontLMotor.setSpeed(0);
  frontRMotor.setSpeed(0);
  backLMotor.setSpeed(0);
  backRMotor.setSpeed(0);

  leftFlyWheel.setSpeed(0);
  rightFlyWheel.setSpeed(0);
}

// must be completely still
void moveRmotorToRingHeight()
{
  rMotorUp();
  delay(reloadMotorTiming);
}

// must be completely still
void moveRmotorBack()
{
  rMotorDown();
  delay(500);
}

void reloadFullAction()
{
  // Pull reload pneumatic and delay
  pullReloadPneumatic();
  delay(800);

  // Push back pneumatic and dely
  pushReloadPneumatic();
  delay(800);

  // Move till the perfect ring height (need testing)
  moveRmotorToRingHeight();
}

void shoot()
{
  pushshootingPneumatic();
  delay(250);
  pullshootingPneumatic();
  delay(100);
}

void loop()
{
  delay(50);

  // PS2 Button Init initialisation
  tri_up = ps2.readButton(PS2_TRIANGLE);
  cross_down = ps2.readButton(PS2_CROSS);
  square_left = ps2.readButton(PS2_SQUARE);
  circ_right = ps2.readButton(PS2_CIRCLE);

  up_button = ps2.readButton(PS2_UP);
  right_button = ps2.readButton(PS2_RIGHT);
  down_button = ps2.readButton(PS2_DOWN);
  left_button = ps2.readButton(PS2_LEFT);

  L1 = ps2.readButton(PS2_LEFT_1);
  R1 = ps2.readButton(PS2_RIGHT_1);

  L2 = ps2.readButton(PS2_LEFT_2);
  R2 = ps2.readButton(PS2_RIGHT_2);

  // ===== Joystick Holonomic Movement =====
  // analog joysticks initialisation
  y = -((ps2.readButton(PS2_JOYSTICK_LEFT_Y_AXIS)) - 128);
  x = (ps2.readButton(PS2_JOYSTICK_LEFT_X_AXIS) - 128);
  rx = ps2.readButton(PS2_JOYSTICK_RIGHT_X_AXIS) - 128;

  // formula that moves the wheels | -128 - 128
  double frontLeftPower = (y + x + rx);
  double backLeftPower = (y - x + rx);
  double frontRightPower = (y - x - rx);
  double backRightPower = (y + x - rx);

  if (movementMode == 1)
  {
    frontLeftPower = map(frontLeftPower, -128, 128, -maxMovement, maxMovement);
    frontRightPower = map(frontRightPower, -128, 128, -maxMovement, maxMovement);
    backLeftPower = map(backLeftPower, -128, 128, -maxMovement, maxMovement);
    backRightPower = map(backRightPower, -128, 128, -maxMovement, maxMovement);
  }

  // double check if correct motors
  frontLMotor.setSpeed(frontLeftPower);  // front left
  frontRMotor.setSpeed(frontRightPower); // back left
  backLMotor.setSpeed(backLeftPower);    // front right
  backRMotor.setSpeed(backRightPower);   // back right
  // ===== Joystick Holonomic Movement =====

  // ===== Flywheel =====
  if (tri_up == 0 || cross_down == 0)
  {
    currentTime_FlyWheelTimer = millis(); // set current timer
    flywheel_diff = currentTime_FlyWheelTimer - prevTime_FlyWheelTimer;

    if (flywheel_diff > FLYWHEEL_TIMEDIFF)
    {
      prevTime_FlyWheelTimer = currentTime_FlyWheelTimer;

      // Go down a level
      if (cross_down == 0)
      {
        // Only allow flyWheelLevel to go till 2
        if (flyWheelLevel > flyMinimumLevel)
        {
          flyWheelLevel -= 1;
          flyWheelSpeed = flyWheelSpeedFactor * flyWheelLevel;
        }
        else if (flyWheelLevel == flyMinimumLevel)
        {
          flyWheelLevel = flyMinimumLevel - 1;
          flyWheelSpeed = 0;
        }
      }
      else if (tri_up == 0)
      {
        // Only allow flyWheelLevel to go up till flyMaxLevel
        if (flyWheelLevel < flyMaxLevel)
        {
          flyWheelLevel += 1;
          flyWheelSpeed = flyWheelSpeedFactor * flyWheelLevel;
        }
      }
    }
  }

  leftFlyWheel.setSpeed(flyWheelSpeed);
  rightFlyWheel.setSpeed(flyWheelSpeed);
  // ===== Flywheel =====

  // ===== Reload Motor =====
  if (square_left == 0)
  {
    rMotorDown();
  }
  else if (circ_right == 0)
  {
    rMotorUp();
  }
  else
  {
    rMotorStop();
  }
  // ===== Reload Motor =====

  // ===== Pneumatics Related =====
  if (left_button == 0)
  {
    pushReloadPneumatic();
  }
  else if (right_button == 0)
  {
    pullReloadPneumatic();
  }
  else if (up_button == 0)
  {
    pushshootingPneumatic();
  }
  else if (down_button == 0)
  {
    pullshootingPneumatic();
  }

  // ===== Pneumatics Related =====

  if (L1 == 0)
  {
    reloadFullAction();
  }
  else if (R1 == 0)
  {
    shoot();
  }

  if (L2 == 0)
  {
    if (movementMode == 2)
    {
      movementMode = 1;
    }
    else
    {
      movementMode = 2;
    }
  }
}