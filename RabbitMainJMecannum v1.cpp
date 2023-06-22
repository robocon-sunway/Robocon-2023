#include <CytronMotorDriver.h>
#include <math.h> //math package
#include <SoftwareSerial.h>
#include <Cytron_PS2Shield.h>
#include <Servo.h>

Cytron_PS2Shield ps2(10, 11); // SoftwareSerial: Rx and Tx pin

// UNO PWM pins | 3, 5, 6, 9, 10, or 11
// MEGA PWM pins | 2 - 13, 44 - 46
// MEGA DIGITAL pins | 22 - 43, 47 - 49

// R2 device: 2 DC MOTOR & 2 SERVO MOTOR
// R2 purpose: to move front/up, back/down, left, right with 2 dc motor, and or and rotate arm and clamp with 2 servo motors.

// Wheels
//  (PWM_DIR, Pwm Pin, Dir Pin)
CytronMD frontLMotor(PWM_DIR, 4, 2);
CytronMD frontRMotor(PWM_DIR, 5, 3);

CytronMD backLMotor(PWM_DIR, 9, 7);
CytronMD backRMotor(PWM_DIR, 8, 6);

// BASE PS2 BUTTON INPU
int up_button = 0;
int down_button = 0;

// Joystick Init.
double y = 0;
double x = 0;
double rx = 0;

// LINEAR ACTUATOR
//  pwm 3, 5, 6, 9, 10, 11
//  twelve servo objects can be created on most boards
Servo actuator;
int pos = 0;

void extendLinearActuator()
{
  actuator.write(180 * 1.25);
}

void retractLinearActuator()
{
  actuator.write(0 * 1.25);
}

void setup()
{
  Serial.begin(9600);
  ps2.begin(9600);

  // Base wheels output
  frontLMotor.setSpeed(0);
  frontRMotor.setSpeed(0);
  backLMotor.setSpeed(0);
  backRMotor.setSpeed(0);

  // linear actuator
  actuator.attach(12);              // PWM pin
  actuator.writeMicroseconds(1000); // let it retract
}

void loop()
{
  delay(50);

  // PS2 Button Init initialisation
  up_button = ps2.readButton(PS2_UP);
  down_button = ps2.readButton(PS2_DOWN);

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

  // double check if correct motors
  frontLMotor.setSpeed(frontLeftPower);  // front left
  frontRMotor.setSpeed(frontRightPower); // back left
  backLMotor.setSpeed(backLeftPower);    // front right
  backRMotor.setSpeed(backRightPower);   // back right
  // ===== Joystick Holonomic Movement =====

  if (up_button == 0)
  {
    extendLinearActuator();
  }
  else if (down_button == 0)
  {
    retractLinearActuator();
  }
}