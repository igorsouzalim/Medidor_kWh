#include <arduino.h>
#include <otadrive_esp.h>

void setup()
{
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);

  
  WiFi.begin("Andre","Julia220816");
  OTADRIVE.setInfo("a04979c1-492a-4e55-834b-d851d8572755", "v@1.1.2");
}

void ota()
{
  if(OTADRIVE.timeTick(30))
  {
    OTADRIVE.updateFirmware();
  }
}

void loop()
{
  digitalWrite(2,HIGH);
  delay(50);
  digitalWrite(2,LOW);
  delay(50);
  
  ota();
}
