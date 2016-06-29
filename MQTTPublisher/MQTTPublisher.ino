#include <WiFi.h>
#include <PubSubClient.h>
#include "OneMsTaskTimer.h"
#include <Wire.h>
#include <BMA222.h>


#define topicButtonPresses "onpu/buttonpresses"
#define topicAccelerometer "onpu/accelerometer"
#define topicSubCounter "onpu/#"


BMA222 accelerometer;
char deviceID[] = "000000000000";
char buf[100];
char ssid[] = "IT-SEA";
char password[] = "IHSITOPon2014";
char server[] = "198.41.30.241"; //iot.eclipse.org
//char server[] = "141.82.100.21";
int previousMillis = 0;
int previousMillisAccData = 0;
int previousMillisSub = 0;
int pauseSub = 5000;
int pause = 50; // Between the loops
int pausePublish = 5000;
uint32_t push1cnt = 0;
uint8_t flag1 = 0;
double accDataResult = 0;
WiFiClient wiFiClient;
PubSubClient client(server, 1883, callback, wiFiClient);

uint8_t subscribedCounter = 0;



void setup()
{
  Serial.begin(115200);
  accelerometer.begin();
  uint8_t accChipID = accelerometer.chipID();
  Serial.print("Accelerometer ChipID: ");
  Serial.println(accChipID);
  printConnectionAttempt();
  MACAddress mac;
  uint8_t macOctets[6];
  mac = WiFi.macAddress(macOctets);
  Serial.print("MAC: ");
  Serial.println(mac);
  sprintf(deviceID, "%02x%02x%02x%02x%02x%02x", macOctets[0], macOctets[1],
                                                macOctets[2], macOctets[3], 
                                                macOctets[4], macOctets[5]);
  Serial.print("deviceID: ");
  Serial.println(deviceID);
  pinMode(PUSH1, INPUT);

}

void printConnectionAttempt() {
  Serial.print("Attempting to connect to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.localIP() == INADDR_NONE) {
    Serial.print("."); 
  }
  printWifiStatus();  
}

void loop()
{
 static int currentMillis = 0;
 currentMillis = millis();
 if (currentMillis > previousMillis + pausePublish) {
   previousMillis = currentMillis;
   if (client.connected()) {
     // publish accelerometer
       Serial.print("Publishing accelerometer: ");
       sprintf(buf, "%.0f", accDataResult);
       accDataResult = 0;
       Serial.println(buf);
       if (client.publish(topicAccelerometer, buf)) 
         Serial.println("Publish accelerometer success!");
       else
         Serial.println("Publish accelerometer failed!");
   }
   else {
     Serial.println("Disconnected. Reconnecting..");
     if (!client.connect(deviceID))
       Serial.println("Connection failed!");
     else
       Serial.println("Connection success!");
   }
 } 
   static int currentMillisAccData = 0;
   currentMillisAccData = millis();
    if (currentMillisAccData > previousMillisAccData + pause) {
     previousMillisAccData = currentMillisAccData;
     double x = accelerometer.readXData();
     double y = accelerometer.readYData();
     double z = accelerometer.readZData();
     double tmp = sqrt(x * x + y * y + z * z);
     if (tmp > accDataResult)
       accDataResult = tmp;
   }
   
   static int currentMillisSub = 0;
   currentMillisSub = millis();
   if (currentMillisSub > previousMillisSub + pauseSub) {
     if (client.connected()) {
       if (!subscribedCounter) {
         if (client.subscribe(topicSubCounter)) {
//           Serial.println("Subscription succesful");
           subscribedCounter = 1;
         }
         else {
//           Serial.println("Subscription failed");
         }
       }
     }
     if (subscribedCounter) {
       //Serial.print("client polling... ");
       client.poll();
       //Serial.println("done!");
     }
   }
}

void action1() {
  uint8_t currentState = 0;
  static uint8_t previousState = 0;
  currentState = digitalRead(PUSH1);
  if (previousState != currentState) {
    previousState = currentState;
    if (currentState == 1) {
      push1cnt++;
      flag1 = 1;      
    }
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}  

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message for topic ");
  Serial.println(topic);
  Serial.println("Message: ");
  Serial.write(payload, length);
  Serial.println();
}
