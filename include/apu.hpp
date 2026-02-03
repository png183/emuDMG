#include <cstdint>

class Length {
public:
  uint8_t readNRx4();
  void writeNRx1(uint8_t data);
  void writeNRx4(uint8_t data);
  void divAPU();
  void trigger();
  void disable();
  void updateClkLength();
  void clockLength();
  virtual constexpr uint8_t LEN_MASK() { return 0x3f; }

  // state used by other portions of audio channel
  bool channelOn;

private:
  // registers
  uint8_t initLength;
  bool lengthEnable;

  // internal state
  bool clkLength;
  bool lengthActive;
  uint8_t length;
  uint8_t subdiv;
};

class CH1 : public Length {
public:
  uint8_t readNRx0();
  uint8_t readNRx1();
  uint8_t readNRx2();
  void writeNRx0(uint8_t data);
  void writeNRx1(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void trigger();
  void disable();
  bool active();
  int16_t tick();
  void calcFrequency();
  void clockSweep();
  void clockEnvelope();

private:
  // registers
  uint8_t dutyCycle;
  uint8_t initVolume;
  bool crescendo;
  uint8_t envelopePace;
  uint16_t period;

  // internal state
  bool dacOn;
  uint16_t dutyTimer;
  uint8_t dutyStep;
  uint8_t envelopeStep;
  uint8_t volume;
  uint16_t activePeriod;

  // CH1 period sweep registers
  uint8_t sweepPace;
  bool sweepDirection;
  uint8_t sweepSize;

  // CH1 period sweep internal state
  uint8_t sweepStep;
  bool sweepActive;
};

class CH3 : public Length {
public:
  uint8_t readNRx0();
  uint8_t readNRx2();
  void writeNRx0(uint8_t data);
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  uint8_t readRAM(uint16_t addr);
  void writeRAM(uint16_t addr, uint8_t data);
  void trigger();
  void disable();
  bool active();
  int16_t tick();
  constexpr uint8_t LEN_MASK() override { return 0xff; }

private:
  // wave RAM
  uint8_t ram[0x10];

  // registers
  uint8_t volume;
  uint16_t period;

  // internal state
  bool dacOn;
  uint16_t dutyTimer;
  uint8_t index;
};

class CH4 : public Length {
public:
  uint8_t readNRx2();
  uint8_t readNRx3();
  void writeNRx2(uint8_t data);
  void writeNRx3(uint8_t data);
  void writeNRx4(uint8_t data);
  void trigger();
  void disable();
  bool active();
  int16_t tick();
  void clockEnvelope();

private:
  // registers
  uint8_t initVolume;
  bool crescendo;
  uint8_t envelopePace;
  uint8_t clockShift;
  bool lfsrWidth;
  uint8_t clockDivider;

  // internal state
  bool dacOn;
  uint32_t clockTimer;
  uint8_t dutyStep;
  uint8_t envelopeStep;
  uint8_t volume;
  uint16_t lfsr;
};

class APU {
public:
  APU() {
    // reset channels upon initialization
    ch1 = CH1();
    ch2 = CH1();
    ch3 = CH3();
    ch4 = CH4();

    // reset NR52
    nr52 = false;

    // reset internal state
    subdiv = 0x00;
  }

  virtual void emitSample(int16_t volume) { return; }
  uint8_t apuReadIO(uint16_t addr);
  void apuWriteIO(uint16_t addr, uint8_t data);
  void apuTick();
  void divAPU();

private:
  // APU channels
  CH1 ch1;
  CH1 ch2;  // note: ch2 should not call ch1-specific functions (readNRx0(), writeNRx0(), clockSweep())
  CH3 ch3;
  CH4 ch4;

  // APU registers
  uint8_t nr50;  // todo: implement panning
  uint8_t nr51;  // todo: implement panning
  bool nr52;

  // APU internal state
  uint8_t subdiv;
};

