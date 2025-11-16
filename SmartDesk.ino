#include <LiquidCrystal.h>
#include <dht.h>

struct Time {
  unsigned long hour;
  unsigned long minute;
  unsigned long seconds;
};

#define SENS_TEMP_PIN 7
#define LIGHT_PIN A0
#define GREEN 13
#define BLUE 8

// Light configuration
const int BRIGHTNESS_THRESHOLD = 50;
const int BRIGHTNESS_MIN = 0;
const int BRIGHTNESS_MAX = 100;

int chk, lightValue, brightnessPercent;

byte coffee[8] = {
  0b00000,
  0b11110,
  0b10010,
  0b10010,
  0b10010,
  0b11110,
  0b01100,
  0b00000
};

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
dht DHT;

unsigned long startTime = 0;
unsigned long elapsedTime = 0;

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
void setLEDs(bool greenOn, bool blueOn) {
  digitalWrite(GREEN, greenOn ? HIGH : LOW);
  digitalWrite(BLUE, blueOn ? HIGH : LOW);
}

void updateLightControl() {
  // Read and process brightness
  lightValue = analogRead(LIGHT_PIN);

  Serial.print("Brightness %d", lightValue);
  brightnessPercent = map(lightValue, BRIGHTNESS_MIN, BRIGHTNESS_MAX, 0, 100);
  
  // Determine if lamp is needed
  isLampNeeded = (brightnessPercent < BRIGHTNESS_THRESHOLD);
  
  // Update LEDs based on mode and brightness
  if (!isLampNeeded) {
    setLEDs(false, false);
  } else {
    if (mode == WORK_MODE) {
      setLEDs(false, true);  // Blue for work
    } else {
      setLEDs(true, false);  // Green for break
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
  
  // Check if countdown finished, reset timer
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
  
  // Calculate remaining time
  unsigned long remaining = activeTimer - elapsedTime;
  
  countdown.hour = (remaining / 3600000);
  countdown.minute = (remaining / 60000) % 60;
  countdown.seconds = (remaining / 1000) % 60;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
0  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  
  startTime = millis();
  Serial.println("Arduino started!");
  Serial.println("Countdown timer started!");
  
  workMode();
}

void loop() {
  getCountdown();
  
  // Read temperature and humidity
  chk = DHT.read11(SENS_TEMP_PIN);
  
  // Update light control (now handles all light logic)
  updateLightControl();
  
  // Display on LCD - Line 1: Mode and Timer
  lcd.setCursor(0, 0);
  lcd.print(title);
  
  // Display countdown timer
  if (countdown.hour < 10 && countdown.hour > 0) lcd.print("0");
  if (countdown.hour > 0) lcd.print(countdown.hour); 
  if (countdown.hour > 0) lcd.print(":");
  if (countdown.minute < 10) lcd.print("0");
  lcd.print(countdown.minute);
  lcd.print(":");
  if (countdown.seconds < 10) lcd.print("0");
  lcd.print(countdown.seconds);
  lcd.print("     ");
  
  // Display on LCD - Line 2: Temperature and Brightness
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(DHT.temperature, 1);
  lcd.print("C ");
  lcd.print("B:");
  lcd.print(brightnessPercent);
  lcd.print("% ");
  
  delay(1000); 
}