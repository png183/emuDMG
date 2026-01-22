#include <cstdint>

class CH1 {
public:
  uint8_t readNRx0();
  uint8_t readNRx1();
  uint8_t readNRx2();
  uint8_t readNRx4();
  void writeNRx0(uint8_t data);
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void trigger();
  bool active();
  int16_t tick();
  void clockLength();
  void clockSweep();
  void clockEnvelope();

private:
  // registers
  uint8_t dutyCycle;
  uint8_t initLength;
  uint8_t initVolume;
  bool crescendo;
  uint8_t envelopePace;
  uint16_t period;
  bool lengthEnable;

  // internal state
  bool dacOn;
  bool channelOn;
  uint16_t dutyTimer;
  uint8_t dutyStep;
  uint8_t envelopeStep;
  uint8_t length;
  uint8_t volume;
  uint16_t activePeriod;

  // CH1 period sweep registers
  uint8_t sweepPace;
  bool sweepDirection;
  uint8_t sweepSize;

  // CH1 period sweep internal state
  uint8_t sweepStep;
};

class CH3 {
public:
  uint8_t readNRx0();
  uint8_t readNRx2();
  uint8_t readNRx4();
  void writeNRx0(uint8_t data);
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  uint8_t readRAM(uint16_t addr);
  void writeRAM(uint16_t addr, uint8_t data);
  void trigger();
  bool active();
  int16_t tick();
  void clockLength();

private:
  // wave RAM
  uint8_t ram[0x10];

  // registers
  uint8_t initLength;
  uint8_t volume;
  uint16_t period;
  bool lengthEnable;

  // internal state
  bool dacOn;
  bool channelOn;
  uint16_t dutyTimer;
  uint8_t index;
  uint8_t length;
};

class CH4 {
public:
  uint8_t readNRx2();
  uint8_t readNRx3();
  uint8_t readNRx4();
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void trigger();
  bool active();
  int16_t tick();
  void clockLength();
  void clockEnvelope();

private:
  // registers
  uint8_t initLength;
  uint8_t initVolume;
  bool crescendo;
  uint8_t envelopePace;
  uint8_t clockShift;
  bool lfsrWidth;
  uint8_t clockDivider;
  bool lengthEnable;

  // internal state
  bool dacOn;
  bool channelOn;
  uint32_t clockTimer;
  uint8_t dutyStep;
  uint8_t envelopeStep;
  uint8_t length;
  uint8_t volume;
  uint16_t lfsr;
};

