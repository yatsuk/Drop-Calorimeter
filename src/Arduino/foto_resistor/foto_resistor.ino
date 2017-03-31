int foto_resistor_value = 0;
int foto_resistor_old_value;
int foto_resistor_ADC_pin = 0;
int delta_foto_resitor = 50;
double average_value_foto_resistor = 0;
double max_delta_value_foto_resistor = 0;
bool first_ADC_read = true;
bool stop_ADC_read = true;

unsigned long time;
unsigned long timeout_ms = 5000;

void setup()
{
  Serial.begin(115200);
  analogReference(DEFAULT);
}

void loop()
{
    if (!stop_ADC_read){
        foto_resistor_value = analogRead(foto_resistor_ADC_pin);
        
        if (first_ADC_read){
            int readValueTimes =30;
            average_value_foto_resistor = 0;
            max_delta_value_foto_resistor = 0;
            for (int i = 0; i < readValueTimes; i++){
              average_value_foto_resistor += analogRead(foto_resistor_ADC_pin);
            }
            average_value_foto_resistor/=readValueTimes;
            first_ADC_read = false;
        } else {
            double delta = foto_resistor_value - average_value_foto_resistor;
            max_delta_value_foto_resistor = delta > max_delta_value_foto_resistor ? delta : max_delta_value_foto_resistor;
            if ((millis() - time) > timeout_ms){
            stop_ADC_read = true;
            Serial.print("fail: drop timeout. Resistor value = ");
            Serial.print(foto_resistor_value);
            Serial.print(", max delta = ");
            Serial.print(max_delta_value_foto_resistor);
            Serial.println("");
            }
            else if (delta > delta_foto_resitor){
                unsigned long endDropTime = millis();
                Serial.print("drop ");
                Serial.print(average_value_foto_resistor);
                Serial.print(" ");
                Serial.print(foto_resistor_value);
                Serial.println("");
                Serial.print("Drop time = ");
                Serial.print(endDropTime - time);
                Serial.print("Max value fotoresistor = ");
                Serial.print(getMaxValueFotoResistor());
                Serial.println("");
                
                stop_ADC_read = true;
            }
        }
        delay(5);
    } else {
        if (Serial.available() > 0) {  //если есть доступные данные
            int incomingByte = Serial.read();
            if (incomingByte == '1'){
                Serial.println("ADC start");
                time = millis();
                stop_ADC_read = false;
                first_ADC_read = true;
            }else if (incomingByte == '4'){
                 int readValueTimes =30;
                 double average_value_foto_resistor = 0;
                 for (int i = 0; i < readValueTimes; i++){
                   average_value_foto_resistor += analogRead(foto_resistor_ADC_pin);
                 }
                 average_value_foto_resistor/=readValueTimes;
                 Serial.print("Test fotoresistor. Value = ");
                 Serial.print(average_value_foto_resistor);
                 Serial.println("");
            }
        }
    }
}

int getMaxValueFotoResistor()
{
  int maxValue = 0;
  const unsigned long timeDeltaMs = 500;
  unsigned long time = millis();
  
  int foto_resistor_value = analogRead(foto_resistor_ADC_pin);
  int foto_resistor_MaxValue = foto_resistor_value;
  
  while (millis() < (time + timeDeltaMs)){
    foto_resistor_value = analogRead(foto_resistor_ADC_pin);
    if (foto_resistor_value > foto_resistor_MaxValue)
        foto_resistor_MaxValue = foto_resistor_value;
  }
  return foto_resistor_MaxValue;
}
