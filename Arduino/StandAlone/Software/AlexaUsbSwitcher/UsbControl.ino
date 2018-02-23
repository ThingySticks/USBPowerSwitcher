// ======================================
// USB Port power control
// and indication.
// ======================================

// Determine if one or more of the ports is powered
bool hasPoweredPorts() {
  for (int channel = 0; channel < MAX_USB_PORTS; channel++) {
    if (usb_port_state[channel]) {
      return true;
    }
  }
  return false;
}

// Check if the USB devices have a fault (we can't
// tell which one in the 4 port 
bool checkFault(int channel) {
  if (!digitalRead(usb_fault_pin[channel])) {
    // 4 port has a single fault indicator, don't keep checking
    if (has_usb_fault) {
      return true;
    }
    
    Serial.print("FAULT! usb: ");
    Serial.println(channel);

    // Switch off the USB channel.
    setUsb(channel, false);
    
    // Make the LEDs red.
    SetLeds(channel, true, false);

    if (!has_usb_fault) {
      // TODO: Send the message once only!
      //String message = "Fault detected on USB Port ";
      //message = message + String(channel + 1);
      //publishTinamousStatus(message);
      has_usb_fault = true;
    }

    return true;
  }
  return false;
}

// Switch on ALL of the USB ports
void allOn() {
  for(int channel =0; channel < MAX_USB_PORTS; channel++) {
    setUsb(channel, true);
  }
}

// Switch off ALL of the USB ports
void allOff() {
  for(int channel =0; channel < MAX_USB_PORTS; channel++) {
    setUsb(channel, true);
  }
}

// ================================================
// Set the USB output state
// ================================================
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
  
  // force a send of the current/voltage after a second or few.
  sentNextPublishAt(1);

  // If the channel mode is to cycle off then on for 
  // an off signal, switch the channel back on after a few seconds delay.
  if (!on && usb_power_mode[channel] == 1) {
    // Schedule the channel to come back on in n seconds.
    usb_power_switch_on_at[channel] = millis() + 5000;
  }
}

// ================================================
// Set the state leds to indicate channel on/off
// They are not connected to the USB switch so 
// can be run independantly.
// ================================================
void SetLeds(int channel, bool on, bool green) {
  if (!on) {
    digitalWrite(led_pin[channel][0], LOW);
    digitalWrite(led_pin[channel][1], LOW);
    return;
  }

  if (led_pin[channel][0] == led_pin[channel][1]) {
    // 4 Port uses single color LED.
    // We don't have a way to show faultes (other than flashing LEDs).
    digitalWrite(led_pin[channel][0], HIGH);
  } else {
    // 2 Port uses Bi-color LED and individual faults
    // so show red/green based on status.
    if (green) {
      digitalWrite(led_pin[channel][0], HIGH);
      digitalWrite(led_pin[channel][1], LOW);
    } else {
      digitalWrite(led_pin[channel][0], LOW);
      digitalWrite(led_pin[channel][1], HIGH);
    }
  }
}
