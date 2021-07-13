// http://arduino.esp8266.com/stable/package_esp8266com_index.json

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include "Adafruit_PWMServoDriver.h"

#define MIN_PULSE_WIDTH 600
#define MAX_PULSE_WIDTH 2600
#define FREQUENCY 50

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
AsyncWebServer server(80);

const char* ssid = "vivo";
const char* password = "tabithayapyap";

const char* PARAM_SERVO = "servo";
const char* PARAM_ANGLE = "angle";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    body {
      text-align: center;
      font-family: "Trebuchet MS", Arial;
      margin-left:auto;
      margin-right:auto;
    }
    .slider {
      width: 300px;
    }
  </style>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
</head>
<body>
  <h1>ESP32 with Servo</h1>
  <p>Position: <span id="servoPos"></span></p>
  Gripper:<input type="range" min="0" max="60" class="slider" id="1" onchange="updateSliderPWM(this)"/><br>
  W.Pitch:<input type="range" min="0" max="180" class="slider" id="2" onchange="updateSliderPWM(this)"/><br>
  W.Roll:<input type="range" min="0" max="180" class="slider" id="3" onchange="updateSliderPWM(this)"/><br>
  Elbow:<input type="range" min="0" max="180" class="slider" id="4" onchange="updateSliderPWM(this)"/><br>
  Shoulder:<input type="range" min="0" max="90" class="slider" id="5" onchange="updateSliderPWM(this)"/><br>
  Waist:<input type="range" min="0" max="180" class="slider" id="6" onchange="updateSliderPWM(this)"/><br>
  <script>
    function updateSliderPWM(element) {
      var servonumber = element.id
  var sliderValue = document.getElementById(servonumber).value;
  console.log("servo "+servonumber+" set at "+sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/set?servo="+servonumber+"&angle="+sliderValue, true);
  xhr.send();
}
  </script>
</body>
</html>
)rawliteral";

int pulseWidth(int angle) {
  int pulse_wide, analog_value;
  pulse_wide = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  analog_value = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  return analog_value;
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
    pwm.begin();
    pwm.setPWMFreq(FREQUENCY);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String servo;
        String angle;
        int servo_int;
        int angle_int;
        if (request->hasParam(PARAM_SERVO)) 
        {
          if (request->hasParam(PARAM_ANGLE)) 
          {
            servo = request->getParam(PARAM_SERVO)->value();
            angle = request->getParam(PARAM_ANGLE)->value();
            Serial.println("servo "+servo+" set at "+angle);
            servo_int = servo.toInt();
            angle_int = angle.toInt();
            pwm.setPWM(servo_int, 0, pulseWidth(angle_int));
            String message = "Set servo "+servo+" to "+angle+" degrees.";
            request->send(200, "text/plain", message);
          }
        }
        
        /**
         
        if (request->hasParam(PARAM_SERVO)) 
        {
          if (request->hasParam(PARAM_ANGLE)) 
          {
            servo = request->getParam(PARAM_SERVO)->value();
            angle = request->getParam(PARAM_ANGLE)->value();
            pwm.setPWM(servo, 0, pulseWidth(angle));
            String message = "Set servo "+servo+" to "+angle+" degrees.";
            request->send(200, "text/plain", message);
          }
          else 
          {
            request->send(200, "text/plain", "Missing angle.");
          }
        } 
        else 
        {
          request->send(200, "text/plain", "Missing servo.");
        }
        *
         */
    });

    server.onNotFound(notFound);

    server.begin();
}

void loop() {
}
