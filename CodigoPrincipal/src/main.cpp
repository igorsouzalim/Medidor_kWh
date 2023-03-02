/**
 *  ---Funcionalidaes para adicionar---
 * 
 * - Tempo de scan do OTA
 * - botão para zerar o KW
 * - 
 */

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

void setup_wifi();

void setup() {
  /* Initialize the serial port to host */
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Start ATM90E36");
  /*Initialise the ATM90E36 + SPI port */

  pinMode(CF1, INPUT);
  pinMode(CF2, INPUT);
  pinMode(CF3, INPUT);
  pinMode(CF4, INPUT);

  pinMode(DMA_CTRL, OUTPUT);
  pinMode(PM0, OUTPUT);
  pinMode(PM1, OUTPUT);
  
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

   //Inicia EEPROM
  preferences.begin("Storage", false); 

  kW_total = preferences.getFloat("kW_total", 0); 
  //ConsumoMensal = preferences.getFloat("ConsumoMensal", 0); 
  //ssid = preferences.getString("SSID", ""); 
  //password = preferences.getString("Password", "");

  // if(debugStartFlashMemory)
  // {
  //   Serial.println();
  //   Serial.println(">>> [STARTS WITH STORED IN EEPROM]");
  //   // Serial.print("ID: "); Serial.println(ID); 
  //   Serial.print("kW_total: "); Serial.println(kW_total);  
  //   // Serial.print("WIFI_SSID: "); Serial.println(ssid); 
  //   // Serial.print("WIFI_PASSWORD: "); Serial.println(password); 
  //   // Serial.println("____________________________");

  //   if (volumeAguaMensal == 0 )
  //     Serial.println("No values saved for volumeAguaMensal");
  //   if (ssid == "" || password == "")
  //   Serial.println("No values saved for ssid or password");
  // }

  digitalWrite(DMA_CTRL, LOW);
  digitalWrite(PM0, HIGH);
  digitalWrite(PM1, HIGH);

  digitalWrite(LED1,1);
  digitalWrite(LED2,1);
  digitalWrite(LED3,1);

  setup_wifi();

  eic.begin();
  conectar_mqtt();

  delay(1000);

  OTADRIVE.setInfo("cc901072-5ee5-4003-bb41-372d1780b039", "v@1.2.2");

  //xTaskCreatePinnedToCore( vTaskLCD, "Task LCD", configMINIMAL_STACK_SIZE*2, NULL, 3, NULL, CORE_1 );
}


void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 50) {
    previousMillis = currentMillis;
     ota();
     digitalWrite(LED3,!digitalRead(LED3));
  }

  double voltageA,freq,voltageB,voltageC,currentA,currentB,currentC,totalActivePower,totalReactivePower,totalApparentPower,totalFactorPower,temp;
  
  currentMillis = millis();
if (currentMillis - previousMillis3 >= 1000) {
    previousMillis3 = currentMillis;

    totalActivePower = eic.GetTotalActivePower();
    kW_total += (float)totalActivePower/3600;

    if( kW_totalAnterior >= (float)(kW_total+(float)0.5))
    {
      kW_totalAnterior = kW_total;
      //preferences.putFloat("kW_total", kW_total); 
    }
}
  
 currentMillis = millis();
if (currentMillis - previousMillis2 >= 3000) {
    previousMillis2 = currentMillis;
    
  voltageA=eic.GetLineVoltageA();
  voltageB=eic.GetLineVoltageB();
  voltageC=eic.GetLineVoltageC();
  currentA = eic.GetLineCurrentA();
  currentB = eic.GetLineCurrentB();
  currentC = eic.GetLineCurrentC();
  totalActivePower = eic.GetTotalActivePower();
  totalReactivePower = eic.GetTotalReactivePower();
  totalApparentPower = eic.GetTotalApparentPower();
  totalFactorPower = eic.GetTotalPowerFactor();
  freq=eic.GetFrequency();
  temp=eic.GetTemperature();

  client.loop();

  if(!client.connected()){
     //Serial.println("status wifi...");
     //Serial.println(WiFi.status());
     delay(500);
     //Serial.println(WiFi.status());
      if (WiFi.status() == 7)
      {
      //Serial.println("***************************************");home
      //Serial.println("conexao perdida...");
     //online = 1;
      }
  conectar_mqtt();
}

char tempString[15];

dtostrf(voltageA, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/va", tempString);

dtostrf(currentA, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/ia", tempString);

dtostrf(totalActivePower, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/p", tempString);

// dtostrf(temp, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/tempATM90", tempString);

dtostrf((kW_total*1000), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/total", tempString);

dtostrf((kW_total), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/kwTotal", tempString);

Serial.println("send mqtt...");

digitalWrite(LED2,!digitalRead(LED2));

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
      //Serial.println("connected");
      
      // Lista de topicos para Subscrever abaixo:
      //client.subscribe("RST_MQTT");
      //client.subscribe("AUTO_MQTT");
      
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

  
  if (String(topic) == "RST_MQTT") {  // Verifica a mensagem recebida no topico RST_MQTT
    Serial.print("Changing output to ");
    
    if(messageTemp == "1"){
      //Serial.println("on");               
      
      //RL1 = 1;
      
    }
  }
  
 }
