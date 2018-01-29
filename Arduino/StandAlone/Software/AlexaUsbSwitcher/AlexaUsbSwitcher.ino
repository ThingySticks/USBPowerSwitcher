#include <MQTTClient.h>
#include <system.h>

#include <WiFi101.h>

// Current.
#include <Adafruit_INA219.h>

#include "Secrets.h"

#define MAX_USB_PORTS 2

// LED 1: 0&1, LED 2, A5 & A6
int led_pin[MAX_USB_PORTS][2] = {{A5, A6},{0,1}};
int switch_pin[] = {A0, A1};

// Usb switch B channel -> pin 5 == USB1
int usb_enable_pin[] = {2, 5};
int usb_fault_pin[] = {3, 4};

Adafruit_INA219 ina219;

volatile bool has_usb_fault = false;

bool usb_port_state[] = {false, false};

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;

float max_current_mA =0;
float max_busvoltage = 0;
float min_busvoltage = 20;

// MQTT publishing.
// Probably want it quicker when powering devices.
// or slower when not!
int updateIntervalSeconds = 60;
unsigned long nextMessageSendAt = 0;

void setup() {
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
 
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Give us a small delay to open the serial monitor...
  delay(5000);

  ina219.begin();

  // We want 5V (0r about) at 2A.
  //void setCalibration_32V_2A(void);
  //void setCalibration_32V_1A(void);

  // If we know the load will be less than 400mA we can get 
  // greater accuracy...
  //void setCalibration_16V_400mA(void);

  for (int channel=0; channel<MAX_USB_PORTS; channel++) {
    setupChannel(channel);
  }

  setupWiFi();

  setupMqtt();

  // Give it a few seconds to warm up!
  sentNextPublishAt(5);

  Serial.println("Alexa Enabled USB Power Switched v0.2 Setup Complete");
}

void setupChannel(int channel) {
  Serial.print("Setup channel");
  Serial.println(channel);
  // Setup the LEDs.
  pinMode(led_pin[channel][0], OUTPUT);
  pinMode(led_pin[channel][1], OUTPUT);
  digitalWrite(led_pin[channel][0], LOW);
  digitalWrite(led_pin[channel][1], LOW);

  // Setup the Switches
  pinMode(switch_pin[channel], INPUT_PULLUP);

  pinMode(usb_enable_pin[channel], OUTPUT);
  digitalWrite(usb_enable_pin[channel], HIGH);
  pinMode(usb_fault_pin[channel], INPUT);

  // NB: A0 (Button S1) doesn't support interrupts.
  //attachInterrupt(switch_pin[channel], switch_pressed, RISING); 
  // USB Fault are active low. 
  attachInterrupt(usb_fault_pin[channel], usb_fault, FALLING);
}

void loop() {

  digitalWrite(6, LOW);

  if (has_usb_fault) {
    Serial.println("USB Fault...");
    has_usb_fault = false;
  }

  for (int channel = 0; channel < MAX_USB_PORTS; channel++) {
    checkFault(channel);
    checkSwitchPressed(channel);
  }

  // Check the current.
  readCurrent();
  
  doDelay();

  mqttLoop();

  sendStatus();
}

void doDelay() {
  if (hasPoweredPorts()) {
    delay(100);
    return;
  }
  // Blinky only when in slow mode so we don't have to delay the
  // reads for the user to see the blinkyness.
  // No ports on, don't update so often.
  delay(100);
  digitalWrite(6, HIGH);
  delay(100);
}

bool hasPoweredPorts() {
  for (int channel = 0; channel < MAX_USB_PORTS; channel++) {
    if (usb_port_state[channel]) {
      return true;
    }
  }
  return false;
}

void checkSwitchPressed(int channel) {
  if (isSwitchPressed(channel)) {
    // Toggle the USB power
    setUsb(channel, !usb_port_state[channel]);
  }
}

bool isSwitchPressed(int channel) {
  // Switch is active low.

  // Debounce delay to ensure the switch is properly pressed.
  // Switch is active low. false = pressed.
  if (digitalRead(switch_pin[channel])) {
    return false;
  }

  // wait to see if it's still pressed after 0.2s
  delay(50);

  if (!digitalRead(switch_pin[channel])) {
    // Switch pressed.
    //while (!digitalRead(switch_pin[channel])) {
      // wait for it to be released.
      //Serial.println("Release the switch!");
      //delay(1);
    //}//;
    // Little debounce...
    delay(100);
    return true;
  }
  return false;
}

void readCurrent() {
  
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  //power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  if (current_mA > max_current_mA) {
    max_current_mA = current_mA;
  }

  if (busvoltage > max_busvoltage) {
    max_busvoltage = busvoltage;
  }

  if (busvoltage < min_busvoltage) {
    min_busvoltage = busvoltage;
  }
  
  printPowerSkinny();
}

void printPowerWide() {
  Serial.print("Bus Voltage:\t"); 
  Serial.print(busvoltage); 
  Serial.print(" V\t");
  
  Serial.print("Current:    \t"); 
  Serial.print(current_mA); 
  Serial.print(" mA\t");
  
  Serial.print("Max Current:\t"); 
  Serial.print(max_current_mA); 
  Serial.print(" mA\t");
  
  Serial.print("Min Voltage:\t");
  Serial.print(min_busvoltage); 
  Serial.print(" V\t");
  
  Serial.print("Max Voltage:\t");
  Serial.print(max_busvoltage); 
  Serial.print(" V\t");

  Serial.println();
}

void printPowerSkinny() {
  return;
  Serial.print(busvoltage); 
  Serial.print("\t");
  
  Serial.print(current_mA); 
  Serial.print("\t\t");
  
  Serial.print(max_current_mA); 
  Serial.print("\t\t");
  
  Serial.print(min_busvoltage); 
  Serial.print("\t");
  
  Serial.print(max_busvoltage); 
  Serial.print("\t");

  Serial.println();
}

void checkFault(int channel) {
  if (!digitalRead(usb_fault_pin[channel])) {
    Serial.print("FAULT! usb: ");
    Serial.println(channel);

    // Switch off the USB channel.
    setUsb(channel, false);
    
    // Make the LEDs red.
    SetLeds(channel, true, false);

    String message = "Fault detected on USB Port ";
    message = message + String(channel + 1);
    publishTinamousStatus(message);
  }
}

void setUsb(int channel, bool on) {
  Serial.print("Switch USB channel ");
  Serial.print(channel);
  Serial.print(" to ");
  Serial.println(on);
  
  if (on) {
    resetStats();
    SetLeds(channel, on, true);
    digitalWrite(usb_enable_pin[channel], LOW);
  } else {
    SetLeds(channel, on, true);
    digitalWrite(usb_enable_pin[channel], HIGH);
  }

  usb_port_state[channel] = on;

  String message = "USB Port ";
  message = message + String(channel + 1);
  message = message + " is now ";
  if (on) {
    message = message + "enabled.";
  } else {
    message = message + "disabled.";
  }
  publishTinamousStatus(message);
  // force a send of the current/voltage after a few seconds.
  sentNextPublishAt(3);
}

void SetLeds(int channel, bool on, bool green) {
  if (!on) {
    digitalWrite(led_pin[channel][0], LOW);
    digitalWrite(led_pin[channel][1], LOW);
    return;
  }

  if (green) {
    digitalWrite(led_pin[channel][0], HIGH);
    digitalWrite(led_pin[channel][1], LOW);
  } else {
    digitalWrite(led_pin[channel][0], LOW);
    digitalWrite(led_pin[channel][1], HIGH);
  }
}

void resetStats() {
  max_current_mA =0;
  max_busvoltage = 0;
  min_busvoltage = 20;
}

void sentNextPublishAt(int secondsTime) {
  nextMessageSendAt = millis() + (secondsTime * 1000);
}

// ============================================
// IoT Bits...
// ============================================
void sendStatus() {
  if (millis() > nextMessageSendAt) { 
    Serial.println("------------------------"); 
    Serial.println("MQTT publish measurements"); 

    sentNextPublishAt(updateIntervalSeconds);
    
    // And do one as senml...
    String senml = "{'e':[";
    // Voltage
    senml = senml + "{'n':'busVoltage'";
    senml = senml + ", 'v':";
    senml = senml + String(busvoltage);
    senml = senml + ", 'u':'V'}";
    
    // Current
    senml = senml + ",{'n':'Current'";
    senml = senml + ", 'v':";
    senml = senml + String(current_mA);
    senml = senml + ", 'u':'mA'}";
    
    // Max current
    senml = senml + ",{'n':'MaxCurrent'";
    senml = senml + ", 'v':'";
    senml = senml + String(max_current_mA);
    senml = senml + "', 'u':'mA'}";
    
    // mAh consumed...
    senml = senml + ",{'n':'mAH'";
    senml = senml + ", 'v':";
    senml = senml + String(0); // TODO!
    senml = senml + ", 'u':'mAH'}";

    for (int channel=0; channel<MAX_USB_PORTS; channel++) {
      senml = senml + ",{'n':'Port-";
      senml = senml + String(channel+1);
      senml = senml + "', 'v':";
      senml = senml + String(usb_port_state[channel]);
      senml = senml + ", 'u':'mAH'}";
    }
        
    senml = senml +  "]}";
    publishTinamousSenMLMeasurements(senml);
    Serial.println("------------------------"); 
    Serial.println(); 
  }
}

// ============================================
// ISR
// ============================================
void usb_fault() {
  has_usb_fault = true;
}

