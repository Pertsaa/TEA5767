#ifndef TEA5767Info_H
#define TEA767Info_H

struct TEA5767State {
  float frequency = 0.0;
  bool stereo = false;
  bool tuned = false;
  bool muted = false;
  bool japanBand = false;
  uint8_t stopLevel = 10;
  uint8_t level = 0;
  uint8_t ifc = 0;
};

#endif
