#include "sm83.hpp"
#include "cart.hpp"

class DMG : public SM83 {
public:
  DMG() {
    rom =  new uint8_t[0x100];
    vram = new uint8_t[0x2000];
    wram = new uint8_t[0x2000];
    oam = new uint8_t[0xa0];
    hram = new uint8_t[0x7f];
  }

  ~DMG() {
    delete[] rom;
    delete[] vram;
    delete[] wram;
    delete[] oam;
    delete[] hram;
  }

  void loadROM(char* fnameBootROM, char* fnameCartROM);
  void idle() override;
  uint8_t read8(uint16_t addr) override;
  void write8(uint16_t addr, uint8_t data) override;

  virtual void frame() { return; }
  virtual uint8_t pollButtons() { return 0xff; }
  virtual uint8_t pollDpad() { return 0xff; }
  virtual void plotPixel(int x, int y, uint8_t data) { return; }

private:
  uint8_t JOYP();
  uint8_t STAT();
  void DMA(uint8_t addrHi);
  void cycle();
  void scanline(uint8_t y);
  void renderBackground(uint8_t y);
  void renderSprites(uint8_t y);

  // joypad register
  uint8_t joyp;

  // PPU registers
  uint8_t lcdc;  // todo: bit 7
  uint8_t stat;  // todo: bit 4
  uint8_t scy;
  uint8_t scx;
  uint8_t ly;
  uint8_t lyc;
  uint8_t bgp;
  uint8_t obp0;
  uint8_t obp1;
  uint8_t wy;
  uint8_t wx;

  // OAM DMA register
  uint8_t dma;

  // Boot ROM disable register
  bool boot;

  // PPU internal state
  int scanCycle;
  uint8_t yWinCount;

  // Scanline renderer state
  uint8_t bgBuffer[160];
  uint8_t objBuffer[160];
  uint8_t attrBuffer[160];

  // Timer registers
  uint16_t div;
  uint8_t tima;
  uint8_t tma;
  uint8_t tac;

  // Timer circuit internal state
  bool clkTimer;

  // Memory
  Cart cart;
  uint8_t* rom;
  uint8_t* vram;
  uint8_t* wram;
  uint8_t* oam;
  uint8_t* hram;
};

