#include <cstdint>

class Channel {
public:
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void start();
  int16_t tick();
  void clockLength();

private:
  // registers
  uint8_t initLength;
  uint16_t period;
  bool channelOn;

  // internal state
  uint16_t dutyTimer;
  uint8_t dutyStep;
  uint8_t length;
};

