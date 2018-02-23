#include <MQTTClient.h>
#include <system.h>
#include <WiFi101.h>
#include <Adafruit_INA219.h>
// Provide your own Secrets.h with WiFi and Tinamous definitions in it.
#include "Secrets.h"

// ================================
/*
#define MAX_USB_PORTS 2

// 2 Port board.
int led_pin[MAX_USB_PORTS][2] = {{A5, A6},{0,1}};
int switch_pin[] = {A0, A1};

// Usb switch B channel -> pin 5 == USB1
int usb_enable_pin[] = {2, 5};
int usb_fault_pin[] = {3, 4};
bool usb_port_state[] = {false, false};
bool usb_power_mode[] = {0, 0, 1, 1};
*/

// ---------------------------------

// 4 Port board.
#define MAX_USB_PORTS 4

// The 2  port board has bi-color LEDs, 4 port doesn't so use same pin......
// LEDs: TX, Rx, D7, D6 (1-4)
int led_pin[MAX_USB_PORTS][2] = {{14, 14}, {13,13}, {7,7}, {6,6}};
int switch_pin[] = {A0, A1, A2, A3};

// USB Enable pins, D1, D3, D4, D5 (not in that order)
int usb_enable_pin[] = {5, 4, 1, 3};
// Single fault pin (D2)
int usb_fault_pin[] = {2, 2, 2, 2};

bool usb_port_state[] = {false, false, false, false};

// Mode: 0 - Switch off as requested.
// Mode: 1 - Switch off, then on after a delay (useful for LED lights that have touch controls).
int usb_power_mode[] = {1, 0, 0, 0};

// ================================

// Automation options.
// time (millis) that the USB port should be switched on/off at.
// zero indicates ignore.
unsigned long usb_power_switch_on_at[] = {0, 0, 0, 0};

// Set > 0 to get processed on first loop.
// Side effect is switching them off that those in smart power 
// mode will schedule to come back on again).
unsigned long usb_power_switch_off_at[] = {1, 1, 1, 1};

// Current monitor INA219
Adafruit_INA219 ina219;

// Flag to indicate if one or more USB ports has a fault.
bool has_usb_fault = false;

// Measured power .
float shunt_voltage = 0;
float bus_voltage = 0;
float current_mA = 0;
float load_voltage = 0;
float power_mW = 0;

// Min/max 
float max_current_mA =0;
float max_busvoltage = 0;
float min_busvoltage = 20;

// track power usage.
unsigned long last_measurement_time = 0;

bool power_failed = false;
bool over_current = false;

// MQTT publishing.
// Probably want it quicker when powering devices.
// or slower when not!
int update_interval_seconds = 20;
unsigned long next_message_send_at = 0;

// =================================================
// Main setup entry point
// =================================================
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  for (int channel=0; channel<MAX_USB_PORTS; channel++) {
    setupChannel(channel);
  }

  delay(2000);
  showSetupStageCompleted(1);
 
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  // Give us a small delay to open the serial monitor...
  delay(3000);
  showSetupStageCompleted(2);


  // Setup the current sensor.
  ina219.begin();

  setupWiFi();
  showSetupStageCompleted(3);

  setupMqtt();
  showSetupStageCompleted(4);

  // Give it a few seconds to warm up!
  sentNextPublishAt(5);

  Serial.println("Alexa Enabled USB Power Switched v0.4 Setup Complete");
}

// Setup the IO pins for the usb port.
void setupChannel(int channel) {
  
  // Setup the LEDs.
  pinMode(led_pin[channel][0], OUTPUT);
  digitalWrite(led_pin[channel][0], LOW);

  if (led_pin[channel][0] != led_pin[channel][1]) {
    pinMode(led_pin[channel][1], OUTPUT);
    digitalWrite(led_pin[channel][1], LOW);
  }

  // Setup the Switches
  pinMode(switch_pin[channel], INPUT_PULLUP);

  pinMode(usb_enable_pin[channel], OUTPUT);
  digitalWrite(usb_enable_pin[channel], HIGH);

  // This is the same pin on all channels for the 4 channel device.
  pinMode(usb_fault_pin[channel], INPUT_PULLUP);
}

// Show setup state 1..4 complete 
// can't do steps 3 & 4 on 2 port board.
void showSetupStageCompleted(int stage) {
  if (stage > MAX_USB_PORTS) {
    return;
  }

  // Light up the LED (1..4) based on the setup progress.
  SetLeds(stage-1 , true, true);
}


// ===================================================
// Main Loop
// ===================================================
void loop() {

  // 4 port device, Debug LED is port 4 LED.
  // if any port 4 is on, then don't switch off the LED.
  // with all 4 ports off, the debug LED can blink as it likes...
  if (!hasPoweredPorts()) {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Check for a fault.
  bool faulted = false;
  for (int channel = 0; channel < MAX_USB_PORTS; channel++) {
    if (checkFault(channel)) {
      faulted = true;
    }

    // Lets us clear the has_usb_fault if ALL of the channels are not faulted.
    has_usb_fault = faulted;
    checkSwitchPressed(channel);
  }

  // Do any automation (e.g. switch on at a certain time)
  runAutomation();

  // Check the current, voltage and all that.
  readCurrent();

  // Check for a power fail.
  checkPowerSupply();
  
  doDelay();

  mqttLoop();

  sendStatus();
}

void doDelay() {
  if (hasPoweredPorts()) {
    delay(100);
    return;
  }

  // Debug only. LED is on Port 4 LED so is shown to the user when 
  // all ports are off.
  //
  // Blinky only when in slow mode so we don't have to delay the
  // reads for the user to see the blinkyness.
  // No ports on, don't update so often.
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}

// Check fr user pressing the switch to turn on a port.
void checkSwitchPressed(int channel) {
  if (isSwitchPressed(channel)) {
    // Toggle the USB power
    setUsb(channel, !usb_port_state[channel]);
  }
}

// Switch press logic to debounce the switch
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
    // Little debounce...
    delay(100);
    return true;
  }
  return false;
}

// Read the INA219 (Voltage, Current)
void readCurrent() {
  
  shunt_voltage = ina219.getShuntVoltage_mV();
  bus_voltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  //power_mW = ina219.getPower_mW();
  load_voltage = bus_voltage + (shunt_voltage / 1000);
  // record the measurement time so we can compute 
  // power*time usage.
  last_measurement_time = millis();

  if (current_mA > max_current_mA) {
    max_current_mA = current_mA;
  }

  if (bus_voltage > max_busvoltage) {
    max_busvoltage = bus_voltage;
  }

  if (bus_voltage < min_busvoltage) {
    min_busvoltage = bus_voltage;
  }
  
  printPowerSkinny();
}

// Check we have power input, it's not got a significant voltage drop
// and we're not overloading the input.
void checkPowerSupply() {
  // Higher bus voltage for restored to stop it 
  // going back and forth at 4.19...
  if (power_failed && bus_voltage > 4.5) {
    Serial.print("External power restored. Bus voltage: "); 
    Serial.print(bus_voltage); 
    Serial.println(" V");

    publishTinamousStatus("External power has been restored.");
    power_failed = false;
  }
  
  if (bus_voltage < 4.2 && !power_failed) {
    Serial.print("External power failed! Bus voltage: "); 
    Serial.print(bus_voltage); 
    Serial.println(" V.");

    publishTinamousStatus("External power lost!");
    power_failed = true;
  }

  if (over_current && current_mA < 1500) {
    publishTinamousStatus("Current fault cleared!");
    over_current = false;
  }

  if (current_mA > 2000 && !over_current) {
    // Overload!
    publishTinamousStatus("Over Current!");
    over_current = true;
  } 
}

void printPowerWide() {
  Serial.print("Bus Voltage:\t"); 
  Serial.print(bus_voltage); 
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
  Serial.print(bus_voltage); 
  Serial.print("\t");
  
  Serial.print(current_mA); 
  Serial.print("\t\t");
  
  Serial.print(max_current_mA); 
  Serial.print("\t\t");
  
  Serial.print(min_busvoltage); 
  Serial.print("\t");
  
  Serial.print(max_busvoltage); 
  Serial.print("\t");

  Serial.print("[");
  for (int channel=0; channel<MAX_USB_PORTS; channel++) {
    Serial.print(usb_port_state[channel] ? "1" : "0" ); 
    Serial.print("\t");
  }
  Serial.print("]\t");

  Serial.print(millis()); 
  Serial.print("\t");

  Serial.println();
}

void resetStats() {
  max_current_mA =0;
  max_busvoltage = 0;
  min_busvoltage = 20;
}

void sentNextPublishAt(int secondsTime) {
  next_message_send_at = millis() + (secondsTime * 1000);
}

// ============================================
// Send the device status.
// ============================================
void sendStatus() {
  if (millis() > next_message_send_at) { 
    Serial.println("------------------------"); 
    Serial.println("MQTT publish measurements"); 

    if (power_failed) {
      // reduce how often we send when the power
      // has failed to preserve battery power.
      sentNextPublishAt(update_interval_seconds * 10);
    } else {
       sentNextPublishAt(update_interval_seconds);
    }
    
    // And do one as senml...
    String senml = "{'e':[";
    // Voltage
    senml = senml + "{'n':'busVoltage'";
    senml = senml + ", 'v':";
    senml = senml + String(bus_voltage);
    senml = senml + ", 'u':'V'}";

    // Max voltage
    senml = senml + ",{'n':'maxBusVoltage'";
    senml = senml + ", 'v':";
    senml = senml + String(max_busvoltage);
    senml = senml + ", 'u':'V'}";

    // Min voltage
    senml = senml + ",{'n':'minBusVoltage'";
    senml = senml + ", 'v':";
    senml = senml + String(min_busvoltage);
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
    senml = senml + ",{'n':'mAh'";
    senml = senml + ", 'v':";
    senml = senml + String(0); // TODO!
    senml = senml + ", 'u':'mAh'}";
   
    senml = senml + ",{'n':'powerState";
    senml = senml + "', 'bv':";
    if (isPowered()) {
      senml = senml +  "true";
    } else {
      senml = senml +  "false";
    }
    senml = senml + "}";
  

    for (int channel=0; channel<MAX_USB_PORTS; channel++) {
      senml = senml + ",{'n':'Port-";
      senml = senml + String(channel+1);
      senml = senml + "', 'v':";
      senml = senml + String(usb_port_state[channel]);
      senml = senml + "}";
    }
        
    senml = senml +  "]}";
    publishTinamousSenMLMeasurements(senml);
    Serial.println("------------------------"); 
    Serial.println(); 
  }
}

bool isPowered() {
  return !power_failed;
}

void runAutomation() {
  for (int channel =0; channel < MAX_USB_PORTS; channel++) {
    // Check to see if the port should be powered on automatically
    unsigned long onAt = usb_power_switch_on_at[channel];
    if (onAt > 0 && onAt < millis()) {
      Serial.print("Auto on for port: ");
      Serial.println(channel + 1);
      
      setUsb(channel, true);
      usb_power_switch_on_at[channel] = 0;
    }

    // Check to see if the port should be powered off automatically
    unsigned long offAt = usb_power_switch_off_at[channel]; 
    if (offAt > 0 && offAt < millis()) {
      Serial.print("Auto off for port: ");
      Serial.println(channel + 1);
      
      setUsb(channel, false);
      usb_power_switch_off_at[channel] = 0;
    }
  }
}

