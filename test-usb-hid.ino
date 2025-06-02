/*
 * ESP32-S3 USB HID Keyboard Test
 * This sketch tests the USB HID functionality
 * 
 * Upload this using Arduino IDE with:
 * - Board: ESP32S3 Dev Module
 * - USB Mode: USB-OTG
 * - USB CDC On Boot: Disabled
 * - USB DFU On Boot: Disabled
 */

#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

void setup() {
  // Start USB and Keyboard
  Keyboard.begin();
  USB.begin();
  
  // Wait for USB to initialize
  delay(2000);
  
  // Visual feedback - built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Blink 3 times to indicate ready
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void loop() {
  // Wait 5 seconds
  delay(5000);
  
  // Flash LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Type "Hello from ESP32-S3!"
  Keyboard.print("Hello from ESP32-S3! ");
  
  // Press Enter
  Keyboard.write(KEY_RETURN);
  
  // Wait another 5 seconds before repeating
  delay(5000);
}