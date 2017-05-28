#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

constexpr int bufferSize = 512;

class WebInterfaceThermometer {
protected:

  void loadCredentialsFromFs() {
    File f = SPIFFS.open("/passwd", "r");
    if (!f) {
        Serial.println("file open /passwd failed");
    } 
    ssid = f.readStringUntil('\n');
    password = f.readStringUntil('\n');
  }

  void setupWifi() {
    loadCredentialsFromFs();
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    if (mdns.begin("esp8266", WiFi.localIP())) {
      Serial.println("MDNS responder started");
    }
  }

  void sendDataCollection() {
      String webPage = "temp,date\n";
      int start = 0;
      if (samplesCount == bufferSize)
      {
        start = currentPointer;
      }
      for (int i = 0; i < samplesCount; i++) {
        int pos = (i + start) % bufferSize;
        webPage += String(temperatures[pos]) + ", " +  String(timeStamps[pos]) + "\n";
      }
      server.send(200, "text/html", webPage);
  }
  
  void setupWebServer() {
    // data source
    server.on("/data", [=]() {
      sendDataCollection();
    });
    // redirect to index.html
    server.on("/", [=]() {
      server.sendHeader("Location", "index.html");
      server.send(302, "text/plain", "");
    });
    // serve folder
    server.serveStatic("/", SPIFFS, "/");
    
    server.begin();
    Serial.println("HTTP server started");
  }

  float getTemp() {
    int reading = analogRead(sensorPin);  
    float voltage = reading * 3.3 / 1024; 
    float temperatureC = voltage * 100 ;  
    return temperatureC;
  }
  
  void collectTemperature() {
    unsigned long now = millis();
    if ((now - lastTime) > sampleRateMs) {
      temperatures[currentPointer] = getTemp();
      timeStamps[currentPointer] = now;
      currentPointer = (currentPointer + 1) % bufferSize;
      samplesCount = _min(samplesCount + 1, bufferSize); 
      lastTime = now;
    }
  }

  MDNSResponder mdns;
  ESP8266WebServer server;
  float temperatures[bufferSize];
  unsigned long timeStamps[bufferSize];
  unsigned long lastTime;
  int samplesCount;
  int currentPointer;
  int sensorPin;
  int sampleRateMs;
  String ssid;
  String password;

public:

  WebInterfaceThermometer() : 
    server(80), currentPointer(0), samplesCount(0), sensorPin(0), sampleRateMs(1000) {
      for (int i = 0; i < bufferSize; i++) {
        temperatures[i] = 0;
        timeStamps[i] = 0;
      }
  }

  void setup(void) {
    Serial.begin(115200); 
    SPIFFS.begin();
    setupWifi();
    setupWebServer();
  } 
  
  void loop(void) {
    server.handleClient();
    collectTemperature();
  } 
};

WebInterfaceThermometer webInterfaceThermometer;

void setup(void) {
  webInterfaceThermometer.setup();
}

void loop(void) {
  webInterfaceThermometer.loop();
} 

