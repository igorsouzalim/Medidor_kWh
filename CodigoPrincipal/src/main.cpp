/**
 *  ---Funcionalidaes para adicionar---
 * 
 * - Tempo de scan do OTA - OK
 * - botão para zerar o KW
 * 
 * ----Bugs------
 * - Evitar potencia negativa no somatorio e na mostragem pro mqtt - OK (testar)
 */

#include "header.h"

void setup_wifi();
void vTaskOTA(void *pvParameters);
void vTaskIntegralEnergy(void *pvParameters);
void vTaskPublishMQTT(void *pvParameters);

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
  kW_totalAnterior = kW_total;


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

  OTADRIVE.setInfo("cc901072-5ee5-4003-bb41-372d1780b039", "v@1.2.4");

  xTaskCreatePinnedToCore( vTaskOTA, "Task OTA", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL, CORE_1 );
  xTaskCreatePinnedToCore( vTaskIntegralEnergy, "Task Energy", configMINIMAL_STACK_SIZE*1, NULL, 1, NULL, CORE_0 );
  xTaskCreatePinnedToCore( vTaskPublishMQTT, "Task OTA", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL, CORE_1 );
}


void loop() {


delay(10);
}



void vTaskOTA(void *pvParameters){
  for(;;)
  {
     ota();
     digitalWrite(LED3,!digitalRead(LED3));
    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}


void vTaskIntegralEnergy(void *pvParameters){
  for(;;)
  {
    double totalActivePower = eic.GetTotalActivePower();
      if(totalActivePower>(float)0.0000)
      {
        kW_total += (float)totalActivePower/3600;

        if( kW_total >= (float)(kW_totalAnterior+(float)0.010))
        {
          kW_totalAnterior = kW_total;
          preferences.putFloat("kW_total", kW_total); 
          
          
          //client.publish("home/powerMeter/debug", "kw_saved");
        }
      }
      else if(totalActivePower<(float)0.0000)
      {
        //client.publish("home/powerMeter/debug", "NegativePowerMesurement");
      }

      if(erase == 1)
      {
        erase = 0;
        preferences.putFloat("kW_total", 0);
      }
    
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}


void vTaskPublishMQTT(void *pvParameters){
  uint16_t onlineCounter=0; 

  for(;;)
  {
    double voltageA,freq,voltageB,voltageC,currentA,currentB,currentC,totalActivePower,totalReactivePower,totalApparentPower,totalFactorPower,temp;

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
    client.publish("home/powerMeter/va", tempString);

    dtostrf(currentA, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/ia", tempString);

    dtostrf(totalActivePower, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/p", tempString);

    // dtostrf(temp, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    // client.publish("home/powerMeter/tempATM90", tempString);

    dtostrf((kW_total*1000), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao);  
    client.publish("home/powerMeter/monthlyConsumption", tempString);

    // dtostrf((daily_consumption*1000), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao);  
    // client.publish("home/powerMeter/dailyConsumption", tempString);

    dtostrf((kW_total), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/kwTotal", tempString);

    dtostrf((onlineCounter), 6, 0, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/onlineCounter", tempString);

    Serial.println("send mqtt...");

    digitalWrite(LED2,!digitalRead(LED2));
    vTaskDelay(2000/portTICK_PERIOD_MS);
    onlineCounter++;  //Monitoramento de continuidade do sistema
  }
}