#include <TinyGPS++.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#define EARTH_RADIUS 6371000

const String SETUP_ERROR = "!!ERROR!! SETUP: Unable to start SoftAP mode";
const String HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";
String HTML_WELCOME = "<p><a href=\"refresh\" style=\"display: inline-block; width: 200px; height: 200px; background-color: #4CAF50; color: white; text-align: center; line-height: 200px; text-decoration: none; font-size: 18px; font-weight: bold; border-radius: 10px; border: 2px solid #2E7D32;\">Refresh</a></p>";

const String refreshMessage = "GET /refresh HTTP/1.1";

const String refreshButton = "<p><a href=\"refresh\" style=\"display: inline-block; width: 200px; height: 200px; background-color: #4CAF50; color: white; text-align: center; line-height: 200px; text-decoration: none; font-size: 18px; font-weight: bold; border-radius: 10px; border: 2px solid #2E7D32;\">Refresh</a></p>";

const char *SSID = "ESP32BOSS";
const char *PASS = "12345678";

float accelerationTime = 0;
double brakingDistance = 0;

bool isTiming = false;
unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long elapsedTime = 0;

double startLat = 0.0, startLon = 0.0;
double endLat = 0.0, endLon = 0.0;
bool isBraking = false;

WiFiServer HttpServer(80);

TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  if (!WiFi.softAP(SSID, PASS)) {
    Serial.println(SETUP_ERROR);
    while (1)
      ;
  }
  HttpServer.begin();
}

void loop() {


  while (Serial2.available()) {
    gps.encode(Serial2.read());
  }
  if (gps.location.isUpdated()) {
    float currentSpeed = gps.speed.kmph();
    double currentLat = gps.location.lat(), currentLon = gps.location.lng();
    Serial.print("SPEED: ");
    Serial.println(currentSpeed);

    //calcul 0-50
    if (!isTiming && currentSpeed > 5.0) {
      isTiming = true;
      startTime = millis();
    }

    if(isTiming && currentSpeed < 5.0){
      isTiming = false;
    }

    if (isTiming && currentSpeed > 50.0) {
      isTiming = false;
      endTime = millis();
      elapsedTime = endTime - startTime;
      if((elapsedTime / 1000.0) > 1){
        accelerationTime = elapsedTime / 1000.0;
      }
    }

    Serial.print("TIME: ");
    Serial.println(accelerationTime);

    //calcul franare
    if (!isBraking && currentSpeed < 50.0) {
      isBraking = true;
      startLat = currentLat;
      startLon = currentLon;
    }

    if (isBraking && currentSpeed > 51.0){
      isBraking = false;
    }

    if (isBraking && currentSpeed < 5.0) {
      endLat = currentLat;
      endLon = currentLon;
      isBraking = false;
      if (haversineDistance(startLat, startLon, endLat, endLon) > 1){
        brakingDistance = haversineDistance(startLat, startLon, endLat, endLon);
      }
    }

    Serial.print("DIST: ");
    Serial.println(brakingDistance);
  }

  WiFiClient client = HttpServer.available();  
  if (client) {                               
    String currentLine = "";           
    while (client.connected()) {       
      if (client.available()) {        
        const char c = client.read();  
        Serial.write(c);               
        if (c == '\n') {               

          if (currentLine == refreshMessage) {
            HTML_WELCOME = refreshButton;
            HTML_WELCOME += "<p><h1>0-50 kmh: ";
            HTML_WELCOME += String(accelerationTime);
            HTML_WELCOME += " s</h1></p>";

            HTML_WELCOME += "<p><h1>Breaking distance: ";
            HTML_WELCOME += String(brakingDistance);
            HTML_WELCOME += " m</h1></p>";
          }

          if (currentLine.length() == 0) {
            printWelcomePage(client);
            break;
          } else currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println();
  }
}

void printWelcomePage(WiFiClient client) {
  client.println(HTTP_HEADER);
  client.print(z);
  client.println();
}

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
  lat1 = radians(lat1);
  lon1 = radians(lon1);
  lat2 = radians(lat2);
  lon2 = radians(lon2);

  double dLat = lat2 - lat1;
  double dLon = lon2 - lon1;

  double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return EARTH_RADIUS * c;
}