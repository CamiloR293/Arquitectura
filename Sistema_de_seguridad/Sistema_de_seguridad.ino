#include <LiquidCrystal.h>
#include <Keypad.h>

/* Display */
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2}; //R1 = 5, R2 = 4, R3 = 3. R4 = 2
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'.', '0', '=', '/'}
};

const int redPin = 15; 
const int greenPin = 16;
const int bluePin = 17; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

uint64_t value = 0;

char password[5];
char passwordCorrect[5] = "12345";
unsigned char idx = 0;
int failCount = 0;
bool flag = false;



void setup() {
  // initialize LCD and set up the number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Ingrese la clave");
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT); 
  pinMode(bluePin, OUTPUT); 

  color(0, 0, 0); // turn the RGB LED red

  lcd.setCursor(2, 1);
  idx = 0;
  flag = false;
}

void loop() {
  // read the potentiometer on A0:

  char key = keypad.getKey();
  
  if (key){
    password[idx++] = key;
    lcd.print("*");
    if(idx == 5){
      for (int i = 0; i < 5; i++){
        if(password[i] != passwordCorrect[i]){
          lcd.clear();
          lcd.print("Clave incorrecta.");
          //digitalWrite(led_red, HIGH);
          color(255, 0, 0); //red On
          delay(1000);
          failCount++;
          setup();
          break;
        }else{
          flag = true;
        }
      }
      if(flag){
        lcd.clear();
        lcd.print("Clave correcta.");
        //digitalWrite(led_green, HIGH);
        color(0, 255, 0); // green On
      }else if(failCount >= 3){
        lcd.clear();
        lcd.print("Sistema bloqueado.");
        //digitalWrite(led_red, HIGH);
        color(255, 0, 0); //red on
      }
    }
  }
}
void color (unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
  analogWrite(redPin, red);
  analogWrite(bluePin, blue);
  analogWrite(greenPin, green);
}
