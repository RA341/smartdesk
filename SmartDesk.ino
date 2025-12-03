#include <LiquidCrystal.h>
#include <DHT.h>
#include <IRremote.hpp>

struct Time {
  unsigned long hour;
  unsigned long minute;
  unsigned long seconds;
};

#define SENS_TEMP_PIN 8
#define LIGHT_PIN A0
#define RED 13
#define YELLOW 7

DHT dht(8, DHT11);

// Light configuration

int chk, lightValue, brightnessPercent;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


const int IR_RECEIVE_PIN = 9;

unsigned long startTime = 0;
unsigned long elapsedTime = 0;

unsigned long lastLCDUpdate = 0;
unsigned long lastSensorUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 1000;
const unsigned long SENSOR_UPDATE_INTERVAL = 2000;

const unsigned long SECONDS = 1000UL;
const unsigned long MINUTE = 60UL * SECONDS;
const unsigned long HOUR = 60UL * MINUTE;
const unsigned long workTimer = 30 * SECONDS;
const unsigned long breakTimer = 30 * SECONDS;
unsigned long activeTimer = workTimer;

const char breakkStr[] = "Break ";
const char workStr[] = "Work ";
char* title = workStr;

// Mode constants
enum Mode {
  WORK_MODE = 0,
  BREAK_MODE = 1
};

int mode = WORK_MODE;
bool isLampNeeded = false;

Time countdown;

// Improved light control functions
void setLEDs(bool REDOn, bool YELLOWOn) {
  // digitalWrite(RED, REDOn ? HIGH : LOW);
  // digitalWrite(YELLOW, YELLOWOn ? HIGH : LOW);

  digitalWrite(LED_BUILTIN, REDOn ? HIGH : LOW);
  digitalWrite(LED_BUILTIN, YELLOWOn ? HIGH : LOW);
}

// allow manual led control
bool ledOverride = false;
const int BRIGHTNESS_THRESHOLD = 50;

const int BRIGHTNESS_MIN = 60;
const int BRIGHTNESS_MAX = 400;

void updateLightControl() {
  lightValue = analogRead(LIGHT_PIN);
  brightnessPercent = map(lightValue, BRIGHTNESS_MIN, BRIGHTNESS_MAX, 0, 100);
  isLampNeeded = (brightnessPercent < BRIGHTNESS_THRESHOLD);
  Serial.print("Is lamp needed ");
  Serial.println(isLampNeeded);

  // Serial.print("Brightness: ");
  // Serial.println(lightValue);

  // Serial.print("Brightness %%: ");
  // Serial.println(brightnessPercent);
  
  // Update LEDs based on mode and brightness
  if (!isLampNeeded || !ledOverride) {
    setLEDs(false, false);
  } else {
    if (mode == WORK_MODE) {
      setLEDs(false, true);  // YELLOW for work
    } else {
      setLEDs(true, false);  // RED for break
    }
  }
}

void workMode() {
  Serial.println("Switching to work mode");
  mode = WORK_MODE;
  activeTimer = workTimer;
  title = workStr;
}

void breakMode() {
  Serial.println("Switching to break mode");
  mode = BREAK_MODE;
  activeTimer = breakTimer;
  title = breakkStr;
}

void getCountdown() {
  elapsedTime = millis() - startTime;
  
  if (elapsedTime >= activeTimer) {
    if (mode == WORK_MODE) {
      breakMode();
    } else {
      workMode();
    }
    startTime = millis();
    elapsedTime = 0;
    Serial.println("Timer reset! Starting new countdown");
  }
  
  unsigned long remaining = activeTimer - elapsedTime;
  
  countdown.hour = (remaining / 3600000);
  countdown.minute = (remaining / 60000) % 60;
  countdown.seconds = (remaining / 1000) % 60;
}

void updateLCD() {

  // Display countdown timer
  // Line 1
  lcd.setCursor(0, 0);
  lcd.print(title);

  if (countdown.hour < 10 && countdown.hour > 0) lcd.print("0");
  if (countdown.hour > 0) lcd.print(countdown.hour); 
  if (countdown.hour > 0) lcd.print(":");
  if (countdown.minute < 10) lcd.print("0");
  lcd.print(countdown.minute);
  lcd.print(":");
  if (countdown.seconds < 10) lcd.print("0");
  lcd.print(countdown.seconds);
  lcd.print("     ");
  
  // Line 2: Temperature and Brightness
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(dht.readTemperature(), 1);
  lcd.print("C ");
  lcd.print("B:");
  lcd.print(brightnessPercent);
  lcd.print("% ");
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);

  startTime = millis();
  lastLCDUpdate = millis();
  lastSensorUpdate = millis();
  
  Serial.println("Real time SmartDesk");
  
  workMode();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (IrReceiver.decode()) {
    uint32_t irCode = IrReceiver.decodedIRData.decodedRawData;
    
    Serial.print("IR Code: 0x");
    Serial.println(irCode, HEX);
    
    switch (irCode) {
    case 0xBA45FF00:
      ledOverride = !ledOverride;
      Serial.print("led overrride: ");
      Serial.println(ledOverride);
      break;
    default:
        Serial.println("Unknown button");
        break;
    }

    IrReceiver.resume();
  }

  getCountdown();
  
  if (currentMillis - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL) {
    lastSensorUpdate = currentMillis;
    // chk = dht.read11(SENS_TEMP_PIN);
    updateLightControl();
  }
  
  if (currentMillis - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = currentMillis;
    updateLCD();
  }

  delay(300);
}
