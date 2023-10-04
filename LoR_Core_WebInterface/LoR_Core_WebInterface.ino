/* LORD of ROBOTS - LoR_Core_WeInterface - 202305222133
   Control inputs - LED Indication:
    Command Recived - Green LEDs
    none/Stop/standby - Red LEDs

  Drive configurations:
    Standard tank style

*/
#include <Adafruit_NeoPixel.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_system.h>
#include "esp_wifi.h"

// version control and major control function settings
String Version = "Base Version : LoR Core Web Interface : 0.0.0";

//====================================================
//===         Customizable Parameters              ===
//====================================================

// SSID & Password Definitions
const String ssid = "MiniBot";
const String password = "password";

// Drive Speeds (0%-100%)
int highSpeed = 90;
int lowSpeed = 50;

//====================================================

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


//====================================================
//===              Motor Controls                  ===
//====================================================

/// Function to control motor output
// Motor speed limits and starting speed
const int MIN_STARTING_SPEED = 150;
const int MAX_SPEED = 255;
const int STOP = 0;
void Set_Motor_Output(int Output, int Motor_ChA, int Motor_ChB) {
  int Mapped_Value = map(abs(Output), 0, 100, MIN_STARTING_SPEED, MAX_SPEED);
  int A, B = 0;
  if (Output < 0) {  // Rotate Clockwise
    A = 0;
    B = Mapped_Value;
  } else if (Output > 0) {  // Rotate Counter-Clockwise
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
void Motor_Control(int Left_Drive_Power, int Right_Drive_Power) {
  Set_Motor_Output(Left_Drive_Power, Motor_M1_A, Motor_M1_B);
  Set_Motor_Output(Left_Drive_Power, Motor_M2_A, Motor_M2_B);
  Set_Motor_Output(-Right_Drive_Power, Motor_M5_A, Motor_M5_B);
  Set_Motor_Output(-Right_Drive_Power, Motor_M6_A, Motor_M6_B);
}

// stop motors from spinning
void Motor_STOP() {
  Set_Motor_Output(STOP, Motor_M1_A, Motor_M1_B);
  Set_Motor_Output(STOP, Motor_M2_A, Motor_M2_B);
  Set_Motor_Output(STOP, Motor_M5_A, Motor_M5_B);
  Set_Motor_Output(STOP, Motor_M6_A, Motor_M6_B);
}

//====================================================
//===              NeoPixels                       ===
//====================================================

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

//====================================================
//===              Web Page                        ===
//====================================================

// Web page (HTML, CSS, JavaScript) for controlling the robot
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>LORD of ROBOTS</title>
    <meta name="viewport" content="width=device-width, height=device-height, initial-scale=1.0, user-scalable=no\">
    <style>
      body {
        text-align: center;
        margin: 0 auto;
        padding-top: 30px;
        background-color: #001336;
      }

      h1 {
        font-family: Monospace;
        color: white;
        margin: 20px auto 50px;
      }

      h3 {
        color: #b3c1db;
        margin-bottom: 20px;
        font-style: italic;
      }

      .button {
        background-color: #b3c1db;
        width: 100px;
        height: 75px;
        color: #001336;
        font-size: 20px;
        font-weight: bold;
        text-align: center;
        border-radius: 5px;
        border: 3px solid;
        border-color: white;
        display: inline-block;
        margin: 6px 6px;
        cursor: pointer;
        -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
        -webkit-user-select: none;
        /* Chrome, Safari, Opera */
        -moz-user-select: none;
        /* Firefox all */
        -ms-user-select: none;
        /* IE 10+ */
        user-select: none;
        /* Likely future */
      }

      .button:active {
        background-color: #001336;
        /* Change background color when pressed */
        color: #ccd6e7;
        border: 3px solid white;
      }

      .slider {
        position: relative;
        display: inline-block;
        -webkit-tap-highlight-color: transparent;
        cursor: pointer;
        width: 100px;
        height: 40px;
        border-radius: 50px;
        background-color: #001336;
        border: 3px solid white;
        margin-bottom: 20px;
      }

      .slider:before {
        position: absolute;
        content: "Low";
        font-style: italic;
        color: #001336;
        line-height: 30px;
        vertical-align: middle;
        border-radius: 50px;
        height: 30px;
        width: 50px;
        left: 5px;
        bottom: 5px;
        background-color: #ccd6e7;
        -webkit-transition: .4s;
        transition: .4s;
      }

      .switch input {
        opacity: 0;
        width: 0;
        height: 0;
      }

      input:checked+.slider:before {
        content: 'High';
        -webkit-transform: translateX(40px);
        -ms-transform: translateX(40px);
        transform: translateX(40px);
      }

      #buttons {
        display: inline-block;
        text-align: center;
      }
      
      .emptySpace {
        width: 100px;
        height: 30px;
        display: inline-block;
        margin: 6px 6px;
      }
      
    </style>
  </head>
  <body oncontextmenu="return false;">
    <h3>LORD of ROBOTS</h3>
    <h1>MiniBot Control Interface</h1>
    <p style="color: white; font-family: Monospace; font-size: 15px">Drive Speed</p>
    <label class="switch">
      <input type="checkbox" id="toggle-switch">
      <span class="slider"></span>
    </label>
    <br>
    <div id="buttons">
      <button class="button" onpointerdown="sendData('forward')" onpointerup="releaseData()" id="forward-button">Forward</button>
      <br>
      <button class="button" onpointerdown="sendData('left')" onpointerup="releaseData()" id="left-button">Left</button>
      <button class="button" onpointerdown="sendData('stop')" onpointerup="releaseData()" id="stop-button">Stop</button>
      <button class="button" onpointerdown="sendData('right')" onpointerup="releaseData()" id="right-button">Right</button>
      <br>
      <button class="button" onpointerdown="sendData('backward')" onpointerup="releaseData()" id="backward-button">Back</button>
    </div>
    <div class="emptySpace" style="width: 75px; height: 25px"></div>
    <div id="buttons">
      <br>
      <button class="button" onpointerdown="sendData('functionY')" onpointerup="releaseData()" id="functionY">Y</button>
      <br>
      <button class="button" onpointerdown="sendData('functionX')" onpointerup="releaseData()" id="functionX">X</button>
      <div class="emptySpace"></div>
      <button class="button" onpointerdown="sendData('functionA')" onpointerup="releaseData()" id="functionA">A</button>
      <br>
      <button class="button" onpointerdown="sendData('functionB')" onpointerup="releaseData()" id="functionB">B</button>
      <br>
    </div>
    <script>
      var isButtonPressed = false; // Add this flag
      function sendData(x) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/action?go=" + x, true);
        xhr.send();
      }

      function releaseData() {
        isButtonPressed = false; // A button has been released
        sendData('stop');
      }
      const keyMap = {
        'ArrowUp': 'forward',
        'ArrowLeft': 'left',
        'ArrowDown': 'backward',
        'ArrowRight': 'right',
        'KeyW': 'forward',
        'KeyA': 'left',
        'KeyS': 'backward',
        'KeyD': 'right',
      };
      document.addEventListener('keydown', function(event) {
        if (!isButtonPressed) { // Only send data if no button is being pressed
          const action = keyMap[event.code];
          if (action) sendData(action);
          isButtonPressed = true; // A button has been pressed
        }
      });
      document.addEventListener('keyup', function(event) {
        releaseData();
      });
      const toggleSwitch = document.getElementById("toggle-switch");
      toggleSwitch.addEventListener("change", function() {
        if (toggleSwitch.checked) {
          sendData('high'); // Send "high" when checked
        } else {
          sendData('low'); // Send "low" when unchecked
        }
      });
    </script>
  </body>
</html>
)rawliteral";


//====================================================
//===                  Server                      ===
//====================================================

// Function to start the server
httpd_handle_t Robot_httpd = NULL;
void startServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri = "/action",
    .method = HTTP_GET,
    .handler = cmd_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&Robot_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(Robot_httpd, &index_uri);
    httpd_register_uri_handler(Robot_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
}


//====================================================
//===                Handlers                      ===
//====================================================

// HTTP handler for serving the web page
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// HTTP handler for processing robot movement commands
int driveSpeed = lowSpeed;  // default speed
String speed = "low";

static esp_err_t cmd_handler(httpd_req_t *req) {
  char *buf;
  size_t buf_len;
  char variable[32] = {
    0,
  };

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  int res = 0;

  if (!strcmp(variable, "high")) {
    Serial.println("High");
    driveSpeed = highSpeed;
    speed = "High";
  } else if (!strcmp(variable, "low")) {
    Serial.println("Low");
    driveSpeed = lowSpeed;
    speed = "Low";
  } else if (!strcmp(variable, "forward")) {
    Serial.println("Forward " + speed);
    NeoPixel_SetColour(GREEN);
    Motor_Control(driveSpeed, driveSpeed);  // send 90% power to drive base
  } else if (!strcmp(variable, "left")) {
    Serial.println("Left " + speed);
    NeoPixel_SetColour(PURPLE);
    Motor_Control(-driveSpeed, driveSpeed);  // send 90% power to drive base
  } else if (!strcmp(variable, "right")) {
    Serial.println("Right " + speed);
    NeoPixel_SetColour(CYAN);
    Motor_Control(driveSpeed, -driveSpeed);  // send 90% power to drive base
  } else if (!strcmp(variable, "backward")) {
    Serial.println("Backward " + speed);
    NeoPixel_SetColour(BLUE);
    Motor_Control(-driveSpeed, -driveSpeed);  // send 90% power to drive base
  } else if (!strcmp(variable, "stop")) {
    Serial.println("Stop");
    NeoPixel_SetColour(YELLOW);
    Motor_STOP();
  } else if (!strcmp(variable, "functionY")) {
    Serial.println("Function Y");
  } else if (!strcmp(variable, "functionX")) {
    Serial.println("FUnction X");
  } else if (!strcmp(variable, "functionB")) {
    Serial.println("Function B");
  } else if (!strcmp(variable, "functionA")) {
    Serial.println("Function A");
  } else {
    Serial.println("Stop");
    NeoPixel_SetColour(RED);
    Motor_STOP();
    res = -1;
  }

  if (res) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}


//====================================================
//===              Wifi Setup                      ===
//====================================================

/* Put IP Address details */
IPAddress local_ip(10, 0, 0, 1);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
void WifiSetup() {
  // Wi-Fi connection
  // Set up access point with SSID "MiniBot" + MAC address
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  // Set up mDNS responder
  if (!MDNS.begin("robot")) Serial.println("Error setting up MDNS responder!");
  MDNS.addService("http", "tcp", 80);
  Serial.println("WiFi start");
  delay(3000);
  Serial.println("MiniBot System Ready! Version = " + Version);
}

//====================================================
//===                 setup                        ===
//====================================================

// Set up pins, LED PWM functionalities and Serial and Serial2 communication
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

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
  Serial.setDebugOutput(false);

  WifiSetup();
  startServer();
}


void loop() {
}
