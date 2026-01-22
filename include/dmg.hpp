#include "sm83.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "cart.hpp"

class DMG : public SM83, public PPU {
public:
  DMG() {
    rom =  new uint8_t[0x100];
    wram = new uint8_t[0x2000];
    hram = new uint8_t[0x7f];
  }

  ~DMG() {
    delete[] rom;
    delete[] wram;
    delete[] hram;
  }

  void loadROM(char* fnameBootROM, char* fnameCartROM);
  void idle() override;
  uint8_t read8(uint16_t addr) override;
  void write8(uint16_t addr, uint8_t data) override;
  void irqRaiseVBLANK() override { setIF(IF() | 0x01); }
  void irqRaiseSTAT() override { setIF(IF() | 0x02); }

  virtual uint8_t pollButtons() { return 0xff; }
  virtual uint8_t pollDpad() { return 0xff; }
  virtual void emitSample(int16_t volume) { return; }

private:
  uint8_t JOYP();
  uint8_t NR52();
  void DMA(uint8_t addrHi);
  uint8_t readDMA(uint16_t addr);
  uint8_t readAPU(uint16_t addr);
  void writeAPU(uint16_t addr, uint8_t data);
  void apuTick();
  void divAPU();
  void cycle();

  // joypad register
  uint8_t joyp;

  // Timer registers
  uint16_t div;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;

  // APU channels
  CH2 ch1;  // todo: implement CH1 functionality
  CH2 ch2;
  CH3 ch3;
  CH4 ch4;

  // APU registers
  uint8_t nr51;  // todo: implement panning
  bool nr52;

  // OAM DMA register
  uint8_t dma;

  // Boot ROM disable register
  bool boot;

  // Timer circuit internal state
  bool clkTimer;

  // OAM DMA internal state
  bool dmaPending[2];
  uint16_t dmaPendingAddr[2];
  bool dmaActive;
  uint16_t dmaAddr;

  // Memory
  Cart cart;
  uint8_t* rom;
  uint8_t* wram;
  uint8_t* hram;
};

