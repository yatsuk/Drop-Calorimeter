#include <TimerOne.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer;
bool temperatureSensorStop;


bool pulse;
bool pulseIsHigh;
bool dropComplited;
bool droped;
bool detectorRunning;
bool dropIsTimeout;
unsigned long pulseCount;
unsigned long missedPulseCount;
unsigned long pulseMaxCount = 25000;
int delayMs = 50;
unsigned long timerPeriod = 200;
unsigned long startDropTime;

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), irPulse, RISING);
  Timer1.initialize(timerPeriod);
  Timer1.setPeriod(timerPeriod);
  Timer1.stop();
  Timer1.attachInterrupt(timer1Callback);


  temperatureSensorStop = false;
  sensors.getAddress(thermometer, 0); 
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(true);

}

void timer1Callback()
{ 
  if (pulseIsHigh){
    if (!pulse){
      missedPulseCount++;
    } else if (droped && pulse){
      dropComplited = true;
    }
    if (missedPulseCount > 4 && !droped){
      droped = true;
      delay(delayMs);
      Serial.print("drop ok: drop time = ");
      Serial.print(pulseCount * timerPeriod * 2, DEC);
      Serial.println(" mcs");
    }


    if (pulseCount > pulseMaxCount){
      dropComplited = true;
      dropIsTimeout = true;
    }
    pulseCount++;

    pulse = false;
  }

  pulseIsHigh =! pulseIsHigh;
  digitalWrite(4, pulseIsHigh);
}

void irPulse()
{
  pulse = true;
}

void test()
{
  int irValue;
  bool testOk = false;
  Serial.println("Begin test detector");
  detachInterrupt(digitalPinToInterrupt(2));
  digitalWrite(4, LOW);
  irValue = digitalRead(2);
  if (!irValue){
    testOk = true;
  }

  digitalWrite(4, HIGH);
  delay(200);
  irValue = digitalRead(2);
  if (testOk && irValue){
    testOk = true;
  } else {
    testOk = false;
  }
  
  if (testOk){
    Serial.println("End test: ok");
  } else {
    Serial.println("End test: fail");
  }

  digitalWrite(4, LOW);
  attachInterrupt(digitalPinToInterrupt(2), irPulse, RISING);
}

void getValueTemperatureSensor()
{
    int i = 0;
    float sum = 0;
    float result = 0;
    unsigned long timeBeginMeas = millis();
    unsigned int maxTimeMeas = 1500; //milliseconds
    while((millis() - timeBeginMeas) <= maxTimeMeas){
      sensors.requestTemperatures();
      sum+=+sensors.getTempCByIndex(0);
      i++;
      if(temperatureSensorStop) return;
    }
    result=sum/i;
    Serial.print("cold_water_temperature: ");
    Serial.println(result);
}

void startDetector()
{
    temperatureSensorStop = true;
    detectorRunning = true;
    digitalWrite(4, LOW);
    pulseIsHigh = false;
    dropComplited = false;
    droped = false;
    pulse = false;
    dropIsTimeout = false;
    pulseCount = 0;
    missedPulseCount = 0;
    startDropTime = millis();
    Timer1.start();
}

void stopDetector()
{
    detectorRunning = false;
    Timer1.stop();
    digitalWrite(4, LOW);
    temperatureSensorStop = false;
}

void loop() {
  if (Serial.available() > 0){
    int incomingByte = Serial.read();
    switch (incomingByte){
    case '1': 
      Serial.println("Start detector");
      startDetector();
      break;
    case '2': 
      Serial.println("Stop detector");
      stopDetector();
      break;
   case '4': 
      digitalWrite(4, LOW);
      test();
      break;
   case '5': 
      getValueTemperatureSensor();
      break;
    }
  }
  if (dropComplited && detectorRunning){
    stopDetector();   
    if (!dropIsTimeout){
      Serial.print("pulses = ");
      Serial.print(missedPulseCount, DEC);
      Serial.print(", blackout time = ");
      Serial.print(missedPulseCount * 2 * timerPeriod, DEC);
      Serial.println(" mcs");
    } else {
      Serial.println("drop fail: timeout (10000 ms)");
    }
  }
  delay(1);
}
