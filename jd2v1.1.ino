#include <Wire.h>
#include <HX711_ADC.h>
#include <HT16K33.h>

#define HX711_DOUT 4
#define HX711_SCK 5
#define SPEAKER 2
#define BUTTON_INC 6
#define BUTTON_DEC 7
#define SWITCH_HIGH 8
#define SWITCH_MEDIUM 9
#define SWITCH_LOW 10

HT16K33 display(0x70);  // I2C Address for HT16K33
HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);

// Variables
int alarmTime = 300;  // Default: 5 minutes
bool alarmTriggered = false;
const float WEIGHT_THRESHOLD = 5.0; // Load threshold to activate alarm

void setup() {
    Serial.begin(115200);
    pinMode(SPEAKER, OUTPUT);
    pinMode(BUTTON_INC, INPUT_PULLUP);
    pinMode(BUTTON_DEC, INPUT_PULLUP);
    pinMode(SWITCH_HIGH, INPUT_PULLUP);
    pinMode(SWITCH_MEDIUM, INPUT_PULLUP);
    pinMode(SWITCH_LOW, INPUT_PULLUP);

    Wire.begin();
    display.begin();  // Initialize HT16K33
    display.displayOn();  // Turn on display
    display.displayOff(); // Turn display off to reset
    display.displayOn();  // Turn display back on
    display.setBrightness(7);  // Set medium brightness

    LoadCell.begin();
    LoadCell.start(2000);
    LoadCell.setCalFactor(1000.0); // Adjust based on calibration

    Serial.println("Load Cell Initialized...");
}

void loop() {
    LoadCell.update();
    float weight = LoadCell.getData();

    // Print weight to Serial Monitor
    Serial.print("Weight: ");
    Serial.print(weight);
    Serial.print(" g - ");
    if (weight > WEIGHT_THRESHOLD) {
        Serial.println("Load Detected!");
    } else {
        Serial.println("No Load.");
    }

    // Start the alarm countdown when weight is detected
    if (weight > WEIGHT_THRESHOLD && !alarmTriggered) {
        alarmTriggered = true;
        startAlarmCountdown();
    }

    handleButtonInput();
    updateDisplay();
    adjustBrightness();

    if (alarmTime == 0) {
        soundAlarm();
    }
}

void handleButtonInput() {
    if (digitalRead(BUTTON_INC) == LOW) {
        alarmTime += 300; // +5 minutes
        if (alarmTime > 5700) {  // 95 minutes (5700 seconds)
            alarmTime = 0;  // Reset to 00:00
        }
        delay(300);
    }
    if (digitalRead(BUTTON_DEC) == LOW) {
        alarmTime = max(0, alarmTime - 300);
        delay(300);
    }
}

void updateDisplay() {
    int minutes = alarmTime / 60;
    int seconds = alarmTime % 60;

    int timeValue = (minutes * 100) + seconds;  // Convert MM:SS to 4-digit format
    display.displayInt(timeValue);  // Corrected function call
}

void adjustBrightness() {
    if (digitalRead(SWITCH_HIGH) == LOW) {
        display.setBrightness(15);  // Bright
    } else if (digitalRead(SWITCH_MEDIUM) == LOW) {
        display.setBrightness(8);   // Medium
    } else if (digitalRead(SWITCH_LOW) == LOW) {
        display.setBrightness(3);   // Low
    }
}

void startAlarmCountdown() {
    while (alarmTime > 0) {
        delay(1000);
        alarmTime--;
        updateDisplay();
    }
}

void soundAlarm() {
    Serial.println("ALARM! 440Hz Tone!");
    for (int i = 0; i < 5; i++) {
        digitalWrite(SPEAKER, HIGH);
        delayMicroseconds(1000000 / 440);
        digitalWrite(SPEAKER, LOW);
        delayMicroseconds(1000000 / 440);
    }
}
