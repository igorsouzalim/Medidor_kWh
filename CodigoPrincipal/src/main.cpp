/**
 *  ---Funcionalidaes para adicionar---
 * 
 * - Tempo de scan do OTA - OK
 * - botão para zerar o KW
 * - Botar segundo wifi para conectar 
 * - verificar se está gravando kw sem wifi conectado
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
  kw_mensal = preferences.getFloat("kw_mensal", 0); 
  kW_diario = preferences.getFloat("kW_diario", 0); 
  custo_parcial = preferences.getFloat("custo_parcial", 0); 
  custo_mensal = preferences.getFloat("custo_mensal", 0); 
  custo_diario = preferences.getFloat("custo_diario", 0); 

  //ssid = preferences.getString("SSID", ""); 
  //password = preferences.getString("Password", "");


    Serial.println();
    Serial.println(">>> [STARTS WITH STORED IN EEPROM]");
    // Serial.print("ID: "); Serial.println(ID); 
    Serial.print("kW_total: "); Serial.println(kW_total);  
    // Serial.print("WIFI_SSID: "); Serial.println(ssid); 
    // Serial.print("WIFI_PASSWORD: "); Serial.println(password); 
    // Serial.println("____________________________");

  //   if (volumeAguaMensal == 0 )
  //     Serial.println("No values saved for volumeAguaMensal");
  //   if (ssid == "" || password == "")
  //   Serial.println("No values saved for ssid or password");
  

  digitalWrite(DMA_CTRL, LOW);
  digitalWrite(PM0, HIGH);
  digitalWrite(PM1, HIGH);

  digitalWrite(LED1,1);
  digitalWrite(LED2,1);
  digitalWrite(LED3,1);

  eic.begin();
  
  delay(1000);

  xTaskCreatePinnedToCore( vTaskIntegralEnergy, "Task Energy", configMINIMAL_STACK_SIZE*8, NULL, 5, NULL, CORE_0 );
  
  setup_wifi();
  conectar_mqtt();

  xTaskCreatePinnedToCore( vTaskOTA, "Task OTA", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL, CORE_1 );
  xTaskCreatePinnedToCore( vTaskPublishMQTT, "Task mqtt", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL, CORE_1 );

  OTADRIVE.setInfo("cc901072-5ee5-4003-bb41-372d1780b039", "v@1.2.6");
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
  uint32_t timeToSave = 0, vFault = 0, vFaultTimerSaveProtection = 0;
  for(;;)
  {
    double totalActivePower = eic.GetTotalActivePower();
      if(totalActivePower>(float)0.0000)
      {
        kW_total += (float)totalActivePower/36000;
        custo_mensal = kW_total;
      }
      else if(totalActivePower<(float)0.0000)
      {
        //client.publish("home/powerMeter/debug", "NegativePowerMesurement");
      }

      switch (erase)
      {
        case 1:
          erase = 0;
          preferences.putFloat("kW_total", 0);
          kW_total = 0;
        break;

        case 2:
          erase = 0;
          preferences.putFloat("kw_mensal", 0);
          kw_mensal = 0;
        break;

        case 3:
          erase = 0;
          preferences.putFloat("kW_diario", 0);
          kW_diario = 0;
        break;

        case 4:
          erase = 0;
          preferences.putFloat("custo_parcial", 0);
          custo_parcial = 0;
        break;

        case 5:
          erase = 0;
          preferences.putFloat("custo_mensal", 0);
          custo_mensal = 0;
        break;

        case 6:
          erase = 0;
          preferences.putFloat("custo_diario", 0);
          custo_diario = 0;
        break;
        
        default:
          break;
      }

      double voltageA=eic.GetLineVoltageA();
      if(voltageA < 30.0)   // Faltou luz
      {
        vFault = 1;
      }
      else{
        vFault = 0;
      }

      if(vFaultTimerSaveProtection < 1200)  // 2 minutos 
        vFaultTimerSaveProtection++;

      if(vFault == 1 && vFaultTimerSaveProtection >= 1200)
      {
        vFault = 0;
        vFaultTimerSaveProtection = 0;
        preferences.putFloat("kW_total", kW_total);
        preferences.putFloat("kw_mensal", kw_mensal); 
        preferences.putFloat("kW_diario", kW_diario); 
        preferences.putFloat("custo_parcial", custo_parcial); 
        preferences.putFloat("custo_mensal", custo_mensal); 
        preferences.putFloat("custo_diario", custo_diario); 
        Serial.println("vFault Saved eprom");
        debugTimerEprom++;
      }

      if(timeToSave < 144000)
        timeToSave ++;
      else if(timeToSave >= 144000 ){
        timeToSave=0;
        preferences.putFloat("kW_total", kW_total);
        preferences.putFloat("kw_mensal", kw_mensal); 
        preferences.putFloat("kW_diario", kW_diario); 
        preferences.putFloat("custo_parcial", custo_parcial); 
        preferences.putFloat("custo_mensal", custo_mensal); 
        preferences.putFloat("custo_diario", custo_diario); 
        Serial.println("saved eeprom");
      }
      
    vTaskDelay(100/portTICK_PERIOD_MS);
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

    dtostrf(voltageA, 6, 1, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/va", tempString);

    dtostrf(currentA, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/ia", tempString);

    dtostrf(totalActivePower*1000, 0, 0, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/actualPower", tempString);

    dtostrf((kW_total), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/kwTotal", tempString);

    // dtostrf((kw_mensal), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    // client.publish("home/powerMeter/kwMensal", tempString);

    // dtostrf((kW_diario), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    // client.publish("home/powerMeter/kwDiario", tempString);

    dtostrf((custo_mensal), 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao);  
    client.publish("home/powerMeter/custoMensal", tempString);

    // dtostrf((custo_mensal), 6, 4, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao);  
    // client.publish("home/powerMeter/custoDiario", tempString);

    dtostrf((custo_mensal), 6, 4, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao);  
    client.publish("home/powerMeter/custoParcial", tempString);

    // dtostrf(temp, 6, 3, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    // client.publish("home/powerMeter/tempATM90", tempString);
  
    dtostrf((onlineCounter), 6, 0, tempString); // dtostrf(variavel,comprimentoDaString,casasDecimais,RecebeAConversao); 
    client.publish("home/powerMeter/onlineCounter", tempString);
    
    Serial.println("send mqtt...");

    digitalWrite(LED2,!digitalRead(LED2));
    vTaskDelay(2000/portTICK_PERIOD_MS);
    onlineCounter++;  //Monitoramento de continuidade do sistema
  }
}