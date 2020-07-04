/*
   ThermoSystem
   Control for ThermoSystem (GPS and Stepper Driver)

   Aarhus University 2019
*/

#include <TinyGPS++.h>
#include "BasicStepperDriver.h"
#include <NeoSWSerial.h>

// ------------- Stepper Driver setup ------------- //
// Constants
#define MOTOR_STEPS 200             // Steps per revolution
#define RPM 10
#define MICROSTEPS 16               // 1 = full step, 2 = half step, etc.

// Pin Connections
#define DIR 4
#define STEP 5

// Setup
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);


// ----------- GPS Module (GY-GPSV3-NEO) ----------- //
// Constants
#define GPS_BAUD 9600

// Pin Connections
#define RX 3
#define TX 2

// The TinyGPS++ object
TinyGPSPlus gps;
NeoSWSerial ss(RX, TX);


// ------------------ Variables ------------------ //
bool newCommand = false;
bool GPS_Status = false;
bool moveIn = false;
bool moveOut = false;
bool GPS_ENC = false;
bool GPS_SAT = false;
String command;
boolean newData = false;
double Longitude = 00.0000;
double Latitude = 00.0000;
double Altitude = 00.00;

int Position = 0;
const int MaxPosition = 5000;

// --------------------- SETUP --------------------- //
void setup() {
  // Stepper Driver
  stepper.begin(RPM, MICROSTEPS);

  // GPS Module
  ss.begin(GPS_BAUD);

  // Serial Communication
  Serial.begin(115200);
}

// ---------------------- LOOP ---------------------- //
void loop() {
  recvWithEndMarker();

  while (moveOut == true) {
    if (Position < MaxPosition){
        stepper.move(2);
        Position += 2;
    }
    else if (Position >= MaxPosition){
      moveOut = false;
      Serial.println("MaxPos");
    }
    Position = min(MaxPosition,Position);
    recvWithEndMarker();
  }
  while (moveIn == true) {
    stepper.move(-2);
    Position -= 2;
    Position = max(0,Position);
    recvWithEndMarker();
  }

  if (gps.location.isValid()) {
    Latitude = gps.location.lat();
    Longitude = gps.location.lng();
    Altitude = gps.altitude.meters();
  }


  smartDelay(250);
}

// -------------------- FUNCTIONS -------------------- //

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


// Receive Commands
void recvWithEndMarker() {
  while (Serial.available())
  {
    char c = Serial.read();
    //Serial.print(c);              // Uncomment to see command string flow
    if (c == '<')
    {
      newCommand = true;
    }
    else if (c == '>' && newCommand == true)
    {
      newCommand = false;
      parseCommand(command);
      command = "";
    }
    else if (newCommand == true)
    {
      command += c;
    }
  }
}

// Command handler
void parseCommand(String com)
{
  String part1;
  String part2;

  part1 = com.substring(0, com.indexOf('.'));
  part2 = com.substring(com.indexOf('.') + 1);

  if (part1.equalsIgnoreCase("GPS")) {
    if (part2.equalsIgnoreCase("Status")) {
      String Status = "Status...";
      Serial.println(Status);
    }
    else if (part2.equalsIgnoreCase("Sat")) {
      if (gps.satellites.isValid() && gps.location.isValid()) {
        Serial.print(gps.satellites.value());
      }
    }
    else {
      Serial.println("NACK");
    }
  }
  else if (part1.equalsIgnoreCase("P")) {
    //if (gps.location.isValid()) {
      Serial.print("<");
      // Serial.print(gps.location.lat(), 6);
      Serial.print(Latitude, 6);
      Serial.print(F(","));
      // Serial.print(gps.location.lng(), 6);
      Serial.print(Longitude, 6);
      Serial.print(F(","));
      // Serial.print(gps.altitude.meters(), 2);
      Serial.print(Altitude, 2);
      Serial.print(F(","));
      Serial.print(gps.location.isValid());
      Serial.println(">");
    //}
  }
  else if (part1.equalsIgnoreCase("Step")) {
    if (part2.equalsIgnoreCase("out")) {
      moveIn = false;
      moveOut = true;
      //Serial.println("ACK OUT");
    }
    else if (part2.equalsIgnoreCase("in")) {
      moveIn = true;
      moveOut = false;
      //Serial.println("ACK IN");
    }
    else if (part2.equalsIgnoreCase("stop")) {
      moveIn = false;
      moveOut = false;
      //Serial.println("ACK STOP");
    }
    else {
      Serial.println("NACK");
    }
  }
  else if (part1.equalsIgnoreCase("System")) {
    if (part2.equalsIgnoreCase("Status")) {
      Serial.println("ACK");
    }
    else {
      Serial.println("NACK");
    }
  }
  else if (part1.equalsIgnoreCase("MotorSpeed")) {
    if (part2.toInt() >= 0 || part2.toInt() <= 120) {
      //Serial.println("ACK MotorSpeed");
      stepper.setRPM(part2.toInt());
    }
    else {
      Serial.println("NACK");
    }
  }
  else {
    Serial.println("NACK");
  }
}
