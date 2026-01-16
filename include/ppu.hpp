#include <cstdint>

class PPU {
public:
  PPU() {
    vram = new uint8_t[0x2000];
    oam = new uint8_t[0xa0];

    // initialize PPU state
    stat = 0x00;
    scx = 0x00;
    lyc = 0x00;
    scanCycle = 0;
  }

  ~PPU() {
    delete[] vram;
    delete[] oam;
  }

  uint8_t ppuReadIO(uint16_t addr);
  void ppuWriteIO(uint16_t addr, uint8_t data);
  void ppuTick();

  virtual void irqRaiseVBLANK() { return; }
  virtual void irqRaiseSTAT() { return; }
  virtual void frame() { return; }
  virtual void plotPixel(int x, int y, uint8_t data) { return; }

  // PPU memory
  uint8_t* vram;
  uint8_t* oam;

private:
  uint8_t STAT();
  void scanline(uint8_t y);
  void renderBackground(uint8_t y);
  void renderSprites(uint8_t y);

  // PPU registers
  uint8_t lcdc;  // todo: bit 7
  uint8_t stat;
  uint8_t scy;
  uint8_t scx;
  uint8_t ly;
  uint8_t lyc;
  uint8_t bgp;
  uint8_t obp0;
  uint8_t obp1;
  uint8_t wy;
  uint8_t wx;

  // PPU internal state
  int scanCycle;
  uint8_t yWinCount;
  bool irqSTAT;

  // Scanline renderer state
  uint8_t bgBuffer[160];
  uint8_t objBuffer[160];
  uint8_t attrBuffer[160];
};

