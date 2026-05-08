Complete Program Code
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define RELAY_PIN 26

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char* ssid = "AI_IRRIGATION";
const char* password = "12345678";

String apiKey = "YOUR_API_KEY";
String city = "Tirunelveli";
String countryCode = "IN";

float temperature;
float humidity;
int soilValue;
int soilPercent;
int rainPercent = 0;

String rainPrediction="Clear";
String pumpStatus="OFF";

String mode="AUTO";
bool manualPump=false;

void getWeather()
{
HTTPClient http;
String url="http://api.openweathermap.org/data/2.5/weather?q="+city+","+countryCode+"&appid="+apiKey+"&units=metric";

http.begin(url);
int httpCode=http.GET();

if(httpCode>0)
{
String payload=http.getString();
DynamicJsonDocument doc(2048);
deserializeJson(doc,payload);

rainPrediction=doc["weather"][0]["main"].as<String>();

if(rainPrediction=="Rain") rainPercent=90;
else if(rainPrediction=="Clouds") rainPercent=50;
else rainPercent=10;
}
http.end();
}

void setAuto(){ mode="AUTO"; server.send(200,"text/plain","OK"); }
void setManual(){ mode="MANUAL"; server.send(200,"text/plain","OK"); }
void pumpOn(){ manualPump=true; server.send(200,"text/plain","OK"); }
void pumpOff(){ manualPump=false; server.send(200,"text/plain","OK"); }

void handleRoot()
{
String page = "<html><body><h2>Smart Irrigation System</h2>";
page += "<p>Temperature: " + String(temperature) + " C</p>";
page += "<p>Humidity: " + String(humidity) + " %</p>";
page += "<p>Soil: " + String(soilPercent) + " %</p>";
page += "<p>Rain: " + String(rainPercent) + " %</p>";
page += "<p>Mode: " + mode + "</p>";
page += "<p>Pump: " + pumpStatus + "</p>";
page += "</body></html>";
server.send(200,"text/html",page);
}

void setup()
{
Serial.begin(115200);

pinMode(RELAY_PIN,OUTPUT);
digitalWrite(RELAY_PIN,HIGH);

dht.begin();

WiFi.softAP(ssid,password);

server.on("/",handleRoot);
server.on("/auto",setAuto);
server.on("/manual",setManual);
server.on("/on",pumpOn);
server.on("/off",pumpOff);

server.begin();
}

void loop()
{
temperature=dht.readTemperature();
humidity=dht.readHumidity();

soilValue=analogRead(SOIL_PIN);
soilPercent=map(soilValue,4095,0,0,100);

static unsigned long lastWeather=0;
if(millis()-lastWeather>10000){
getWeather();
lastWeather=millis();
}

if(mode=="AUTO")
{
if(soilValue > 2500 && rainPrediction != "Rain")
{
digitalWrite(RELAY_PIN,LOW);
pumpStatus="ON";
}
else
{
digitalWrite(RELAY_PIN,HIGH);
pumpStatus="OFF";
}
}
else
{
if(manualPump)
{
digitalWrite(RELAY_PIN,LOW);
pumpStatus="ON";
}
else
{
digitalWrite(RELAY_PIN,HIGH);
pumpStatus="OFF";
}
}

server.handleClient();
}
