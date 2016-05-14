// doesn't work yet!

#include "sleep.h"
#include "timestamp.h"
#include "Wire.h"
#include "EEPROM.h"
#define DS3231_I2C_ADDRESS 0x68

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
const int enablePin = 3;  // the enable switch 
const int interruptPin = 2;
int sensorValue = 0;  // variable to store the value coming from the sensor
const int sirenPin = 4;
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void sleepNow()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, wakeUpNow, LOW);
  sleep_mode();
  sleep_disable();
  detachInterrupt(0);
}

void wakeUpNow() {}


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  // setDS3231time(30,42,21,2,29,2,16);
   pinMode(ledPin, OUTPUT);
   pinMode(sirenPin, OUTPUT);
   pinMode(enablePin, INPUT); 
   pinMode(interruptPin, INPUT); 
}

static void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write((uint8_t) 0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}



static void readTime(TimeStamp* ts)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write((uint8_t) 0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  ts->second = bcdToDec(Wire.read() & 0x7f);
  ts->minute = bcdToDec(Wire.read());
  ts->hour = bcdToDec(Wire.read() & 0x3f);
  ts->dayOfWeek = bcdToDec(Wire.read());
  ts->dayOfMonth = bcdToDec(Wire.read());
  ts->month = bcdToDec(Wire.read());
  ts->year = bcdToDec(Wire.read());
}

static const char* months[12] = {
  "Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ", "Jul ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec "};
void printTimeStamp(TimeStamp* ts)
{
  switch(ts->dayOfWeek){
  case 1:
    Serial.print("Sunday");
    break;
  case 2:
    Serial.print("Monday");
    break;
  case 3:
    Serial.print("Tuesday");
    break;
  case 4:
    Serial.print("Wednesday");
    break;
  case 5:
    Serial.print("Thursday");
    break;
  case 6:
    Serial.print("Friday");
    break;
  case 7:
    Serial.print("Saturday");
    break;
  }
  Serial.print(", ");

  Serial.print(months[ts->month - 1]);
  Serial.print(ts->dayOfMonth, DEC);
  Serial.print(", 20");
  Serial.print(ts->year, DEC);
  Serial.print(" at ");
  Serial.print(ts->hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (ts->minute<10)
  {
    Serial.print("0");
  }
  Serial.print(ts->minute, DEC);
  Serial.print(":");
  if (ts->second<10)
  {
    Serial.print("0");
  }
  Serial.print(ts->second, DEC);
  Serial.println("");


}

void clearEEPROM()
{
  Serial.print("please wait ... ");
  for (int i = 0 ; i < 1024 ; i++) {
    if (i % 128 == 0) {
      Serial.print(8 - i/128, DEC);
      Serial.print(" ");
    }
    EEPROM.write(i, 0);
  }
  Serial.println("\nOK!");
}

void dumpEEPROM()
{
  byte numEntries = EEPROM.read(0);
  TimeStamp ts;
  Serial.print(numEntries, DEC);
  if (numEntries == 1) {
    Serial.println(" entry recorded");
  } else {
    Serial.println(" entries recorded");
  }
  for (size_t i = 0; i < numEntries; i++) {
    byte * p = (byte*)&ts;
    for (size_t j = 0; j < sizeof(TimeStamp); j++) {
      uint32_t adrs = 1 + j + i*sizeof(TimeStamp);
      *p++ = EEPROM.read(adrs);
    }

    Serial.print("event ");
    Serial.print(i, DEC);
    Serial.print(": ");
    printTimeStamp(&ts);
  }
}

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

void impactDetected()
{
  testSiren();
  TimeStamp ts;
  size_t N = 10;
  byte * p = (byte*)&ts;
  readTime(&ts);
  byte numEntries = EEPROM.read(0);
  if (numEntries < N) {
    for (size_t i = 0; i < sizeof(TimeStamp); i++) {
      uint32_t adrs = i + 1 + sizeof(TimeStamp)*(numEntries);
      EEPROM.write(adrs, *p++);
    }
    EEPROM.write(0, numEntries+1);
    Serial.print("captured event on ");
    printTimeStamp(&ts);
  } else {
    Serial.println("shot record is full");
  }
}

void testSiren()
{ 
   digitalWrite(sirenPin, HIGH);
   delay(1000);
   Serial.println("OK!");
   digitalWrite(sirenPin, LOW);
}

void showInputs()
{
  bool enabled = digitalRead(enablePin);
  Serial.print("Enabled: ");
  Serial.println(enabled);
  bool x = digitalRead(interruptPin);
  Serial.print("wakesense: ");
  Serial.println(x);
  
}

void handleSerialPort()
{
  char ch = Serial.read();
  if (ch == 'T') {
   byte second = Serial.parseInt();
   byte minute = Serial.parseInt();
   byte hour = Serial.parseInt();
   byte dayOfWeek = Serial.parseInt();
   byte dayOfMonth = Serial.parseInt();
   byte month = Serial.parseInt();
   byte year = Serial.parseInt();
   setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
   Serial.println("time set");
 } else if (ch == 't') {
   TimeStamp ts;
   readTime(&ts);
   printTimeStamp(&ts);
 } else if (ch == 'C') {
   clearEEPROM();
 } else if (ch == 'd') {
   dumpEEPROM();   
 } else if (ch == 'x') {
   bool enabled = digitalRead(enablePin);
   if (enabled) {
     impactDetected();
   } else {
     Serial.println("not enabled!");
   }
 } else if (ch == 's') {
   Serial.println(analogRead(sensorPin), DEC);
 } else if (ch == 'e') {
   int sw = digitalRead(enablePin);
   Serial.println(sw, DEC);
 } else if (ch == 'h') {
   testSiren();
 } else if (ch == '?') {
   showInputs();
 } else if (ch != -1 && ch != '\n') {
   Serial.println("unrecognized command");
 }
}

bool ledOn = false;
const unsigned long normalOn = 250;
const unsigned long normalOff = 5000;
const unsigned long dataOn = 500;
const unsigned long dataOff = 500;
unsigned long startTime = 0;

void ledToggle(unsigned long on, unsigned long off)
{ 
  unsigned long t = millis() - startTime; 
  if (ledOn) {
    if (t > on) {
       ledOn = false;
       digitalWrite(ledPin, LOW);
       startTime = millis();
    }
  } else {
    if (t > off) {
      ledOn = true;
      digitalWrite(ledPin, HIGH);
      startTime = millis();
    }
  }
}

void updateLEDs()
{
  byte numEntries = EEPROM.read(0);
  if (numEntries > 0) {
    ledToggle(dataOn, dataOff);
  } else {
    ledToggle(normalOn, normalOff);
  }
}

void flashLed(int pin, int onTime, int offTime)
{
    digitalWrite(pin, HIGH);
    delay(onTime);
    digitalWrite(pin, LOW);
    delay(offTime);
}

void loop()
{
  bool enabled = digitalRead(enablePin);
  if (! enabled) {
    flashLed(ledPin, 100, 100);
  };
  
  handleSerialPort();
  if (enabled) {
    sensorValue = analogRead(sensorPin); 
    if (sensorValue > 900) {
      impactDetected();
      Serial.println(sensorValue, DEC);
      delay(500);
    }
  }
  int x;
  x = digitalRead(interruptPin); 
  if (x == 1) {
    Serial.println("going to sleep now");
    digitalWrite(ledPin, LOW);
    delay(2000);
     sleepNow();
     Serial.println("awake again!");
  }
  updateLEDs();
}

