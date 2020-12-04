#include "DHTesp.h"
#include "EasyNextionLibrary.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"

DHTesp dht;
EasyNex nex(Serial);

const String uri = String("http://api.openweathermap.org/data/2.5/weather")
                    + "?zip=" + String(ZIP) + "," + COUNTRY
                    + "&units=metric"
                    + "&appid=" + KEY;

// slow_update time delay in ms
const unsigned long slow_delay = 5000;
unsigned long time_prev = 0;

DynamicJsonDocument http_get(String uri) {
  HTTPClient http;

  http.useHTTP10(true);
  http.begin(uri);
  http.GET();

  DynamicJsonDocument doc(2048);
  deserializeJson(doc, http.getStream());

  http.end();
  return doc;
}

void setup() {
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  nex.begin(9600);
  dht.setup(D5, DHTesp::DHT22);
}

int weather_code(String code) {
  if     (code == "01d") return 0;
  else if(code == "01n") return 1;
  else if(code == "02d") return 2;
  else if(code == "02n") return 3;
  else if(code == "03d") return 4;
  else if(code == "04d") return 5;
  else if(code == "09d") return 6;
  else if(code == "10d") return 7;
  else if(code == "10n") return 8;
  else if(code == "11d") return 9;
  else if(code == "13d") return 10;
  else if(code == "50d") return 11;
  else return 0;
}

void slow_loop() {
  float h = dht.getHumidity();
  float t = dht.getTemperature();

  nex.writeStr("in_humid.txt", String(h) + "%");
  nex.writeStr("in_temp.txt", String(t) + "°C");

  if (WiFi.status() == WL_CONNECTED) {
    DynamicJsonDocument res = http_get(uri);
    String t = res["main"]["temp"];
    String h = res["main"]["humidity"];
    float  w = res["wind"]["speed"];
    String d = res["weather"][0]["description"];
    int    i = weather_code(res["weather"][0]["icon"]);
    nex.writeStr("out_temp.txt", t + "°C");
    nex.writeStr("out_humid.txt", h + "%");
    nex.writeStr("out_wind.txt", String(w) + "km/h");
    nex.writeStr("weather_desc.txt", d);
    nex.writeNum("weather_ico.pic", i);
  }
}

void loop() {
  nex.NextionListen();
  if (millis() - time_prev >= slow_delay) {
    slow_loop();
    time_prev = millis();
  }
}
