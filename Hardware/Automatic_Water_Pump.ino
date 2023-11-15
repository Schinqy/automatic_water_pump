#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

const int wifiLed = 10;
const int pump = 9;
const byte triggerPin = 4;
const byte echoPin = 5;
 float lati, lon;
 const float h = 0.00;
const float ht = 21.87;
float perc;
const signed long lastTime = 0;
const unsigned long timerDelay = 1000; //post data with a delay of 1sec


const char* serverName = "http://protathings.000webhostapp.com/esp-outputs-action.php?action=outputs_state&board=1";
    String mO;
     

// Update interval time set to 5 seconds
const long interval = 5000;
unsigned long previousMillis = 0;


String outputsState;

const char* ssid = "WaterMgmnt";
const char* password = "passc0d6";

void setup() {
  
  Serial.begin(9600);
pinMode(triggerPin , OUTPUT);
pinMode(pump , OUTPUT);
pinMode(wifiLed , OUTPUT);

    Serial.println(F("IM ALIVE"));

          digitalWrite(pump, HIGH);
          WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(wifiLed, LOW);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
 
}

void loop() {
if(WiFi.status() == WL_CONNECTED)
{
 digitalWrite(wifiLed, HIGH); 
}
else
{
  digitalWrite(wifiLed, LOW); 
}

 unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval) {
    if(httpAutoStatus() == "0")
    {
          webCtrl();
         postData();
         // sendTGNotification();
    }
    else if(httpAutoStatus() == "1")
    {
      autoCtrl(); 
      postData();
    }
    else
    {
      //OFFLINE DO SOMETHING
      Serial.println(F("OFFLINE"));
     //autoCtrl();
    }
    
    
      // save the last HTTP GET Request
      previousMillis = currentMillis;
    
  
  }
    
}

void webCtrl()
{
  
      outputsState = httpGET();
      Serial.println(outputsState);
      JSONVar myObject = JSON.parse(outputsState);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println(F("Parsing input failed!"));
        return;
      }
    
      // myObject.keys() can be used to get an array of all the keys in the object
      JSONVar keys = myObject.keys();
    
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = myObject[keys[i]];
        Serial.print(F("GPIO: "));
        Serial.print(keys[i]);
        Serial.print(F(" - SET to: "));
        Serial.println(value);
        pinMode(atoi(keys[i]), OUTPUT);
        digitalWrite(atoi(keys[i]), atoi(value));
      }
}



String httpGET() {
  if(WiFi.status() == WL_CONNECTED) {
  
  String payload = "{}";
  HTTPClient http;

  // Send GET request to the specified URL
  http.begin("http://protathings.000webhostapp.com/esp-outputs-action.php?action=outputs_state&board=1");

  // Start the request
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.println("Response:");
      Serial.println(payload);
    }
  } else {
    Serial.println("Error in HTTP request");
  }
  http.end();
  return payload;
  
  }
   else {
      Serial.println("WiFi Disconnected");
      digitalWrite(wifiLed, LOW);
    }
}


String httpAutoStatus() {
  if (WiFi.status() == WL_CONNECTED) {

  
  String payload = "{}";
  HTTPClient http;

  // Send GET request to the specified URL
  http.begin("http://protathings.000webhostapp.com/200/auto_hw.php");

  // Start the request
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.print("Auto Status: ");
      Serial.println(payload);
    }
  } else {
    Serial.print("Error in HTTP request. Error code: ");
    Serial.println(httpCode);
  }

  http.end();
  return payload;
}
 else {
      Serial.println("WiFi Disconnected");
      digitalWrite(wifiLed, LOW);
    }

}






float getVolume()
{


float v,V = 0.00;
float h = 0.00;
float ht = 150.00;
  
  //*******************************CALCULATE DISTANCE*******************************//
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin,HIGH);
  delayMicroseconds(10);
  float duration=pulseIn(echoPin, HIGH); 
  float d=duration/58.2;

 Serial.println(d);
  delay(100);


  Serial.print("volume:");

  Serial.print("\n");
  h = ht - d;  // water height, 0 - 150 cm
  
  v = (500 * h)/15;  
  V= 2000-v;// volume for 2000L tank with 150cm height 
  Serial.print(V);
  Serial.println(" L");

  return V;
}


void postData()
{

    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);


      
//      If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/urlencoded");
 float volume = getVolume();
     //String f = api_key=tPmAT5Ab3j7F9&temp=30&humidity=25&vol=30&lat=-17&lng=35;
   String f = String("api_key=tPmAT5Ab3j7F9") + "&vol=" + String(volume, 2)+ "";

int httpResponseCode = http.POST(f);

     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        String payload = "{}"; 
        payload = http.getString();
         Serial.println(payload);    
      http.end();

    }
    else {
      Serial.println("WiFi Disconnected");
      digitalWrite(wifiLed, LOW);
    }

}













void autoCtrl()
{
   float volume = getVolume();
  
   if(volume >= 1900) 
   {
    digitalWrite(pump, HIGH); // Switch off the pump
   }
   else if(volume <= 400)
   {
    digitalWrite(pump, LOW); // Switch on the pump
   } 
}
