#include <cstdint>

class Channel {
public:
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void start();
  bool active();
  int16_t tick();
  void clockLength();
  void clockEnvelope();

private:
  // registers
  uint8_t initLength;
  uint8_t initVolume;
  uint8_t sweepPace;
  uint16_t period;
  bool channelOn;
  bool lengthEnable;

  // internal state
  uint16_t dutyTimer;
  uint8_t dutyStep;
  uint8_t sweepStep;
  uint8_t length;
  uint8_t volume;
};

