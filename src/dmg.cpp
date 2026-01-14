#include "dmg.hpp"

void DMG::loadROM(char* fnameBootROM, char* fnameCartROM) {
  boot = false;
  FILE* f = fopen(fnameBootROM, "rb");
  fread(rom, sizeof(uint8_t), 0x100, f);
  fclose(f);
  cart.loadROM(fnameCartROM);
}

uint8_t DMG::JOYP() {
  //todo: is JOYP = 0x00 handled correctly?
  //todo: are bits 7 and 6 handled correctly?
  uint8_t data = joyp | 0xcf;
  if(!(joyp & 0x20)) data &= pollButtons();
  if(!(joyp & 0x10)) data &= pollDpad();
  return data;
}

void DMG::DMA(uint8_t addrHi) {
  dma = addrHi;
  //todo: implement actual timings
  for(uint8_t i = 0; i < 0xa0; i++) {
    oam[i] = read8(addrHi << 8 | i);
  }
}

void DMG::idle() {
  cycle();
}

uint8_t DMG::read8(uint16_t addr) {
  cycle();

  if(addr < 0x0100 && !boot) return rom[addr & 0xff];  //boot ROM
  if(addr < 0x8000) return cart.readROM(addr);  //cartridge ROM region
  if(addr < 0xa000) return vram[addr & 0x1fff];  //VRAM
  if(addr < 0xc000) return cart.readRAM(addr);  //cartridge RAM region
  if(addr < 0xfe00) return wram[addr & 0x1fff];  //WRAM and echo RAM regions
  if(addr < 0xfea0) return oam[addr & 0xff];  //OAM
  if(addr < 0xff00) return 0x00;  //unused part of OAM region

  //todo: I/O
  if(addr == 0xff00) return JOYP();  //JOYP
  if(addr == 0xff04) return div >> 6;  //DIV
  if(addr == 0xff05) return tima;  //TIMA
  if(addr == 0xff0f) return IF();  //IF
  if(addr == 0xff40) return lcdc;  //LCDC
  if(addr == 0xff41) return stat;  //STAT
  if(addr == 0xff42) return scy;  //SCY
  if(addr == 0xff44) return ly;  //LY
  if(addr == 0xff46) return dma;  //DMA
  if(addr == 0xff4a) return wy;  //WY
  if(addr >= 0xff80 && addr < 0xffff) return hram[addr & 0x7f];
  if(addr == 0xffff) return IE();  //IE
  printf("TODO: Reading address 0x%04x\n", addr);
  return 0xff;
}

void DMG::write8(uint16_t addr, uint8_t data) {
  cycle();

  if(addr < 0x8000) { cart.writeROM(addr, data); return; }  //cartridge ROM region
  if(addr < 0xa000) { vram[addr & 0x1fff] = data; return; }  //VRAM
  if(addr < 0xc000) { cart.writeRAM(addr, data); return; }  //cartridge RAM region
  if(addr < 0xfe00) { wram[addr & 0x1fff] = data; return; }  //WRAM and echo RAM regions
  if(addr < 0xfea0) { oam[addr & 0xff] = data; return; }  //OAM
  if(addr < 0xff00) return;  //unused part of OAM region

  //todo: I/O
  if(addr == 0xff00) { joyp = data & 0x30; return; }  //JOYP
  if(addr == 0xff01) return;  //todo: SB
  if(addr == 0xff02) return;  //todo: SC
  if(addr == 0xff04) { div = 0x0000; return; }  //DIV
  if(addr == 0xff05) { tima = data; return; }  //TIMA
  if(addr == 0xff06) { tma = data; return; }  //TMA
  if(addr == 0xff07) { tac = data; return; }  //TAC
  if(addr == 0xff0f) { setIF(data); return; }  //IF
  if(addr == 0xff40) { lcdc = data; return; }  //LCDC
  if(addr == 0xff41) { stat = data; return; }  //STAT
  if(addr == 0xff42) { scy = data; return; }  //SCY
  if(addr == 0xff43) { scx = data; return; }  //SCX
  if(addr == 0xff45) { lyc = data; return; }  //LYC
  if(addr == 0xff46) { DMA(data); return; }  //DMA
  if(addr == 0xff48) { obp0 = data; return; }  //OBP0
  if(addr == 0xff49) { obp1 = data; return; }  //OBP1
  if(addr == 0xff50) { boot |= (data & 0x01); return; }  //BOOT
  if(addr == 0xff4a) { wy = data; return; }  //WY
  if(addr == 0xff4b) { wx = data; return; }  //WX
  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }
  if(addr == 0xffff) { setIE(data); return; }  //IE
  printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
}

void DMG::cycle() {
  //run 1 M-cycle
  div++;
  scanCycle++;

  //find new state of timer clocking signal
  bool clkTimerNew = false;
  switch(tac & 0x07) {
  case 0x04: clkTimerNew = div & 0x80; break;
  case 0x05: clkTimerNew = div & 0x02; break;
  case 0x06: clkTimerNew = div & 0x08; break;
  case 0x07: clkTimerNew = div & 0x20; break;
  }

  //clock timer on falling edge
  if(clkTimer && !clkTimerNew) {
    tima++;
    if(!tima) {
      tima = tma;
      setIF(IF() | 0x04);
    }
  }
  clkTimer = clkTimerNew;
}

void DMG::frame() {
  yWinCount = 0;
  for(ly = 0; ly < 154; ly++) {
    //handle VBLANK and STAT interrupts
    if(ly == 144) setIF(IF() | 0x01);
    if((stat & 0x40) && ly == lyc) setIF(IF() | 0x02);

    //note: 114 M-cycles (456 T-cycles) per scanline
    while(scanCycle < 114) {
      instruction();
    }
    scanCycle -= 114;
    scanline(ly);
  }
}

void DMG::scanline(uint8_t y) {
  if(y >= 144) return;

  //clear pixels
  for(uint8_t x = 0; x < 160; x++) lineBuffer[x] = 0;

  //render background
  if(lcdc & 0x01) renderBackground(y);

  //render sprites
  if(lcdc & 0x02) renderSprites(y);

  //output scanline
  for(int x = 0; x < 160; x++) plotPixel(x, y, lineBuffer[x]);
}

void DMG::renderBackground(uint8_t y) {
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
    lineBuffer[x] = pxHi << 1 | pxLo;
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
    lineBuffer[x] = pxHi << 1 | pxLo;
  }
  if(wx < 167) yWinCount++;
}

void DMG::renderSprites(uint8_t y) {
  //calculate sprite height
  uint8_t spriteHeight = (lcdc & 0x04) ? 16 : 8;

  //perform OAM scan
  uint8_t spriteBuffer[40];
  uint8_t bufSize = 0;
  for(uint8_t i = 0; i < 160; i += 4) {
    uint8_t dataY = y + 16 - oam[i + 0];
    if(dataY >= spriteHeight) continue;
    spriteBuffer[bufSize + 0] = oam[i + 0];
    spriteBuffer[bufSize + 1] = oam[i + 1];
    spriteBuffer[bufSize + 2] = oam[i + 2];
    spriteBuffer[bufSize + 3] = oam[i + 3];
    bufSize += 4;
    if(bufSize >= 40) break;
  }

  for(uint8_t x = 0; x < 160; x++) {
    //find leftmost sprite that intersects this pixel
    uint8_t index = 0xff;
    for(uint8_t i = 0; i < bufSize; i += 4) {
      uint8_t dataX = spriteBuffer[i + 1];
      if((x + 8) >= dataX && (x + 8) < (dataX + 8)) {
        if(index == 0xff || dataX < spriteBuffer[index]) {
          index = i;
        }
      }
    }
    if(index == 0xff) continue;

    //fetch tile data
    uint8_t dataY = y + 16 - spriteBuffer[index + 0];
    uint8_t dataX = spriteBuffer[index + 1];
    uint8_t tile = spriteBuffer[index + 2];
    uint8_t attributes = spriteBuffer[index + 3];
    uint8_t fineY = dataY & (spriteHeight - 1);
    if(attributes & 0x40) fineY ^= (spriteHeight - 1);
    if(lcdc & 0x04) tile &= ~1;  //mask low bit of tile ID for 8x16 sprites
    uint16_t tileAddr = tile << 4 | fineY << 1;
    uint8_t tileDataLo = vram[tileAddr | 0];
    uint8_t tileDataHi = vram[tileAddr | 1];

    //render pixel
    uint8_t fineX = x + 8 - dataX;
    if(!(attributes & 0x20)) fineX ^= 0x07;
    uint8_t pxLo = (tileDataLo >> fineX) & 1;
    uint8_t pxHi = (tileDataHi >> fineX) & 1;
    uint8_t palette = pxHi << 1 | pxLo;
    uint8_t colour = (((attributes & 0x10) ? obp1 : obp0) >> (palette  << 1)) & 0x03;
    if(palette && (!lineBuffer[x] || !(attributes & 0x80))) lineBuffer[x] = colour;
  }
}

