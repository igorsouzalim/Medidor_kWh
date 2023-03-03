#include <WiFi.h>
#include <SPI.h>
#include <ATM90E36.h>
#include <otadrive_esp.h>
#include <PubSubClient.h>   
#include <Preferences.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//DEFINES SPI
#define VSPI_MISO   MISO
#define VSPI_MOSI   MOSI
#define VSPI_SCLK   SCK
#define VSPI_SS     SS

#define CF1 13
#define CF2 22
#define CF3 14
#define CF4 27
#define DMA_CTRL 21
#define PM0 32
#define PM1 15

#define LED1 2
#define LED2 25
#define LED3 33

//FreeRTOS
#define CORE_0 0 
#define CORE_1 1

const char* mqttServer = "postman.cloudmqtt.com";
const int mqttPort = 14923;
const char* mqttUser = "zkytiriu";
const char* mqttPassword = "WcfXgbW1cWDs";


WiFiClient Inst2;
PubSubClient client(Inst2);
Preferences preferences;
ATM90E36 eic(5);


const char* ssid = "Andre";
const char* password = "Julia220816";

unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long previousMillis3 = 0; 

float kW_total,kW_totalAnterior;
uint8_t erase = 0; //apaga dados da memoria flash   1=kwTotal,2=kwMensal,3=kwDiario,4=Parcial


void publicar();
void conectar_wifi();
void conectar_mqtt();
void callback(char* topic, byte* message, unsigned int length);


void ota()
{
  if(OTADRIVE.timeTick(180))  // Limite de verificações de atualização no servidor 500 vezes (172ms valor minimo)
  {
    OTADRIVE.updateFirmware();
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED2,!digitalRead(LED2));
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void conectar_mqtt()
{
   //offline = 1;
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  while (!client.connected()) 
  {
    
    //Serial.println("Connecting to MQTT...");
    if (client.connect("Inst2", mqttUser, mqttPassword )) 
    {
      Serial.println("connected");
      
      // Lista de topicos para Subscrever abaixo:
      client.subscribe("home/powerMeter/erase/kwTotal");
      client.subscribe("home/powerMeter/erase/monthlyConsumption");
      client.subscribe("home/powerMeter/erase/dailyConsumption");
      
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      //conectar_wifi(); //se nao conseguiu reconectar Mqtt verifica e reconecta WiFi
      WiFi.begin(ssid, password);
      Serial.println("");

      // Wait for connection
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }

      
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) // Função que recebe a mensagem da subscrição
 {  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  
  if (String(topic) == "home/powerMeter/erase/kwTotal") {  // Verifica a mensagem recebida no topico RST_MQTT
    if(messageTemp == "1"){
      erase=1;
    }
  }
  //   if (String(topic) == "home/powerMeter/erase/monthlyConsumption") {  // Verifica a mensagem recebida no topico RST_MQTT
  //   if(messageTemp == "1"){
  //     preferences.putFloat("kW_total", 0);
  //   }
  // }
  //   if (String(topic) == "home/powerMeter/erase/dailyConsumption") {  // Verifica a mensagem recebida no topico RST_MQTT
  //   if(messageTemp == "1"){
  //     preferences.putFloat("kW_total", 0);
  //   }
  // }
  
 }
