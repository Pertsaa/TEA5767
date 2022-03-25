#include <Arduino.h>
#include <Wire.h>
#include "TEA5767.h"

#define IC_ADR 0x60   // Last 7-bits used as address by the Wire library.

#define R1 0x00
#define R1_MUTE 0x80  // Mute (on/off)
#define R1_SM 0x40    // Search mode (on/off)

#define R2 0x01

#define R3 0x02
#define R3_SUD 0x80   // Search (up/down)
#define R3_SSL 0x60   // Search stop level
#define R3_HLSI 0x10  // High/Low side injection
#define R3_MS 0x08    // Mono to Stereo (forced mono/stereo)
#define R3_MR 0x04    // Left channel mute
#define R3_ML 0x02    // Right channel mute
#define R3_SWP1 0x01  // Software programmable port 1

#define R4 0x03
#define R4_SWP2 0x80  // Software programmable port 2
#define R4_STBY 0x40  // Standby (on/off)
#define R4_BL 0x20    // Band limits (FM Japanese/FM US-Europe)
#define R4_XTAL 0x10  // Clock frequency
#define R4_SMUTE 0x08 // Soft mute (on/off)
#define R4_HCC 0x04   // High Cut Control (on/off)
#define R4_SNC 0x02   // Stereo Noice Cancelling (on/off)
#define R4_SI 0x01    // Search indicator

#define R5 0x04

// ##################################
// ############ PUBLIC ##############
// ##################################

TEA5767::TEA5767() {}

void TEA5767::init() {
  Wire.begin();
  registers[R1] = 0x00 | R1_MUTE;
  registers[R2] = 0x00;
  registers[R3] = R3_SSL; // ADC High
  registers[R4] = R4_XTAL; // 32.768 kHz Clock Frequency
  registers[R5] = 0x00;
  sendStaged();
  setFrequency(91.0);
  setMute(false);
}

void TEA5767::setFrequency(float frequency) {
  if (!state.japanBand && (frequency < 87.5 || frequency > 108.0)) return; // US/Europe FM band
  if (state.japanBand && (frequency < 76.0 || frequency > 91.0)) return; // Japanese FM Band
  bool HLSI = getChannelHLSI(frequency);
  stageHLSI(HLSI);
  stageFrequency(frequency, HLSI);
  sendStaged();
}

void TEA5767::setStopLevel(uint8_t level) {
  state.stopLevel = level;
}

void TEA5767::searchUp() {
  bool wasMuted = state.muted;
  setMute(true);
  setFrequency(state.frequency + 0.1);
  readOutput();
  int wrapped = 0;
  while(!state.tuned && wrapped != 2) {
    if ((state.japanBand && state.frequency > 90.9) || (!state.japanBand && state.frequency > 107.9)) {
      setFrequency(state.japanBand ? 76.0 : 87.5);
      wrapped += 1;
    } else {
      setFrequency(state.frequency + 0.1);
    }
    readOutput();
  }
  if (!wasMuted) setMute(false);
}

void TEA5767::searchDown() {
  bool wasMuted = state.muted;
  setMute(true);
  setFrequency(state.frequency - 0.1);
  readOutput();
  int wrapped = 0;
  while(!state.tuned && wrapped != 2) {
    if ((state.japanBand && state.frequency < 76.1) || (!state.japanBand && state.frequency < 87.6)) {
      setFrequency(state.japanBand ? 91.0 : 108.0);
      wrapped += 1;
    } else {
      setFrequency(state.frequency - 0.1);
    }
    readOutput();
  }
  if (!wasMuted) setMute(false);
}

void TEA5767::setMute(bool value) {
  stageMute(value);
  sendStaged();
}

void TEA5767::setSoftMute(bool value) {
  stageSMUTE(value);
  sendStaged();
}

void TEA5767::setHCC(bool value) {
  stageHCC(value);
  sendStaged();
}

void TEA5767::setSNC(bool value) {
  stageSNC(value);
  sendStaged();
}

void TEA5767::setMono(bool value) {
  stageMS(value);
  sendStaged();
}

void TEA5767::setJapanBand(bool value){
  state.japanBand = value;
}

TEA5767State TEA5767::getInfo() {
  sendStaged(); // Write is needed so IF counter is updated.
  readOutput();
  return state;
}

// ##################################
// ########### STAGING ##############
// ##################################

void TEA5767::stageFrequency(float frequency, bool isHighSide) {
  state.frequency = frequency;
  uint16_t PLL;
  if (isHighSide) {
    PLL = (uint16_t)round(4*(frequency*1000+225)/32.768);
  } else {
    PLL = (uint16_t)round(4*(frequency*1000-225)/32.768);
  }
  registers[R1] &= 0xC0;
  registers[R1] |= (PLL >> 8 & ~0xC0);
  registers[R2] = PLL & 0XFF;
}

void TEA5767::stageMute(bool value) {
  state.muted = value;
  if (value) registers[R1] |= R1_MUTE;
  else registers[R1] &= ~R1_MUTE;
}

void TEA5767::stageHLSI(bool value) {
  if (value) registers[R3] |= R3_HLSI;
  else registers[R3] &= ~R3_HLSI;
}

void TEA5767::stageMS(bool value) {
  if (value) registers[R3] |= R3_MS;
  else registers[R3] &= ~R3_MS;
}

void TEA5767::stageSM(bool value) {
  if (value) registers[R1] |= R1_SM;
  else registers[R1] &= ~R1_SM;
}

void TEA5767::stageSUD(bool value) {
  if (value) registers[R3] |= R3_SUD;
  else registers[R3] &= ~R3_SUD;
}

void TEA5767::stageSMUTE(bool value) {
  if (value) registers[R4] |= R4_SMUTE;
  else registers[R4] &= ~R4_SMUTE;
}

void TEA5767::stageHCC(bool value) {
  if (value) registers[R4] |= R4_HCC;
  else registers[R4] &= ~R4_HCC;
}

void TEA5767::stageSNC(bool value) {
  if (value) registers[R4] |= R4_SNC;
  else registers[R4] &= ~R4_SNC;
}

// ##################################
// ####### I2C COMMUNICATION ########
// ##################################

void TEA5767::sendStaged() {
  Wire.beginTransmission(IC_ADR);
  for (int i = 0; i < sizeof registers / sizeof registers[0]; i++) {
    Wire.write(registers[i]);
  }
  Wire.endTransmission();
}

void TEA5767::readOutput(unsigned int waitTime = 30) {
  byte reading[5];
  delay(waitTime); // Wait for ADC and/or IF counter
  Wire.requestFrom(IC_ADR, 5);
  while(!Wire.available() == 5);

  reading[R1] = Wire.read();
  reading[R2] = Wire.read();
  reading[R3] = Wire.read();
  reading[R4] = Wire.read();
  reading[R5] = Wire.read();

  state.stereo = reading[R3] & 0x80;
  state.ifc = reading[R3] & 0x7F;
  state.level = reading[R4] >> 4;
  state.tuned = (state.level >= state.stopLevel) && ( 0x31 < state.ifc ) && (state.ifc < 0x3F );
}

// ##################################
// ####### UTILITY FUNCTIONS ########
// ##################################

bool TEA5767::getChannelHLSI(float frequency) {
  bool wasMuted = state.muted;

  stageMute(true);
  stageHLSI(true);
  stageFrequency(frequency + 0.45, true);
  sendStaged();
  readOutput(10);
  uint16_t levelHigh = state.level;

  stageFrequency(frequency - 0.45, true);
  if (!wasMuted) stageMute(false);
  sendStaged();
  readOutput(10);
  uint16_t levelLow = state.level;

  return levelHigh < levelLow;
}
