
#include <SPI.h>
#include <ATM90E36.h>
#include <otadrive_esp.h>
#include <PubSubClient.h>    // Biblioteca MQTT Publicar e Subescrever


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


#define HOST_NAME "remotedebug"

// Board especific libraries

#if defined ESP8266 || defined ESP32

// Use mDNS ? (comment this do disable it)

#define USE_MDNS true

// Arduino OTA (uncomment this to enable)

//#define USE_ARDUINO_OTA true

#else

// RemoteDebug library is now only to Espressif boards,
// as ESP32 and ESP82266,
// If need for another WiFi boards,
// please add an issue about this
// and we will see if it is possible made the port for your board.
// access: https://github.com/JoaoLopesF/RemoteDebug/issues

#error "The board must be ESP8266 or ESP32"

#endif // ESP

//////// Libraries

#if defined ESP8266

// Includes of ESP8266

#include <ESP8266WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#endif

#elif defined ESP32

// Includes of ESP32

#include <WiFi.h>

#ifdef USE_MDNS
#include <DNSServer.h>
#include "ESPmDNS.h"
#endif

#endif // ESP

// Remote debug over WiFi - not recommended for production, only for development

#include <RemoteDebug.h>       //https://github.com/JoaoLopesF/RemoteDebug


RemoteDebug Debug;

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

// SSID and password

const char* ssid = "Andre";
const char* password = "Julia220816";


ATM90E36 eic(5);

void iniciaTelnet();

void ota()
{
  if(OTADRIVE.timeTick(30))  //30
  {
    OTADRIVE.updateFirmware();
  }
}


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


  eic.begin();
    conectar_mqtt();
    delay(500);

  //iniciaTelnet();
  delay(1000);

  OTADRIVE.setInfo("a04979c1-492a-4e55-834b-d851d8572755", "v@1.1.4");
}



void loop() {

  

  digitalWrite(LED1,!digitalRead(LED1));
  digitalWrite(LED2,!digitalRead(LED2));
  digitalWrite(LED3,!digitalRead(LED3));

  ota();
  
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
  // delay(1000);



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

  //GetActivePowerA();
  //GetActivePowerB();
  //GetActivePowerC();
  //GetReactivePowerA();
  //GetReactivePowerB();
  //GetReactivePowerC();
  //GetApparentPowerA();
  //GetApparentPowerB();
  //GetApparentPowerC();
  //GetPowerFactorA();
  //GetPowerFactorB();
  //GetPowerFactorC();
  

    
  
  // debugI("Voltage A: %.3fV | B: %.3fV | C: %.3fV ", voltageA,voltageB,voltageC);
  // debugI("Current A: %.3fA | B: %.3fA | C: %.3fA ", currentA,currentB,currentC);
  // debugI("ActivePower: %.3fW | ReactivePower: %.3fVAR", totalActivePower, totalReactivePower);
  // debugI("ApparentPower: %.3fVA | PowerFactor: %.2f ", totalApparentPower, totalFactorPower);
  // debugI("Freq: %.3fHz| Temp: %.3fC", freq, temp);
  // debugI("__________________________________________________________");
  // debugI("");
 
  // Debug.handle();
  // delay(1000);


  client.loop();

  if(!client.connected()){
     //Serial.println("status wifi...");
     //Serial.println(WiFi.status());
     delay(3000);
     //Serial.println(WiFi.status());

      if (WiFi.status() == 7)
      {

      //Serial.println("***************************************");home
      //Serial.println("conexao perdida...");
     //online = 1;

      }
  conectar_mqtt();
}

char tempString[10];

dtostrf(voltageA, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/va", tempString);

dtostrf(voltageB, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/vb", tempString);

dtostrf(voltageC, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/vc", tempString);

dtostrf(currentA, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/ia", tempString);

dtostrf(currentB, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/ib", tempString);

dtostrf(currentC, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/ic", tempString);

dtostrf(totalActivePower, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/p", tempString);

dtostrf(temp, 10, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
client.publish("home/tempATM90", tempString);

delay(1000);
}



void iniciaTelnet()
{
    // WiFi connection

    WiFi.begin(ssid, password);
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

    // Register host name in WiFi and mDNS

    String hostNameWifi = HOST_NAME;
    hostNameWifi.concat(".local");


#ifdef USE_MDNS  // Use the MDNS ?

    if (MDNS.begin(HOST_NAME)) {
        Serial.print("* MDNS responder started. Hostname -> ");
        Serial.println(HOST_NAME);
    }

    MDNS.addService("telnet", "tcp", 23);

#endif

  // Initialize RemoteDebug

    Debug.begin(HOST_NAME); // Initialize the WiFi server
    Debug.setResetCmdEnabled(true); // Enable the reset command
    Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
    Debug.showColors(true); // Colors

    // End off setup
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("*");
    Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");


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
