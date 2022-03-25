/*
 * This example shows most common usecases of this module.
 * radio.init() should be called first in the setup.
 */

#include <TEA5767.h>

TEA5767 radio;
TEA5767State info;

void setup() {
  // Enable serial communication for printing radio info
  Serial.begin(9600);
  
  // Initialize radio
  radio.init();

  // Set radio settings
  radio.setStopLevel(11);
  radio.setMono(true);
  radio.setSNC(true);

  // Set radio to a frequency
  radio.setFrequency(90.7);

  // Add 100 kHz to current frequency
  info = radio.getInfo();
  radio.setFrequency(info.frequency + 0.1);

  // Search next station up
  radio.searchUp();

  // Print current frequency
  info = radio.getInfo();
  Serial.print("Frequency: ");
  Serial.println(info.frequency);
}

void loop() {}
