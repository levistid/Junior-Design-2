#include <Wire.h>
#include <HX711_ADC.h>
#include <HT16K33.h>

#define HX711_DOUT 4  // Load Cell Data
#define HX711_SCK 5   // Load Cell Clock
#define SPEAKER 2
#define BUTTON_INC 6  // Increment time
#define BUTTON_DEC 7  // Decrement time
#define SWITCH_HIGH 8   // Top Position - High Brightness
#define SWITCH_MEDIUM 9 // Middle Position - Medium Brightness (Default)
#define SWITCH_LOW 10   // Bottom Position - Low Brightness

HT16K33 display(0x70);  // I2C Address for HT16K33
HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);

// Variables
int alarmTime = 300;  // Default: 5 minutes
bool countdownRunning = false;
const float WEIGHT_THRESHOLD = 5.0;  // Minimum weight to start timer
unsigned long previousMillis = 0;  // Timer tracking
const long interval = 1000;  // 1-second interval
bool weightPresent = false;

// Brightness levels
enum BrightnessLevel { BRIGHTNESS_LOW, BRIGHTNESS_MEDIUM, BRIGHTNESS_HIGH };
BrightnessLevel brightness = BRIGHTNESS_MEDIUM;  // Default brightness

void setup() {
    Serial.begin(115200);
    pinMode(SPEAKER, OUTPUT);
    pinMode(BUTTON_INC, INPUT_PULLUP);
    pinMode(BUTTON_DEC, INPUT_PULLUP);
    pinMode(SWITCH_HIGH, INPUT_PULLUP);
    pinMode(SWITCH_MEDIUM, INPUT_PULLUP);
    pinMode(SWITCH_LOW, INPUT_PULLUP);

    Wire.begin();
    display.begin();
    display.displayOn();
    
    LoadCell.begin();
    LoadCell.start(2000);
    LoadCell.setCalFactor(1000.0); // Adjust this value based on calibration

    updateDisplayBrightness();  // Set initial brightness

    Serial.println("System Initialized. Place weight to start countdown.");
}

void loop() {
    LoadCell.update();
    float weight = LoadCell.getData();

    // Detect weight presence
    bool newWeightPresent = (weight > WEIGHT_THRESHOLD);

    if (newWeightPresent && !weightPresent) {
        Serial.println("Weight Detected! Timer Resuming...");
        countdownRunning = true;
    } 
    else if (!newWeightPresent && weightPresent) {
        Serial.println("Weight Removed! Timer Stopping...");
        countdownRunning = false;
    }

    weightPresent = newWeightPresent;  // Update weight status

    handleButtonInput();
    updateBrightnessFromSwitch();  // Check switch position and update brightness

    if (countdownRunning && alarmTime > 0) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            alarmTime--;
            updateDisplay();
        }

        if (alarmTime == 0) {
            soundAlarm();
            countdownRunning = false;
            Serial.println("Timer Complete! Reset weight or press buttons to restart.");
        }
    }
}

void handleButtonInput() {
    if (digitalRead(BUTTON_INC) == LOW) {
        alarmTime += 300; // +5 minutes
        if (alarmTime > 5700) {  // 95 minutes max
            alarmTime = 0;  // Reset to 00:00
        }
        Serial.print("Timer Adjusted: ");
        Serial.print(alarmTime / 60);
        Serial.println(" min");
        delay(200);
    }

    if (digitalRead(BUTTON_DEC) == LOW) {
        alarmTime = max(0, alarmTime - 300); // -5 minutes
        Serial.print("Timer Adjusted: ");
        Serial.print(alarmTime / 60);
        Serial.println(" min");
        delay(200);
    }
}

void updateBrightnessFromSwitch() {
    if (digitalRead(SWITCH_HIGH) == LOW) {
        brightness = BRIGHTNESS_HIGH;
    } else if (digitalRead(SWITCH_LOW) == LOW) {
        brightness = BRIGHTNESS_LOW;
    } else {
        brightness = BRIGHTNESS_MEDIUM;  // If neither HIGH nor LOW is triggered, default to MEDIUM
    }
    
    updateDisplayBrightness();
}

void updateDisplayBrightness() {
    int brightnessValue;
    switch (brightness) {
        case BRIGHTNESS_LOW: 
            brightnessValue = 3; 
            Serial.println("Brightness: LOW"); 
            break;
        case BRIGHTNESS_MEDIUM: 
            brightnessValue = 8; 
            Serial.println("Brightness: MEDIUM"); 
            break;
        case BRIGHTNESS_HIGH: 
            brightnessValue = 15; 
            Serial.println("Brightness: HIGH"); 
            break;
    }
    display.setBrightness(brightnessValue);
}

void updateDisplay() {
    int minutes = alarmTime / 60;
    int seconds = alarmTime % 60;
    int timeValue = (minutes * 100) + seconds;

    display.displayInt(timeValue);  // Update HT16K33 7-segment display
    Serial.print("Time Remaining: ");
    Serial.print(alarmTime);
    Serial.println(" sec");
}

void soundAlarm() {
    Serial.println("ALARM! 440Hz Tone!");

    int beepCount = 5;
    int beepDuration = 300;
    int pauseDuration = 200;

    for (int i = 0; i < beepCount; i++) {
        tone(SPEAKER, 440);
        delay(beepDuration);
        noTone(SPEAKER);
        delay(pauseDuration);
    }
}
