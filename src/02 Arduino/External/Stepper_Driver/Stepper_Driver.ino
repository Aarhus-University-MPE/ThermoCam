/*
   Stepper driver
   Control for Robotics project

   Aarhus University 2019
*/

#include "BasicStepperDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// 1=full step, 2=half step etc.
#define MICROSTEPS 1

// All the wires needed for full functionality
#define UR_Input 3
#define DIR 4
#define STEP 5

// Setup
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
#define Revolutions 4                                       // Number of revolutions per input (360 deg * Revolutions)


void setup() {
  stepper.begin(RPM, MICROSTEPS);
  attachInterrupt(digitalPinToInterrupt(UR_Input), Rotate, RISING);
}

void loop() {
}

void Rotate() {
  stepper.rotate(360 * Revolutions);
}
