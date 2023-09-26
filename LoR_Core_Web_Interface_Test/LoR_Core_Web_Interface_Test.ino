#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ESPmDNS.h>

/* Put your SSID & Password */
const char* ssid = "MiniBot";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

#define LED_DataPin 12
#define LED_COUNT 36
bool LED1status = LOW;
bool LED2status = LOW;

// Motor Pin Definitions
#define MotorEnablePin 13
#define motorPin_M1_A 14
#define motorPin_M1_B 5
#define motorPin_M2_A 26
#define motorPin_M2_B 18
#define motorPin_M3_A 23
#define motorPin_M3_B 19
#define motorPin_M4_A 15
#define motorPin_M4_B 33
#define motorPin_M5_A 25
#define motorPin_M5_B 27
#define motorPin_M6_A 4
#define motorPin_M6_B 32
const int motorPins_A[] = { motorPin_M1_A, motorPin_M2_A, motorPin_M3_A, motorPin_M4_A, motorPin_M5_A, motorPin_M6_A };
const int motorPins_B[] = { motorPin_M1_B, motorPin_M2_B, motorPin_M3_B, motorPin_M4_B, motorPin_M5_B, motorPin_M6_B };

// Motor PWM Configuration Definitions
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

// Process joystick input and calculate motor speeds
bool MecanumDrive_Enabled = false;
const int DEAD_BAND = 20;
const float TURN_RATE = 1.5;
int Motor_FrontLeft_SetValue, Motor_FrontRight_SetValue, Motor_BackLeft_SetValue, Motor_BackRight_SetValue = 0;
void Motion_Control(int LY_Axis, int LX_Axis, int RX_Axis) {
  int FrontLeft_TargetValue, FrontRight_TargetValue, BackLeft_TargetValue, BackRight_TargetValue = 0;
  int ForwardBackward_Axis = LY_Axis;
  int StrafeLeftRight_Axis = LX_Axis;
  int TurnLeftRight_Axis = -RX_Axis;

  //Set deadband
  if (abs(ForwardBackward_Axis) < DEAD_BAND) ForwardBackward_Axis = 0;
  if (abs(StrafeLeftRight_Axis) < DEAD_BAND) StrafeLeftRight_Axis = 0;
  if (abs(TurnLeftRight_Axis) < DEAD_BAND) TurnLeftRight_Axis = 0;

  //Calculate strafe values
  FrontLeft_TargetValue = -ForwardBackward_Axis + (StrafeLeftRight_Axis * MecanumDrive_Enabled);
  BackLeft_TargetValue = -ForwardBackward_Axis - (StrafeLeftRight_Axis * MecanumDrive_Enabled);
  FrontRight_TargetValue = ForwardBackward_Axis + (StrafeLeftRight_Axis * MecanumDrive_Enabled);
  BackRight_TargetValue = ForwardBackward_Axis - (StrafeLeftRight_Axis * MecanumDrive_Enabled);

  //calculate rotation values
  if (abs(TurnLeftRight_Axis) > DEAD_BAND) {
    FrontLeft_TargetValue += (TURN_RATE * TurnLeftRight_Axis);
    BackLeft_TargetValue += (TURN_RATE * TurnLeftRight_Axis);
    FrontRight_TargetValue += (TURN_RATE * TurnLeftRight_Axis);
    BackRight_TargetValue += (TURN_RATE * TurnLeftRight_Axis);
  }

  //constrain to joystick range
  FrontLeft_TargetValue = constrain(FrontLeft_TargetValue, -127, 127);
  BackLeft_TargetValue = constrain(BackLeft_TargetValue, -127, 127);
  FrontRight_TargetValue = constrain(FrontRight_TargetValue, -127, 127);
  BackRight_TargetValue = constrain(BackRight_TargetValue, -127, 127);

  //set motor speed through slew rate function
  Motor_FrontLeft_SetValue = SlewRateFunction(FrontLeft_TargetValue, Motor_FrontLeft_SetValue);
  Motor_FrontRight_SetValue = SlewRateFunction(FrontRight_TargetValue, Motor_FrontRight_SetValue);
  Motor_BackLeft_SetValue = SlewRateFunction(BackLeft_TargetValue, Motor_BackLeft_SetValue);
  Motor_BackRight_SetValue = SlewRateFunction(BackRight_TargetValue, Motor_BackRight_SetValue);
}

// Function to handle slew rate for motor speed ramping
// Slew rate for ramping motor speed
const int SLEW_RATE_MS = 20;
int SlewRateFunction(int Input_Target, int Input_Current) {
  int speedDiff = Input_Target - Input_Current;
  if (speedDiff > 0) Input_Current += min(speedDiff, SLEW_RATE_MS);
  else if (speedDiff < 0) Input_Current -= min(-speedDiff, SLEW_RATE_MS);
  constrain(Input_Current, -127, 127);
  return Input_Current;
}

// Function to control motor output based on input values
// Motor speed limits and starting speed
const int MAX_SPEED = 255;
const int MIN_SPEED = -255;
const int MIN_STARTING_SPEED = 140;
const int STOP = 0;
bool INVERT = false;
void Set_Motor_Output(int Output, int Motor_ChA, int Motor_ChB) {
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

// configure motor output
void Motor_Control() {
  Set_Motor_Output(Motor_FrontLeft_SetValue, Motor_M1_A, Motor_M1_B);
  Set_Motor_Output(Motor_BackLeft_SetValue, Motor_M2_A, Motor_M2_B);
  Set_Motor_Output(Motor_FrontRight_SetValue, Motor_M5_A, Motor_M5_B);
  Set_Motor_Output(Motor_BackRight_SetValue, Motor_M6_A, Motor_M6_B);
}

// stop motors from spinning
void Motor_STOP() {
  Set_Motor_Output(STOP, Motor_M1_A, Motor_M1_B);
  Set_Motor_Output(STOP, Motor_M2_A, Motor_M2_B);
  Set_Motor_Output(STOP, Motor_M5_A, Motor_M5_B);
  Set_Motor_Output(STOP, Motor_M6_A, Motor_M6_B);
}

// Set a specific color for the entire NeoPixel strip
// NeoPixel Configurations
Adafruit_NeoPixel strip(LED_COUNT, LED_DataPin, NEO_GRB + NEO_KHZ800);
const uint32_t RED = strip.Color(255, 0, 0, 0);
const uint32_t GREEN = strip.Color(0, 255, 0, 0);
const uint32_t BLUE = strip.Color(0, 0, 255, 0);
const uint32_t WHITE = strip.Color(0, 0, 0, 255);
const uint32_t PURPLE = strip.Color(255, 0, 255, 0);
const uint32_t CYAN = strip.Color(0, 255, 255, 0);
const uint32_t YELLOW = strip.Color(255, 255, 0, 0);
void NeoPixel_SetColour(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip.show();                                // Update strip with new contents
  }
}

//====================================================
//===              handlers                        ===
//====================================================

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

//====================================================
//===              web page                        ===
//====================================================
String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 100px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>LoR MiniBots</h1>\n";
  ptr +="<h3>Web Control Interface</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">STOP</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">FORWARD</a>\n";}

  /*if(led2stat)
  {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
  {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}*/

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

//====================================================
//===                 setup                        ===
//====================================================

void setup() {
  Serial.begin(115200);
  pinMode(LED_DataPin, OUTPUT);
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

  // Neopixels Configuration
  strip.begin();            // INITIALIZE NeoPixel strip object
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)

  // configure LED PWM functionalitites
  for (int i = 0; i < 6; i++) {
    ledcSetup(MOTOR_PWM_Channel_A[i], PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(MOTOR_PWM_Channel_B[i], PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(motorPins_A[i], MOTOR_PWM_Channel_A[i]);
    ledcAttachPin(motorPins_B[i], MOTOR_PWM_Channel_B[i]);
  }

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
   if (!MDNS.begin("robot")) Serial.println("Error setting up MDNS responder!");
  MDNS.addService("http", "tcp", 80);

  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  /*server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);*/
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

//====================================================
//===              main loop                       ===
//====================================================

void loop() {
  server.handleClient();

  if(LED1status){
    NeoPixel_SetColour(GREEN);
    Motion_Control(150, 0, 0);  // Joystick control
    delay(5);
    Motor_Control();
  }
  else if(!LED1status){
    NeoPixel_SetColour(RED);
    Motion_Control(0, 0, 0);  // Joystick control
    delay(5);
    Motor_Control();
  }

  /*if(LED2status){
    NeoPixel_SetColour(BLUE);
    Motion_Control(-150, 0, 0);  // Joystick control
    delay(5);
    Motor_Control();
  }
  else if(!LED2status){
    NeoPixel_SetColour(RED);
    Motion_Control(0, 0, 0);  // Joystick control
    delay(5);
    Motor_Control();
  }*/
}