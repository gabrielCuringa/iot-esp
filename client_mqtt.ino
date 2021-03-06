/*********
 Based on Rui Santos work :
 https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
 Modified by GM
 Modified by Benjamin Dekeyser & Gabriel Curinga
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "OneWire.h"
#include "DallasTemperature.h";

WiFiClient espClient;           // Wifi 
PubSubClient client(espClient); // MQTT client 

/*===== MQTT broker/server and TOPICS ========*/
const char* mqtt_server = "broker.hivemq.com"; /* "broker.shiftr.io"; */

#define TOPIC_TEMP "m1/miage/dc/temperature"

/*============= GPIO ======================*/
float temperature = 21.0;
float light = 0;
const int ledPin = 23; // LED Pin
const int temperaturePin = 21;
const int serial = 9600;

/** TEMPERATURE **/
OneWire oneWire(temperaturePin); 
DallasTemperature tempSensor(&oneWire);

/*================ WIFI =======================*/
void print_connection_status() {
  Serial.print("WiFi status : \n");
  Serial.print("\tIP address : ");
  Serial.println(WiFi.localIP());
  Serial.print("\tMAC address : ");
  Serial.println(WiFi.macAddress());
}

void connect_wifi() {

  /** CHOOSE YOUR OWN WIFI**/
  
  //const char* ssid = "HUAWEI-6EC2";
  //const char *password= "FGY9MLBL";

  const char* ssid = "Villalucie";
  const char *password= "villaluciejulie";
  
  Serial.println("Connecting Wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect Wifi ..");
    delay(1000);
  }
  Serial.print("Connected to local Wifi\n");
  print_connection_status();
}

/*=============== SETUP =====================*/
void setup() {

  Serial.begin(serial);
  
  tempSensor.begin(); // Init temperature sensor
  //pinMode(ledPin, OUTPUT);
  
  connect_wifi();
  
  client.setServer(mqtt_server, 1883);
  
  // set callback when publishes arrive for the subscribed topic
  //client.setCallback(mqtt_pubcallback); 
}

/*============== CALLBACK ===================*/
void mqtt_pubcallback(char* topic, byte* message, 
                      unsigned int length) {
  // Callback if a message is published on this topic.
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Byte list to String and print to Serial
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic,
  // you check if the message is either "on" or "off".
  // Changes the output state according to the message
  /*if (String(topic) == TOPIC_LED) {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      set_LED(HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      set_LED(LOW);
    }
  }*/
}

void mqtt_publish(char *topic, char *message){

  if(client.connect("esp32", "try", "try")){
    client.publish(topic, message);
    // Serial info
    Serial.print("Published datas : "); 
    Serial.println(message);
  }else{
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

void set_LED(int v){
  
}

/*============= SUBSCRIBE =====================*/
void mqtt_mysubscribe(char *topic) {
  while (!client.connected()) { // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32", "try", "try")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float get_Temperature(){

  float t;
  tempSensor.requestTemperaturesByIndex(0);
  t = tempSensor.getTempCByIndex(0);
  
  return t;
}

char* generate_json(char* sensor, float value){

  String message = "{\"sensor\"";
  //sensor type
  message += ":\"";
  message += sensor;
  message += "\",";

  //sensor value
  message += "\"value\"";
  message += ":\"";
  message += temperature;
  message += "\"}";

  char* copy = new char[message.length()+1];
  message.toCharArray(copy, message.length()+1);
  return copy;
}

/*================= LOOP ======================*/
void loop() {
  int32_t period = 5000; // 5 sec
  
  /*--- subscribe to TOPIC_TEMPERATURE if not yet ! */
  /*if (!client.connected()) { 
    mqtt_mysubscribe((char *)(TOPIC_TEMP));
  }*/

  /*--- Publish Temperature periodically   */
  delay(period);
  //temperature = get_Temperature();
  // Convert the value to a char array
  temperature = temperature + 1;
  /*char tempString[8];
  dtostrf(temperature, 1, 2, tempString);*/
  // MQTT Publish
  mqtt_publish(TOPIC_TEMP, generate_json("temperature", temperature));
   
  client.loop(); // Process MQTT ... une fois par loop()
}
