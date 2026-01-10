#include "sm83.hpp"

class DMG : public SM83 {
public:
  DMG() {
    rom = new uint8_t[maxRomSize];
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

  void loadROM(char* fname);
  uint8_t read8(uint16_t addr) override;
  void write8(uint16_t addr, uint8_t data) override;
  void frame();
  virtual void plotPixel(int x, int y, uint8_t data) { return; }

private:
  void scanline(uint8_t y);
  void renderBackground(uint8_t y);
  void renderSprites(uint8_t y);

  // PPU registers
  uint8_t lcdc;  // todo: bit 7
  uint8_t stat;  // todo: bits 5-0
  uint8_t scy;
  uint8_t scx;
  uint8_t lyc;
  uint8_t obp0;
  uint8_t obp1;
  uint8_t wy;
  uint8_t wx;

  // PPU internal state
  uint8_t yWinCount;

  // Scanline renderer state
  uint8_t lineBuffer[160];

  // Memory
  const int maxRomSize = 0x8000;
  uint8_t* rom;
  uint8_t* vram;
  uint8_t* wram;
  uint8_t* oam;
  uint8_t* hram;
};

