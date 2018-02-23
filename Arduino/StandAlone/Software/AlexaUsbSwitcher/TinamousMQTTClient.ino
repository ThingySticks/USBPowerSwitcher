// ======================================
// Tinamous connectivity via MQTT
// ======================================
#include <MQTTClient.h>
#include <system.h>
#include "secrets.h"

// WiFi and MQTT settings in Secrets.h
// Be sure to use WiFiSSLClient for an SSL connection.
// for a non ssl (port 1883) use regular WiFiClient.
//WiFiClient networkClient; 
WiFiSSLClient networkClient; 

// https://github.com/256dpi/arduino-mqtt
// Specifying 4096 bytes buffer size
MQTTClient mqttClient(4096); 

// If we have been connected since powered up 
bool was_connected = false;
String senml = "";
unsigned long nextSendMeasurementsAt = 0;

// =================================================
// Setup the MQTT connection information
// =================================================
bool setupMqtt() {
  senml.reserve(4096);
  Serial.print("Connecting to Tinamous MQTT Server on port:");
  Serial.println(MQTT_SERVERPORT);
  Serial.print("Server:");
  Serial.println(MQTT_SERVER);
  mqttClient.begin(MQTT_SERVER, MQTT_SERVERPORT, networkClient);

  // Handle received messages.
  mqttClient.onMessage(messageReceived);

  connectToMqttServer();
}

// =================================================
// Connect to the MQTT server. This can be called 
// repeatedly and will be ignored if already connected
// =================================================
bool connectToMqttServer() { 
  if (mqttClient.connected()) {
    return true;
  }

  Serial.println("Reconnecting...."); 

  Serial.println("checking wifi..."); 
  if (WiFi.status() != WL_CONNECTED) { 
    Serial.print("WiFi Not Connected. Status: "); 
    Serial.print(WiFi.status(), HEX); 
    Serial.println();
    delay(10000);
    return false;
  } 
 
  Serial.println("Connecting to MQTT Server..."); 
  
  if (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) { 
    Serial.println("Failed to connect to MQTT Server."); 
    Serial.print("Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.print(", Return Code: "); 
    Serial.print(mqttClient.returnCode()); 
    Serial.println(); 

    if (mqttClient.lastError() == LWMQTT_CONNECTION_DENIED) {
      Serial.println("Access denied. Check your username and password. Username should be 'DeviceName.AccountName' e.g. MySensor.MyHome"); 
    }

    if (mqttClient.lastError() == -6) {
      Serial.println("Check your Arduino has the SSL Certificate loaded for Tinmaous.com"); 
      // Load the Firmware Updater sketch onto the Arduino.
      // Use the Tools -> WiFi Firmware Updater utility
    }

    // Wait 10s before it gets re-tried.
    delay(10000); 
    return false;
  } 
 
  Serial.println("Connected to Tinamous MQTT!"); 
 
  mqttClient.subscribe("/Tinamous/V1/Status.To/" DEVICE_USERNAME); 
  Serial.println("Subscribed to status.to topic."); 
  
  // Say Hi.
  publishTinamousStatus("Hello! Usb switch is now connected. @ me with help for help.");

  was_connected = true;
  return true;
} 

// =================================================
// Loop for mqtt processing.
// =================================================
void mqttLoop() {
  // Call anyway, does nothing if already connected.
  connectToMqttServer();
  
  mqttClient.loop(); 
}

// =================================================
// Publish a status message on the Tinamous timeline
// =================================================
void publishTinamousStatus(String message) {
  Serial.println("Status: " + message);
  mqttClient.publish("/Tinamous/V1/Status", message); 
}

// =================================================
// Publish measurements using the plain json format
// =================================================
void publishTinamousJsonMeasurements(String json) {
  Serial.println("Measurement: " + json);
  mqttClient.publish("/Tinamous/V1/Measurements/Json", json); 
}

// =================================================
// Publish measurements using senml json format
// =================================================
void publishTinamousSenMLMeasurements(String senml) {
  Serial.println("SenML Measurement: " + senml);
  mqttClient.publish("/Tinamous/V1/Measurements/SenML", senml); 
  if (mqttClient.lastError() != 0) {
    Serial.print("MQTT Error: "); 
    Serial.print(mqttClient.lastError()); 
    Serial.println(); 
  }
  Serial.println("Done.");
}
// =================================================
// Message received from the MQTT server
// =================================================
void messageReceived(String &topic, String &payload) { 
  Serial.println("Message from Tinamous on topic: " + topic + " - " + payload); 

  // If it starts with @ it's a status message to this device.
  if (payload.startsWith("@")) {
    payload.toLowerCase();
    if (handleStatusMessage(payload)) {
      Serial.println("@ me status message handled.");
      return;
    }
  } 

  // Didn't get an expected command, so the message was to us.
  // Publish a help message.
  publishTinamousStatus("Hello! Sorry I didn't understand the message. @ me with help for help.");
} 

// =================================================
// Message was a Status Post (probably Alexa)
// =================================================
bool handleStatusMessage(String payload) {
char buffer[25];

   // for 1..4 (maps to 0..3)
  for (int port = 0; port <MAX_USB_PORTS; port++) {
    sprintf(buffer, "turn on port-%01d", port + 1);
    
    if (payload.indexOf(buffer)> 0) {
      Serial.print("Turn on port ");
      Serial.println(port);
      setUsb(port, true);
      return true;
    }

    sprintf(buffer, "turn off port-%01d", port + 1);
    if (payload.indexOf(buffer)> 0) {
      Serial.print("Turn off port ");
      Serial.println(port);
      setUsb(port, false);
      return true;
    }
  }

  // No port specified, turn on all.
  if (payload.indexOf("turn on")> 0) {
    allOn();
    return true;
  }

  if (payload.indexOf("turn off")> 0) {
    allOff();
    return true;
  }

  if (payload.indexOf("help")> 0) {
    Serial.println("Sending help...");
    publishTinamousStatus(
    "Send a message to me (@" DEVICE_USERNAME ") then:" 
    "'Turn on Port-1' to turn on usb port 1,"
    "'Turn off Port-1' to turn off usb port 1,"
    "'Turn on' to turn on all usb ports,"
    "'Turn off' to turn off all usb ports,"
    " \n* Port number can be 1, 2, 3 or 4."
    );
    return true;
  }

  Serial.print("Unknown status message: ");
  Serial.println(payload);
  
  return false;
}
