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

// Pin definitions and other constants
#define PART_BOUNDARY "123456789000000000000987654321"

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

// WiFi configuration of SSID
const char *ssid = "MiniBot-";

// Global variables for HTTP server instances
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

// Function to convert MAC address to string format. MAC address of the ESP32 in format "XX:XX:XX:XX:XX:XX"
String UniqueID() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);  // Get unique MAC address
  char buffer[13];                      // Save MAC address to string
  sprintf(buffer, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  String uniqueID = buffer;
  uniqueID = uniqueID.substring(6, 12);  // limit to last 6 digits
  return uniqueID;
}

String PasswordGen() {
  String uniqueID = UniqueID();
  String mixedString = "";
  for (int i = 0; i < 6; i++) {
    char uniqueChar = uniqueID.charAt(i);
    char camronChar = "CAMRON"[i];
    int mixedValue = (uniqueChar - '0') + (camronChar - 'A') + i;
    mixedValue %= 16;  // limit to single hex digit
    mixedString += (char)((mixedValue < 10) ? ('0' + mixedValue) : ('A' + mixedValue - 10));
  }
  mixedString = "LoR" + mixedString;
  return mixedString;
}
const String SystemPassword = String(PasswordGen());

// Web page (HTML, CSS, JavaScript) for controlling the robot
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <html>
  <head>
    <title>LORD of ROBOTS</title>
    <meta name="viewport" content="width=device-width, height=device-height, initial-scale=1" >
    <style>
      body { font-family: Arial; text-align: center; margin:0 auto; padding-top: 30px;}
      .button {
        background-color: #2f4468;
        width: 100px;
        height: 80px;
        border: none;
        color: white;
        font-size: 20px;
        font-weight: bold;
        text-align: center;
        text-decoration: none;
        border-radius: 10px;
        display: inline-block;
        margin: 6px 6px;
        cursor: pointer;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
        -webkit-user-select: none; /* Chrome, Safari, Opera */
        -moz-user-select: none; /* Firefox all */
        -ms-user-select: none; /* IE 10+ */
        user-select: none; /* Likely future */
      }
      
      #buttons { text-align: center; }
    </style>
  </head>
  <body style="background-color:black;" oncontextmenu="return false;">
    <h1 style="color:white">CAMRON MiniBot</h1>
    <div id="buttons">
      <button class="button" onpointerdown="sendData('forward')" onpointerup="releaseData()">Forward</button><br>
      <button class="button" onpointerdown="sendData('left')" onpointerup="releaseData()">Left</button>
      <button class="button" onpointerdown="sendData('stop')" onpointerup="releaseData()">Stop</button>
      <button class="button" onpointerdown="sendData('right')" onpointerup="releaseData()">Right</button><br>
      <button class="button" onpointerdown="sendData('ledon')" onpointerup="releaseData()">LED ON</button>
      <button class="button" onpointerdown="sendData('backward')" onpointerup="releaseData()">Backward</button>
      <button class="button" onpointerdown="sendData('ledoff')" onpointerup="releaseData()">LED OFF</button>
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
        'KeyL': 'ledon',
        'KeyO': 'ledoff'
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

      window.onload = function() {
        document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
      }
    </script>
  </body>
</html>
)rawliteral";

// Function to start the camera server
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

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;

}


// HTTP handler for serving the web page
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// HTTP handler for processing robot movement commands
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
  int LED_Max = 50;
  if (!strcmp(variable, "forward")) {
    Serial.println("Forward");
  } else if (!strcmp(variable, "left")) {
    Serial.println("Left");
  } else if (!strcmp(variable, "right")) {
    Serial.println("Right");
  } else if (!strcmp(variable, "backward")) {
    Serial.println("Backward");
  } else if (!strcmp(variable, "stop")) {
    Serial.println("Stop");
  }
  /* else if (!strcmp(variable, "ledon")) {
    Serial.println("xON");
    analogWrite(LED_OUTPUT, LED_Max);
  } else if (!strcmp(variable, "ledoff")) {
    Serial.println("xOFF");
    analogWrite(LED_OUTPUT, 0);
  } */
  else {
    Serial.println("Stop");
    res = -1;
  }
  //Serial.println("\n");

  if (res) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}


void WifiSetup() {
  // Wi-Fi connection
  // Set up access point with SSID "MiniBot" + MAC address
  WiFi.mode(WIFI_AP);

  WiFi.softAP("robot", "password");
  // Set up mDNS responder
  if (!MDNS.begin("robot")) Serial.println("Error setting up MDNS responder!");
  MDNS.addService("http", "tcp", 80);
  Serial.println("WiFi start");
  delay(3000);
  Serial.println("MiniBot System Ready! Version = " + Version);
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
const int DEAD_BAND = 20;
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
int Motor_FrontLeft_SetValue, Motor_FrontRight_SetValue, Motor_BackLeft_SetValue, Motor_BackRight_SetValue = 0;
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

