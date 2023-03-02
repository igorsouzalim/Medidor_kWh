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

void publicar();
void conectar_wifi();
void conectar_mqtt();
void callback(char* topic, byte* message, unsigned int length);

WiFiClient Inst2;
PubSubClient client(Inst2);

// SSID and password

const char* ssid = "Andre";
const char* password = "Julia220816";

unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long previousMillis3 = 0; 

float total = 0;

ATM90E36 eic(5);


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

  digitalWrite(DMA_CTRL, LOW);
  digitalWrite(PM0, HIGH);
  digitalWrite(PM1, HIGH);

  digitalWrite(LED1,1);
  digitalWrite(LED2,1);
  digitalWrite(LED3,1);

  setup_wifi();

  eic.begin();
  conectar_mqtt();
  delay(500);

  delay(1000);

  OTADRIVE.setInfo("cc901072-5ee5-4003-bb41-372d1780b039", "v@1.2.1");
}


void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 50) {
    previousMillis = currentMillis;
     ota();
     digitalWrite(LED3,!digitalRead(LED3));
  }


  //digitalWrite(LED1,!digitalRead(LED1));
  //digitalWrite(LED2,!digitalRead(LED2));
  

 
  
  /*Repeatedly fetch some values from the ATM90E36 */
  double voltageA,freq,voltageB,voltageC,currentA,currentB,currentC,totalActivePower,totalReactivePower,totalApparentPower,totalFactorPower,temp;
  // int sys0=eic.GetSysStatus0();
  // int sys1=eic.GetSysStatus1();
  // int en0=eic.GetMeterStatus0();
  // int en1=eic.GetMeterStatus1();

  
  // Serial.println("S0:0x"+String(sys0,HEX));
  // delay(10);
  // Serial.println("S1:0x"+String(sys1,HEX));
  // delay(10);
  // Serial.println("E0:0x"+String(en0,HEX));
  // delay(10);
  // Serial.println("E1:0x"+String(en1,HEX));


  // voltageA=eic.GetLineVoltageA();
  // Serial.println("VA:"+String(voltageA)+"V");
  // voltageB=eic.GetLineVoltageB();
  // Serial.println("VB:"+String(voltageB)+"V");
  // voltageC=eic.GetLineVoltageC();
  // Serial.println("VC:"+String(voltageC)+"V");
  // delay(10);
  // currentA = eic.GetLineCurrentA();
  // Serial.println("IA:"+String(currentA)+"A");
  // currentB = eic.GetLineCurrentB();
  // Serial.println("IB:"+String(currentB)+"A");
  // currentC = eic.GetLineCurrentC();
  // Serial.println("IC:"+String(currentC)+"A");
  // delay(10);
  // freq=eic.GetFrequency();
  // delay(10);
  // Serial.println("f"+String(freq)+"Hz");
  // delay(10);
  // temp=eic.GetTemperature();
  // delay(10);
  // Serial.println("Temperatura:"+String(temp)+"C");
  // delay(10);
  // totalActivePower = eic.GetTotalActivePower();
  // Serial.println("potencia ativa:"+String(totalActivePower)+"W");
  // delay(1000);



  currentMillis = millis();
if (currentMillis - previousMillis3 >= 1000) {
    previousMillis3 = currentMillis;


    totalActivePower = eic.GetTotalActivePower();
    total += (float)totalActivePower/3600;

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

// dtostrf(voltageB, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/vb", tempString);

// dtostrf(voltageC, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/vc", tempString);

dtostrf(currentA, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/ia", tempString);

// dtostrf(currentB, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/ib", tempString);

// dtostrf(currentC, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/ic", tempString);

dtostrf(totalActivePower, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/p", tempString);

// dtostrf(temp, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
// client.publish("home/tempATM90", tempString);

dtostrf(codeVersion, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/versao", tempString);


dtostrf((total*1000), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/total", tempString);

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
