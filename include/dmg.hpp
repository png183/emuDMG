#include "sm83.hpp"

class DMG : public SM83 {
public:
  DMG() {
    rom = new uint8_t[maxRomSize];
    vram = new uint8_t[0x2000];
    wram = new uint8_t[0x2000];
    hram = new uint8_t[0x7f];
  }

  ~DMG() {
    delete[] rom;
    delete[] vram;
    delete[] wram;
    delete[] hram;
  }

  void loadROM(char* fname);
  uint8_t read8(uint16_t addr) override;
  void write8(uint16_t addr, uint8_t data) override;
  void frame();
  virtual void plotPixel(int x, int y, uint8_t data) { return; }

private:
  const int maxRomSize = 0x8000;
  uint8_t* rom;
  uint8_t* vram;
  uint8_t* wram;
  uint8_t* hram;
};

