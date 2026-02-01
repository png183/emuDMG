#include "sm83.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "cart.hpp"

#include <cstdio>
#include <cstdlib>

class DMG : public SM83, public PPU, public APU {
public:
  DMG() {
    rom =  new uint8_t[0x100];
    wram = new uint8_t[0x2000];
    hram = new uint8_t[0x7f];

    // reset CPU
    reset();

    // reset I/O
    joyp = 0x00;
    tac = 0x00;
    boot = false;

    // reset DMA state
    dmaActive = false;
    dmaPending[0] = false;
    dmaPending[1] = false;
  }

  ~DMG() {
    delete[] rom;
    delete[] wram;
    delete[] hram;
  }

  void insertCart(Cart* cartridge) { cart = cartridge; }
  void loadBootROM(char* fname);
  void idle() override;
  uint8_t read8(uint16_t addr) override;
  void write8(uint16_t addr, uint8_t data) override;
  void irqRaiseVBLANK() override { setIF(IF() | 0x01); }
  void irqRaiseSTAT() override { setIF(IF() | 0x02); }

  virtual uint8_t pollButtons() { return 0xff; }
  virtual uint8_t pollDpad() { return 0xff; }

private:
  void DMA(uint8_t addrHi);
  uint8_t readBus(uint16_t addr);
  void writeBus(uint16_t addr, uint8_t data);
  void joypadTick();
  void cycle();

  // joypad register
  uint8_t joyp;

  // Timer registers
  uint16_t div;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;

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
  Cart* cart;
  uint8_t* rom;
  uint8_t* wram;
  uint8_t* hram;
};

