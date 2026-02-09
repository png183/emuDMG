#include <cstdint>

class PPU {
public:
  PPU() {
    vram = new uint8_t[0x2000];
    oam = new uint8_t[0xa0];

    // initialize PPU state
    lcdc = 0x00;
    stat = 0x00;
    scx = 0x00;
    ly = 0x00;
    lyc = 0x00;
    wy = 0x00;
    wx = 0x00;
    scanCycle = 0;
    irqSTAT = false;
    bgStep = 0;
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
  void oamScan();
  uint8_t bgReadTilemap(uint8_t x);
  uint8_t bgGetTileData(uint8_t tile, uint8_t bitLoHi);
  void bgTickFIFO();
  void renderSprites();

  // PPU registers
  uint8_t lcdc;
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

  // BG FIFO
  uint8_t bgTile;
  uint8_t bgDataLo;
  uint8_t bgDataHi;
  uint8_t bgFifoLo;
  uint8_t bgFifoHi;
  uint8_t bgFifoSize;
  uint8_t bgStep;
  bool bgIsWin;

  // OAM buffer
  uint8_t spriteBuffer[40];

  // PPU internal state
  int scanCycle;
  uint8_t yWinCount;
  bool irqSTAT;
  uint8_t lx;
  int xOut;
  bool rendering;

  // Scanline renderer state
  uint8_t objBuffer[160];
  uint8_t attrBuffer[160];
};

