#include "StateMachineLib.h"
#include "DHTStable.h"
#include "AsyncTaskLib.h"
#include <LiquidCrystal.h>
#include <Keypad.h>


void timeout_T1();
void timeout_T2();
void timeout_T3();
void timeout_T4();
void measure_Temp();
void readPassword();

void intHallSensor();
void intTrackingSensor();
void intMetalTouchSensor();

#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);

//Def liquid and dht
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
DHTStable DHT;

//Def asyncTask
AsyncTask asyncTaskTime(2000,timeout_T1);
AsyncTask asyncTaskTime1(10000,timeout_T2);
AsyncTask asyncTaskTime2(6000,timeout_T3);
AsyncTask asyncTaskTemp(500, true, measure_Temp);
AsyncTask asyncTaskTime3(5000,timeout_T4);


// Keypad setup 
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'.', '0', '=', '/'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
#define DHT11_PIN       A15
const int buzzerPin = A14;
int fre;//set the variable to store the frequence value
int execute = 0;
const int redPin = 15; 
const int greenPin = 16;
const int bluePin = 17;
const int intPinHall = 19;
const int intPinMetal = 20;
const int analogPin = A8;
const int intPinTracking = 21;

float tempOnState = 0;  

char inputPassword[5];
char password[5] = "12345";
unsigned char idx = 0;

int failCount = 0;

bool flag = false;
bool buzzer = false;
bool gate = false;

// State Alias
enum State
{
  IngresoSeguridad = 0,
  MonitorAmbiental = 1,
  EventosPuertosYVentanas = 2,
  AlertaSeguridad = 3,
  AlarmaAmbiental = 4
};

// Input Alias
enum Input
{
  passwordCorrect = 0,
  timeOut = 1,
  gateOpen = 2,
  tempVeriffy = 3,
  tempAndTime = 4,
  Unknown = 5,
};

// Create new StateMachine
StateMachine stateMachine(5, 8);

// Stores last user input
Input input;
//====================================================
//Entrys
//====================================================
void entryIngreso(){
  
  idx = 0;
  flag = false;
  color(0,0,0);
  lcd.clear();
  
  Serial.println("In Ev Mo AlS AlA");
  Serial.println("X               ");
  Serial.println();
  lcd.print("Ingrese la clave:");
  lcd.setCursor(0,1);
}
void entryEvents(){
  buzzer = false;
  color(0,0,0);
  lcd.clear();
  asyncTaskTime.Start(); //timeout 2 sec
  asyncTaskTemp.Stop(); //Detener escaneo de temperatura
  Serial.println("In Ev Mo AlS AlA");
  Serial.println("   X            ");
  Serial.println();
  lcd.print("En eventos");
}
void entryMonitor(){
  
  lcd.clear();
  asyncTaskTime1.Start(); //timeout 10 sec
  asyncTaskTemp.Start(); //Revisar temperatura
  tempOnState = 25.3;
  Serial.println("In Ev Mo AlS AlA");
  Serial.println("      X         ");
  Serial.println();
  lcd.print("En monitor");
}
void entryAlertSecurity(){
  execute = 0;
  lcd.clear();
  color(255,0,255);
  asyncTaskTime.Stop(); //que deje de contar 2 segundos
  asyncTaskTime1.Stop();
  asyncTaskTime3.Stop();
  asyncTaskTemp.Stop();
  asyncTaskTime2.Start();
  tempOnState = 24.5;
  Serial.println("In Ev Mo AlS AlA");
  Serial.println("         X      ");
  Serial.println();
  lcd.print("Alerta seguridad");
  buzzer = true;
}
void entryAlarmEnvironment(){
  color(0,0,255);
  asyncTaskTime1.Stop();
  lcd.clear();
  if(DHT.getTemperature() < 25.5){
    asyncTaskTime1.Stop();
    input = Input::tempVeriffy;
  }
  Serial.println("In Ev Mo AlS AlA");
  Serial.println("             X  ");
  Serial.println();
  lcd.print("Alarma ambiente");
}
//====================================================
//====================================================
// Setup the State Machine
//====================================================
void setupStateMachine()
{
  // Add transitions
  stateMachine.AddTransition(IngresoSeguridad, EventosPuertosYVentanas, []() { return input == passwordCorrect; });

  stateMachine.AddTransition(EventosPuertosYVentanas, MonitorAmbiental, []() { return input == timeOut; });
  stateMachine.AddTransition(EventosPuertosYVentanas, AlertaSeguridad, []() { return input == gateOpen; });
  stateMachine.AddTransition(AlertaSeguridad, EventosPuertosYVentanas, []() { return input == timeOut; });

  stateMachine.AddTransition(MonitorAmbiental, AlarmaAmbiental, []() { return input == tempVeriffy; });
  stateMachine.AddTransition(MonitorAmbiental, EventosPuertosYVentanas, []() { return input == timeOut; });
  stateMachine.AddTransition(AlarmaAmbiental, MonitorAmbiental, []() { return input == tempVeriffy; });
  stateMachine.AddTransition(AlarmaAmbiental, AlertaSeguridad, []() { return input == tempAndTime; });

  stateMachine.AddTransition(AlertaSeguridad, EventosPuertosYVentanas, []() { return input == timeOut; });

  // Add actions
  stateMachine.SetOnEntering(IngresoSeguridad, entryIngreso);
  stateMachine.SetOnEntering(EventosPuertosYVentanas, entryEvents);
  stateMachine.SetOnEntering(MonitorAmbiental, entryMonitor);
  stateMachine.SetOnEntering(AlarmaAmbiental, entryAlarmEnvironment);
  stateMachine.SetOnEntering(AlertaSeguridad, entryAlertSecurity);

  stateMachine.SetOnLeaving(IngresoSeguridad, []() {Serial.println("Leaving IngresoSeguridad"); });
  stateMachine.SetOnLeaving(EventosPuertosYVentanas, []() {Serial.println("Leaving EventosPuertosYVentanas"); });
  stateMachine.SetOnLeaving(MonitorAmbiental, []() {Serial.println("Leaving MonitorAmbiental"); });
  stateMachine.SetOnLeaving(AlarmaAmbiental, []() {Serial.println("Leaving AlarmaAmbiental"); });
  stateMachine.SetOnLeaving(AlertaSeguridad, []() {Serial.println("Leaving AlertaSeguridad"); });
}

//====================================================
void setup() 
{
  lcd.begin(16, 2);
  lcd.clear();
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT); 
  pinMode(bluePin, OUTPUT); 
  pinMode(buzzerPin, OUTPUT);

  pinMode(intPinHall, INPUT);
  pinMode(intPinTracking, INPUT);
  pinMode(intPinMetal, INPUT);

  color(0, 0, 0); //turn off

  lcd.print("Hello!");
  delay(500);
  lcd.clear();
  
  attachInterrupt(digitalPinToInterrupt(intPinHall), intHallSensor, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPinTracking), intTrackingSensor, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPinMetal), intMetalTouchSensor, FALLING);

  Serial.begin(115200);
  Serial.println("Starting State Machine...");

  lcd.print("Starting State ");
  lcd.setCursor(0,1);
  lcd.print("Machine...");
  
  setupStateMachine();  
  
  delay(1000);
  lcd.clear();

  Serial.println("Start Machine Started");
  
  lcd.print("Start Machine ");
  lcd.setCursor(0,1);
  lcd.print("Started");
  
  delay(500);
  lcd.clear();
  
  // Initial state
  stateMachine.SetState(IngresoSeguridad, false, true);
}
//====================================================

//====================================================
void loop() 
{
  // Read user input
  input = static_cast<Input>(readInput());

  // Update async tasks
  asyncTaskTime.Update();
  asyncTaskTime1.Update();
  asyncTaskTime2.Update();
  asyncTaskTime3.Update();
  asyncTaskTemp.Update();

  activateAlert();
  
  if(buzzer){
    buzz();
  }else{
    noTone(buzzerPin);
  }
  
  stateMachine.Update();

}
//====================================================



//====================================================
// Auxiliar function that reads the user input
//====================================================
int readInput(){
  Input currentInput = Input::Unknown;
  char key = keypad.getKey();
  
  if (key){
    inputPassword[idx++] = key;
    lcd.print("*");
    if(idx == 5){
      for (int i = 0; i < 5; i++){
        if(inputPassword[i] != password[i]){
          lcd.clear();
          lcd.print("Clave incorrecta.");
          color(255, 0, 0);
          delay(1000);
          failCount++;
          entryIngreso();
          flag = false;
          break;
        }else{
          flag = true;
        }
      }
      if(flag){
        lcd.clear();
        lcd.print("Clave correcta.");
        color(0, 255, 0);
        delay(1000);
        currentInput = Input::passwordCorrect;
      }else if(failCount >= 3){
        lcd.clear();
        lcd.print("Sistema bloqueado.");
        color(255, 0, 0);
        delay(1000);
      }
    }
  }
  
  return static_cast<int>(currentInput);
}


//====================================================
//buzz
//====================================================

void buzz(){
  tone(buzzerPin,800);
}

//====================================================
//Interrupciones
//====================================================

void activateAlert(){
  if(gate && stateMachine.GetState() == EventosPuertosYVentanas){
    input = Input::gateOpen;
    gate = false;
  }

}

void intHallSensor() {
  gate = true;
  Serial.println("Sensor de campo magnetico activado");
}

void intTrackingSensor() {
  gate = true;
  Serial.println("Sensor de seguimiento activado");
}

void intMetalTouchSensor() {
  gate = true;
  Serial.println("Sensor de metales activado");
}

//====================================================
//timeouts
//====================================================
void timeout_T1(){
  DEBUG("timeout 2 sec going monitor");
  
  input = Input::timeOut;
}
void timeout_T2(){
  DEBUG("going events timeout 10 sec");
  color(0,0,0);
  input = Input::timeOut;
}
void timeout_T3(){
  DEBUG("going events timeout 6 sec");
  color(0,0,0);
  input = Input::timeOut;
}
void timeout_T4(){
  DEBUG("going alertSecurity 5 sec");
  color(255,0,0);
  input = Input::tempAndTime;
}
//====================================================
//measure_Temp
//====================================================
void measure_Temp() {
  
  input = Input::Unknown;
  Serial.println("Temp");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk) {
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
  float value_Temp = DHT.getTemperature();
  Serial.println(value_Temp, 1);
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(value_Temp);

  if (stateMachine.GetState() == MonitorAmbiental && value_Temp >= 25) {
    input = Input::tempVeriffy;
  }
  if(stateMachine.GetState() == AlarmaAmbiental && value_Temp >= 25 && execute < 1){
    execute++;
    asyncTaskTime3.Start();
    asyncTaskTemp.Stop();
  }
  if (stateMachine.GetState() == AlarmaAmbiental && value_Temp < 25) {
    input = Input::tempVeriffy;
  }
}

void color (unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
  analogWrite(redPin, red);
  analogWrite(bluePin, blue);
  analogWrite(greenPin, green);
}
