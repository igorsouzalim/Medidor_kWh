
#include <SPI.h>
#include <ATM90E36.h>
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

// SSID and password

const char* ssid = "ESP32";
const char* password = "eletronica";


ATM90E36 eic(5);

void iniciaTelnet();

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
  

  digitalWrite(DMA_CTRL, LOW);
  digitalWrite(PM0, HIGH);
  digitalWrite(PM1, HIGH);
  
  eic.begin();

  iniciaTelnet();
  delay(1000);
}



void loop() {
  
  /*Repeatedly fetch some values from the ATM90E36 */
  double voltageA,freq,voltageB,voltageC,currentA,currentB,currentC,totalActivePower,totalReactivePower,totalApparentPower,totalFactorPower,temp;
  int sys0=eic.GetSysStatus0();
  int sys1=eic.GetSysStatus1();
  int en0=eic.GetMeterStatus0();
  int en1=eic.GetMeterStatus1();

  
  Serial.println("S0:0x"+String(sys0,HEX));
  delay(10);
  Serial.println("S1:0x"+String(sys1,HEX));
  delay(10);
  Serial.println("E0:0x"+String(en0,HEX));
  delay(10);
  Serial.println("E1:0x"+String(en1,HEX));


  voltageA=eic.GetLineVoltageA();
  Serial.println("VA:"+String(voltageA)+"V");
  voltageB=eic.GetLineVoltageB();
  Serial.println("VB:"+String(voltageB)+"V");
  voltageC=eic.GetLineVoltageC();
  Serial.println("VC:"+String(voltageC)+"V");
  delay(10);
  currentA = eic.GetLineCurrentA();
  Serial.println("IA:"+String(currentA)+"A");
  currentB = eic.GetLineCurrentB();
  Serial.println("IB:"+String(currentB)+"A");
  currentC = eic.GetLineCurrentC();
  Serial.println("IC:"+String(currentC)+"A");
  delay(10);
  freq=eic.GetFrequency();
  delay(10);
  Serial.println("f"+String(freq)+"Hz");
  delay(10);
  temp=eic.GetTemperature();
  delay(10);
  Serial.println("Temperatura:"+String(temp)+"C");
  delay(10);
  totalActivePower = eic.GetTotalActivePower();
  Serial.println("potencia ativa:"+String(totalActivePower)+"W");
  delay(1000);
  delay(1000);



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
  

    
  
  debugI("Voltage A: %.3fV | B: %.3fV | C: %.3fV ", voltageA,voltageB,voltageC);
  debugI("Current A: %.3fA | B: %.3fA | C: %.3fA ", currentA,currentB,currentC);
  debugI("ActivePower: %.3fW | ReactivePower: %.3fVAR", totalActivePower, totalReactivePower);
  debugI("ApparentPower: %.3fVA | PowerFactor: %.2f ", totalApparentPower, totalFactorPower);
  debugI("Freq: %.3fHz| Temp: %.3fC", freq, temp);
  debugI("__________________________________________________________");
  debugI("");
 
  Debug.handle();
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
