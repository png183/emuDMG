#include "dmg.hpp"

void DMG::loadROM(char* fname) {
  printf("Loading %s\n", fname);
  FILE* f = fopen(fname, "rb");
  fread(rom, sizeof(uint8_t), maxRomSize, f);
  fclose(f);
}

uint8_t DMG::read8(uint16_t addr) {
  if(addr < 0x8000) return rom[addr];
  if(addr < 0xc000) {
    printf("TODO: Reading address 0x%04x\n", addr);
    exit(0);
    return 0xff;
  }
  if(addr < 0xe000) return wram[addr & 0x1fff];
  if(addr < 0xfe00) {
    printf("TODO: Reading address 0x%04x\n", addr);
    exit(0);
    return 0xff;
  }
  if(addr < 0xff00) {
    printf("TODO: Reading address 0x%04x\n", addr);
    exit(0);
    return 0xff;
  }

  //todo: I/O
  if(addr == 0xff0f) return IF();  //IF
  if(addr == 0xff40) return lcdc;  //LCDC
  if(addr == 0xff44) return 144;  //todo: LY
  if(addr >= 0xff80 && addr < 0xffff) return hram[addr & 0x7f];
  printf("TODO: Reading address 0x%04x\n", addr);
//  exit(0);
  return 0xff;
}

void DMG::write8(uint16_t addr, uint8_t data) {
  if(addr < 0x8000) {
//    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
//    exit(0);
    return;
  }
  if(addr < 0xa000) { vram[addr & 0x1fff] = data; return; }
  if(addr < 0xc000) {
    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
    exit(0);
    return;
  }
  if(addr < 0xe000) { wram[addr & 0x1fff] = data; return; }
  if(addr < 0xfe00) {
    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
    exit(0);
    return;
  }
  if(addr < 0xff00) {
//    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
//    exit(0);
    return;
  }

  //todo: I/O
  if(addr == 0xff01) return;  //todo: SB
  if(addr == 0xff02) return;  //todo: SC
  if(addr == 0xff0f) { setIF(data); return; }  //IF
  if(addr == 0xff40) { lcdc = data; return; }  //LCDC
  if(addr == 0xff42) { scy = data; return; }  //SCY
  if(addr == 0xff43) { scx = data; return; }  //SCX
  if(addr == 0xff45) { lyc = data; return; }  //LYC
  if(addr == 0xff4a) { wy = data; return; }  //WY
  if(addr == 0xff4b) { wx = data; return; }  //WX
  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }
  if(addr == 0xffff) { setIE(data); return; }  //IE
  printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
}

void DMG::frame() {
  yWinCount = 0;
  for(uint8_t y = 0; y < 154; y++) {
    if(y == lyc) setIF(IF() | 0x02);  //todo: allow disabling and setting alternate conditions
    for(int i = 0; i < 456; i++) instruction();
    scanline(y);
  }
}

void DMG::scanline(uint8_t y) {
  if(y >= 144) return;

  //clear pixels
  for(uint8_t x = 0; x < 160; x++) plotPixel(x, y, 0);

  //render background
  if(!(lcdc & 0x01)) return;
  for(uint8_t x = 0; x < 160; x++) {
    //determine rendering Y-position
    uint8_t dataY = y + scy;
    uint8_t tileY = dataY >> 3;
    uint8_t fineY = dataY & 0x07;

    //determine rendering X-position
    uint8_t dataX = x + scx;
    uint8_t tileX = dataX >> 3;
    uint8_t fineX = dataX & 0x07;

    //render pixel
    uint8_t tile = vram[((lcdc & 0x08) ? 0x1c00 : 0x1800) | tileY << 5 | tileX];
    uint16_t tileAddr = tile << 4 | fineY << 1;
    if(!(lcdc & 0x10) && !(tile & 0x80)) tileAddr += 0x1000;
    uint8_t tileDataLo = vram[tileAddr | 0];
    uint8_t tileDataHi = vram[tileAddr | 1];
    uint8_t pxLo = (tileDataLo >> (fineX ^ 0x07)) & 1;
    uint8_t pxHi = (tileDataHi >> (fineX ^ 0x07)) & 1;
    plotPixel(x, y, pxHi << 1 | pxLo);
  }

  //render window
  if(!(lcdc & 0x20)) return;
  if(y < wy) return;
  for(uint8_t x = 0; x < 160; x++) {
    if((x + 7) < wx) continue;

    //determine rendering Y-position
    uint8_t tileY = yWinCount >> 3;
    uint8_t fineY = yWinCount & 0x07;

    //determine rendering X-position
    uint8_t dataX = x + 7 - wx;
    uint8_t tileX = dataX >> 3;
    uint8_t fineX = dataX & 0x07;

    //render pixel
    uint8_t tile = vram[((lcdc & 0x40) ? 0x1c00 : 0x1800) | tileY << 5 | tileX];
    uint16_t tileAddr = tile << 4 | fineY << 1;
    if(!(lcdc & 0x10) && !(tile & 0x80)) tileAddr += 0x1000;
    uint8_t tileDataLo = vram[tileAddr | 0];
    uint8_t tileDataHi = vram[tileAddr | 1];
    uint8_t pxLo = (tileDataLo >> (fineX ^ 0x07)) & 1;
    uint8_t pxHi = (tileDataHi >> (fineX ^ 0x07)) & 1;
    plotPixel(x, y, pxHi << 1 | pxLo);
  }
  if(wx < 167) yWinCount++;
}

