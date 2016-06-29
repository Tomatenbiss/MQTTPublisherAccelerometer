#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

// your network name also called SSID
char ssid[] = "IT-SEA";
//char ssid[] = "McDonald's";
// your network password
char password[] = "IHSITOPon2014";
// MQTTServer to use
char server[] = "iot.eclipse.org";

// Counter var
int counter_in1;
int counter_out1;
int counter_in2;
int counter_out2;

byte buffer[128];

// For blink
int ledState = LOW;         
long previousMillis = 0;
long previousMillis2 = 0;
int alert = 0;

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Got a message from: ");
  Serial.println(topic);

  // If length is larger than 128,
  // then make sure we do not write more then our buffer can handle
  if(length > 128) length = 128;
  
  int j;
  memcpy(buffer, payload, length);
  
  char str[128];
  memset(str, 0, sizeof(str));
  
  // Create Response String
  for(j = 0; j < length; j++)
  {
    int i = buffer[j];
    str[j] = (char)i;
  }
  
  if(strcmp(topic, "onpu/accelerometer") == 0)
  {
    Serial.println("Accelerometer:");
    Serial.println(str);
    if(atoi(str) > 100)
    {
      alert = 1;
      previousMillis2 = millis();
      client.publish("onpu/alert", "1");
    }
    else
      client.publish("onpu/alert", "0");
  }
  else
  if(strcmp(topic, "onpu/counter/1/in") == 0)
  {
    counter_in1 = atoi(str);
    Serial.println("Counter1_IN:");
    Serial.println(counter_in1);
  }
  else
  if(strcmp(topic, "onpu/counter/1/out") == 0)
  {
    counter_out1 = atoi(str);
    Serial.println("Counter1_OUT:");
    Serial.println(counter_out1);
  }
  else
  if(strcmp(topic, "onpu/counter/2/in") == 0)
  {
    counter_in2 = atoi(str);
    Serial.println("Counter2_IN:");
    Serial.println(counter_in2);
  }
  else
  if(strcmp(topic, "onpu/counter/2/out") == 0)
  {
    counter_out2 = atoi(str);
    Serial.println("Counter2_OUT:");
    Serial.println(counter_out2);
  }
  
  int dcounter = counter_in1 + counter_in2 - counter_out1 - counter_out2;
  Serial.println(dcounter);
  
  if(dcounter <= 50 && dcounter >= 0)
  {
    client.publish("onpu/counter", "1");
    led(1);
  }
  else
  if(dcounter > 50 && dcounter <= 70)
  {
    client.publish("onpu/counter", "2");
    led(2);
  }
  else
  if(dcounter > 70)
  {
    client.publish("onpu/counter", "3");
    led(3);
  }
}

void setup()
{
  Serial.begin(115200);
  
  pinMode(RED_LED, OUTPUT);      // set the LED pin mode
  pinMode(YELLOW_LED, OUTPUT);      // set the LED pin mode
  pinMode(GREEN_LED, OUTPUT);      // set the LED pin mode
  
  led(1);
  
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  // Print the WiFi status.
  printWifiStatus();
}

void loop()
{
  // Reconnect if the connection was lost
  if (!client.connected()) {
    Serial.println("Disconnected. Reconnecting....");

    if(!client.connect("onpuClient")) {
      Serial.println("Connection failed");
    } else {
      Serial.println("Connection success");
      if(client.subscribe("onpu/accelerometer")) {
        Serial.println("Accelerometer subscription successfull");
      }
      if(client.subscribe("onpu/counter/1/in")) {
        Serial.println("Counter1_IN subscription successfull");
      }
      if(client.subscribe("onpu/counter/1/out")) {
        Serial.println("Counter1_OUT subscription successfull");
      }
      if(client.subscribe("onpu/counter/2/in")) {
        Serial.println("Counter2_IN subscription successfull");
      }
      if(client.subscribe("onpu/counter/2/out")) {
        Serial.println("Counter2_OUT subscription successfull");
      }
    }
  }
  
  // Check if any message were received
  // on the topic we subsrcived to
  client.poll();
  
  // ACCELEROMETER ALERT
  if(alert == 1)
  {
    unsigned long currentMillis = millis();
  
    if(currentMillis - previousMillis > 500)
    {
      if (ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;
  
      digitalWrite(RED_LED, ledState);
      previousMillis = currentMillis;  
    }
    
    if(currentMillis - previousMillis2 > 30000)
    {  
      alert = 0;
      digitalWrite(RED_LED, LOW);
    }
  }  
  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void led(int n) 
{  
  switch(n)
  {
    case 1:
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, LOW);
    break;
    
    case 2:
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(RED_LED, LOW);
    break;
    
    case 3:
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
    break;
  }
}
