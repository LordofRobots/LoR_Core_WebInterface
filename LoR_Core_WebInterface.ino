/* LORD of ROBOTS - LoR_Core_WeInterface - 202305222133
   Control inputs - LED Indication:
    Command Recived - Green LEDs
    none/Stop/standby - Red LEDs

  Drive configurations:
    Standard tank style

*/
#include <Adafruit_NeoPixel.h>

// version control and major control function settings
String Version = "Base Version : LoR Core Web Interface : 0.0.0";

// IO Interface Definitions
#define LED_DataPin 12
#define LED_COUNT 36
#define ControllerSelectPin 34
#define MotorEnablePin 13

// Motor Pin Definitions
#define motorPin_M1_A 5
#define motorPin_M1_B 14
#define motorPin_M2_A 18
#define motorPin_M2_B 26
#define motorPin_M3_A 23
#define motorPin_M3_B 19
#define motorPin_M4_A 15
#define motorPin_M4_B 33
#define motorPin_M5_A 27
#define motorPin_M5_B 25
#define motorPin_M6_A 32
#define motorPin_M6_B 4
const int motorPins_A[] = { motorPin_M1_A, motorPin_M2_A, motorPin_M3_A, motorPin_M4_A, motorPin_M5_A, motorPin_M6_A };
const int motorPins_B[] = { motorPin_M1_B, motorPin_M2_B, motorPin_M3_B, motorPin_M4_B, motorPin_M5_B, motorPin_M6_B };

// PWM Configuration Definitions
const int Motor_M1_A = 0;
const int Motor_M1_B = 1;
const int Motor_M2_A = 2;
const int Motor_M2_B = 3;
const int Motor_M3_A = 4;
const int Motor_M3_B = 5;
const int Motor_M4_A = 6;
const int Motor_M4_B = 7;
const int Motor_M5_A = 8;
const int Motor_M5_B = 9;
const int Motor_M6_A = 10;
const int Motor_M6_B = 11;
const int MOTOR_PWM_Channel_A[] = { Motor_M1_A, Motor_M2_A, Motor_M3_A, Motor_M4_A, Motor_M5_A, Motor_M6_A };
const int MOTOR_PWM_Channel_B[] = { Motor_M1_B, Motor_M2_B, Motor_M3_B, Motor_M4_B, Motor_M5_B, Motor_M6_B };
const int PWM_FREQUENCY = 20000;
const int PWM_RESOLUTION = 8;

// Function to control motor output based on input values
// Motor speed limits and starting speed
const int MAX_SPEED = 255;
const int MIN_SPEED = -255;
const int MIN_STARTING_SPEED = 140;
const int STOP = 0;
const int SerialControl_SPEED = 110;
bool INVERT = false;
void Set_Motor_Output(int Output, int Motor_ChA, int Motor_ChB) {
  int DEAD_BAND = 5;
  if (INVERT) Output = -Output;

  Output = constrain(Output, -127, 127);

  int Mapped_Value = map(abs(Output), 0, 127, MIN_STARTING_SPEED, MAX_SPEED);
  int A, B = 0;
  if (Output < -DEAD_BAND) {  // Rotate Clockwise
    A = 0;
    B = Mapped_Value;
  } else if (Output > DEAD_BAND) {  // Rotate Counter-Clockwise
    A = Mapped_Value;
    B = 0;
  } else {  // Rotation Stop
    A = STOP;
    B = STOP;
  }
  ledcWrite(Motor_ChA, A);  //send to motor control pins
  ledcWrite(Motor_ChB, B);
}

// Function to handle slew rate for motor speed ramping
int SlewRateFunction(int Input_Target, int Input_Current) {
  const int SLEW_RATE_MS = 5;
  int speedDiff = Input_Target - Input_Current;
  if (speedDiff > 0) Input_Current += min(speedDiff, SLEW_RATE_MS);
  else if (speedDiff < 0) Input_Current -= min(-speedDiff, SLEW_RATE_MS);
  constrain(Input_Current, -127, 127);
  return Input_Current;
}

// Function to handle serial control input
// Define variables to store the current motor speeds
bool STOP_FLAG = true;
long TIME_OUT = 1;
int Serial_Input_L_Set, Serial_Input_R_Set = 0;
int Serial_Input_L_Target, Serial_Input_R_Target = 0;
String LastCommand = "Stop";
boolean SerialControl() {
  float SPEED_Adjustment = 0.75;
  if (Serial2.available()) {
    String request = Serial2.readStringUntil('\n');  // ensure entire packet is recieved

    Serial.print("READ ---- ");
    Serial.println(request);  //reply

    while (Serial2.available()) { Serial2.read(); }  // clear/dump buffer

    STOP_FLAG = false;

    if (LastCommand == "Stop") {
      // Check the received string and change the robot's direction accordingly
      if (request.indexOf("Forward") != -1) {
        Serial_Input_L_Target = int(-SerialControl_SPEED * SPEED_Adjustment);
        Serial_Input_R_Target = int(SerialControl_SPEED * SPEED_Adjustment);
        TIME_OUT = millis() + 500;
        LastCommand = "Forward";
      } else if (request.indexOf("Backward") != -1) {
        Serial_Input_L_Target = int(SerialControl_SPEED * SPEED_Adjustment);
        Serial_Input_R_Target = int(-SerialControl_SPEED * SPEED_Adjustment);
        TIME_OUT = millis() + 500;
        LastCommand = "Backward";
      } else if (request.indexOf("Left") != -1) {
        Serial_Input_L_Target = SerialControl_SPEED;
        Serial_Input_R_Target = SerialControl_SPEED;
        TIME_OUT = millis() + 200;
        LastCommand = "Left";
      } else if (request.indexOf("Right") != -1) {
        Serial_Input_L_Target = -SerialControl_SPEED;
        Serial_Input_R_Target = -SerialControl_SPEED;
        TIME_OUT = millis() + 200;
        LastCommand = "Right";
      }
    } else if (request.indexOf("Stop") != -1) {
      Serial_Input_L_Target = STOP;
      Serial_Input_R_Target = STOP;
      STOP_FLAG = true;
      LastCommand = "Stop";
      while (Serial2.available()) { Serial2.read(); }  // clear/dump buffer
    } else {
      STOP_FLAG = true;
    }
  }
  if (millis() > TIME_OUT && TIME_OUT != 0) {
    Serial_Input_L_Target = STOP;
    Serial_Input_R_Target = STOP;
    STOP_FLAG = true;
    LastCommand = "Stop";
  } else if (LastCommand != "Stop") STOP_FLAG = false;

  Serial_Input_L_Set = SlewRateFunction(Serial_Input_L_Target, Serial_Input_L_Set);
  Serial_Input_R_Set = SlewRateFunction(Serial_Input_R_Target, Serial_Input_R_Set);

  if (STOP_FLAG) return false;
  else return true;
}

void Motor_Contol() {
  Set_Motor_Output(Serial_Input_L_Set, Motor_M1_A, Motor_M1_B);
  Set_Motor_Output(Serial_Input_L_Set, Motor_M2_A, Motor_M2_B);
  Set_Motor_Output(Serial_Input_R_Set, Motor_M5_A, Motor_M5_B);
  Set_Motor_Output(Serial_Input_R_Set, Motor_M6_A, Motor_M6_B);
}

void Motor_STOP() {
  Serial_Input_L_Set = SlewRateFunction(STOP, Serial_Input_L_Set);
  Serial_Input_R_Set = SlewRateFunction(STOP, Serial_Input_R_Set);
}

// NeoPixel Configurations
Adafruit_NeoPixel strip(LED_COUNT, LED_DataPin, NEO_GRB + NEO_KHZ800);
const uint32_t RED = strip.Color(255, 0, 0, 0);
const uint32_t GREEN = strip.Color(0, 255, 0, 0);
const uint32_t BLUE = strip.Color(0, 0, 255, 0);
const uint32_t WHITE = strip.Color(0, 0, 0, 255);
const uint32_t PURPLE = strip.Color(255, 0, 255, 0);
const uint32_t CYAN = strip.Color(0, 255, 255, 0);
const uint32_t YELLOW = strip.Color(255, 255, 0, 0);

// Set a specific color for the entire NeoPixel strip
void NeoPixel_SetColour(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
  }
  strip.show();  // Update strip with new contents
}

// Set up pins, LED PWM functionalities and Serial and Serial2 communication
void setup() {
  // Set up the pins
  pinMode(LED_DataPin, OUTPUT);
  pinMode(ControllerSelectPin, INPUT_PULLUP);
  pinMode(MotorEnablePin, OUTPUT);

  for (int i = 0; i < 6; i++) {
    pinMode(motorPins_A[i], OUTPUT);
    pinMode(motorPins_B[i], OUTPUT);
    digitalWrite(motorPins_A[i], 0);
    digitalWrite(motorPins_B[i], 0);
  }

  // output preset bias
  digitalWrite(LED_DataPin, 0);
  digitalWrite(MotorEnablePin, 1);

  // configure LED PWM functionalitites
  for (int i = 0; i < 6; i++) {
    ledcSetup(MOTOR_PWM_Channel_A[i], PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(MOTOR_PWM_Channel_B[i], PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(motorPins_A[i], MOTOR_PWM_Channel_A[i]);
    ledcAttachPin(motorPins_B[i], MOTOR_PWM_Channel_B[i]);
  }

  // Neopixels Configuration
  strip.begin();            // INITIALIZE NeoPixel strip object
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)

  // Serial comms configurations (USB for debug messages)
  Serial.begin(115200);  // USB Serial
  delay(1500);

  //debug messages
  Serial.print("Serial Control: ");

  //Serial Contol port configuration as input for control from modules like Camron. (Alternative control method)
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  //  secondary serial to communicate with the other devices like Camron module
  Serial.println("ready");
  delay(500);

  Serial.println("CORE System Ready! " + Version);
}

long minimumLED_displayTime = 200;
long LED_displayTime = 1;
void loop() {
  // Main loop to handle serial input
  if (SerialControl()) {  //Serial Control
    NeoPixel_SetColour(GREEN);
    LED_displayTime = millis() + minimumLED_displayTime;
  } else {  //Stop/Standby
    if (millis() > LED_displayTime) NeoPixel_SetColour(RED);
  }
  Motor_Contol();
}


