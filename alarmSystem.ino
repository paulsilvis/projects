#include <Keypad.h>

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
    {'6','4','5'},
    {'9','7','8'},
    {'#','*','0'},
    {'3','1','2'}
};

/* Wiring:

  pad -- > Arduino
  
  1  -->  6  
  2  -->  2
  3  -->  7
  4  -->  3
  5  -->  8
  6  -->  4
  7  -->  5
*/

enum State {
  INIT,
  START,
  M1,
  M2,
  M3,
  M4,
};

char code[] = "5857";
int state = INIT;
bool enabled = false;
void monitorKeypad(char key)
{
  if (key) {
    switch (state) {
      case INIT:
        if (key == '*') {
          state = START;
        }
        break;
      case START:
        if (key == code[0]) {
          state = M1;
        } else if (key != '*') {
          state = INIT;
        }
        break;
      case M1:
        if (key == code[1]) {
          state = M2;
        } else if (key == '*') {
          state = START;
        } else {
          state = INIT;
        }
        break;
      case M2:
        if (key == code[2]) {
          state = M3;
        } else if (key == '*') {
          state = START;
        } else  {
          state = INIT;
        }
        break;
      case M3:
        if (key == code[3]) {
          state = M4;
        } else if (key == '*') {
          state = START;
        } else {
          state = INIT;
        }
        break;
      case M4:
        if (key == '#') {
          enabled = true;
          Serial.println("ENABLED");
        } else if (key == '*') {
          enabled = false;
          Serial.println("DISABLED");
        }
        state = INIT;
        break;
    }
  }
}
    
        
       
        
       
  
  
byte rowPins[rows] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[cols] = {8, 7, 6}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );
void setup()
{
    Serial.begin(9600);
    state = INIT;
}

void loop()
{
  char key = keypad.getKey();
  if (key != NO_KEY){
    Serial.println(key);
    monitorKeypad(key);
  }
}

