#ifndef TEA5767_h
#define TEA5767_h

#include <Arduino.h>
#include <Wire.h>
#include "TEA5767State.h"

class TEA5767
{
public:
  TEA5767();

  void init();

  void setFrequency(float frequency);

  void setStopLevel(uint8_t level);
  
  void searchUp();
  void searchDown();

  void setMute(bool value);
  void setSoftMute(bool value);
  void setHCC(bool value);
  void setSNC(bool value);
  void setMono(bool value);
  void setJapanBand(bool value);

  TEA5767State getInfo();

private:
  void stageFrequency(float frequency, bool isHighSide);
  void stageMute(bool value);
  void stageHLSI(bool value);
  void stageMS(bool value);
  void stageSM(bool value);
  void stageSUD(bool value);
  void stageSMUTE(bool value);
  void stageHCC(bool value);
  void stageSNC(bool value);

  void sendStaged();
  void readOutput(unsigned int waitTime = 30);

  bool getChannelHLSI(float frequency);

  byte registers[5];
  TEA5767State state;
};

#endif
