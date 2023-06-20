#include "AsyncTaskLib.h"
#include "DHTStable.h"
#include <LiquidCrystal.h>

DHTStable DHT;

#define DHT11_PIN       A5
#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);
const int redPin = 15; 
const int greenPin = 16;
const int bluePin = 17; 

int outputValue = 0;
const int photocellPin = A0;
void readLight();
void readTemp();

AsyncTask asyncTask1(1000,true, readTemp);
AsyncTask asyncTask2(2000,true, readLight);
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);


void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT); 
  pinMode(bluePin, OUTPUT); 
  
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHTSTABLE_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");

  asyncTask1.Start();
  asyncTask2.Start();
}

void loop()
{
  asyncTask1.Update();
  asyncTask2.Update();
}


void readTemp() {
  
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
      Serial.print("OK,\t"); 
      break;
    case DHTLIB_ERROR_CHECKSUM: 
      Serial.print("Checksum error,\t"); 
      break;
    case DHTLIB_ERROR_TIMEOUT: 
      Serial.print("Time out error,\t"); 
      break;
    default: 
      Serial.print("Unknown error,\t"); 
      break;
  }

  // DISPLAY DATA

  if (DHT.getTemperature() >= 25) //
  {
    color(255, 0, 0); //red On
  }
  else
  {
    color(0, 255, 0); //green On
  }
  
  //print on lcd humidity
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity:" );
  lcd.print(DHT.getHumidity());
  Serial.print(DHT.getHumidity(), 1);

  Serial.print(",\t");

  //print on lcd temp
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.print(DHT.getTemperature());
  Serial.println(DHT.getTemperature(), 1);

  delay(2000);
  lcd.clear();
}

void readLight() {
  outputValue = analogRead(photocellPin);
  Serial.println(outputValue);
  lcd.print("Light: ");
  lcd.print(outputValue);
  delay(1000);
  lcd.clear();
}

void color (unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
  analogWrite(redPin, red);
  analogWrite(bluePin, blue);
  analogWrite(greenPin, green);
}