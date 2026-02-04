#include "ppu.hpp"

uint8_t PPU::ppuReadIO(uint16_t addr) {
  switch(addr) {
  case 0xff40: return lcdc;  // LCDC
  case 0xff41: return STAT();  // STAT
  case 0xff42: return scy;  // SCY
  case 0xff43: return scx;  // SCX
  case 0xff44: return ly;  // LY
  case 0xff45: return lyc;  // LYC
  case 0xff47: return bgp;  // BGP
  case 0xff48: return obp0;  // OBP0
  case 0xff49: return obp1;  // OBP1
  case 0xff4a: return wy;  // WY
  case 0xff4b: return wx;  // WX
  }
  return 0xff;
}

void PPU::ppuWriteIO(uint16_t addr, uint8_t data) {
  switch(addr) {
  case 0xff40: lcdc = data; return;  // LCDC
  case 0xff41: stat = data & 0x78; return;  // STAT
  case 0xff42: scy = data; return;  // SCY
  case 0xff43: scx = data; return;  // SCX
  case 0xff45: lyc = data; return;  // LYC
  case 0xff47: bgp = data; return;  // BGP
  case 0xff48: obp0 = data; return;  // OBP0
  case 0xff49: obp1 = data; return;  // OBP1
  case 0xff4a: wy = data; return;  // WY
  case 0xff4b: wx = data; return;  // WX
  }
}

void PPU::ppuTick() {
  if(!(lcdc & 0x80)) return;

  // run PPU if LCD is enabled
  scanCycle++;
  if(scanCycle == 80) {
    scanline(ly);
  } else if(scanCycle == 456) {
    scanCycle = 0;
    ly++;
    if(ly == 154) {
      ly = 0;
      yWinCount = 0;
      frame();
    }
    if(ly == 144) irqRaiseVBLANK();
  }

  bool irqPrevSTAT = irqSTAT;
  irqSTAT = false;
  if((stat & 0x40) && ly == lyc                    ) irqSTAT = true;  // LYC
  if((stat & 0x20) && ly <= 144 && scanCycle <   80) irqSTAT = true;  // mode 2
  if((stat & 0x10) && ly >= 144                    ) irqSTAT = true;  // mode 1
  if((stat & 0x08) && ly <  144 && scanCycle >= 252) irqSTAT = true;  // mode 0
  if(irqSTAT && !irqPrevSTAT) irqRaiseSTAT();
}

uint8_t PPU::STAT() {
  // todo: is bit 7 handled correctly?
  uint8_t data = 0x80 | stat;
  if(ly == lyc) data |= 0x04;
  if(ly >= 144) {
    data |= 0x01;
  } else if(scanCycle < 80) {
    data |= 0x02;
  } else if(scanCycle < 252) {
    // todo: HBLANK start time can vary
    data |= 0x03;
  }
  return data;
}

void PPU::scanline(uint8_t y) {
  if(y >= 144) return;

  // clear sprite buffer (other scanline buffers will be sufficiently overwritten)
  for(uint8_t x = 0; x < 160; x++) objBuffer[x] = 0x00;

  // render background
  if(lcdc & 0x01) renderBackground(y);
  if(lcdc & 0x01) renderWindow(y);

  // render sprites
  if(lcdc & 0x02) renderSprites(y);

  // output scanline
  for(int x = 0; x < 160; x++) {
    uint8_t bgPalette = bgBuffer[x];
    uint8_t objPalette = objBuffer[x];
    uint8_t attributes = attrBuffer[x];
    uint8_t bgColour = (bgp >> (bgPalette << 1)) & 0x03;
    uint8_t objColour = (((attributes & 0x10) ? obp1 : obp0) >> (objPalette  << 1)) & 0x03;
    uint8_t colour = (objPalette && (!bgPalette || !(attributes & 0x80))) ? objColour : bgColour;
    plotPixel(x, y, colour);
  }
}

uint8_t PPU::bgReadTilemap(uint8_t x, uint8_t y) {
  uint8_t tileY = (uint8_t)(y + scy) >> 3;
  uint8_t tileX = (uint8_t)(x + scx) >> 3;
  uint16_t baseAddr = (lcdc & 0x08) ? 0x1c00 : 0x1800;
  return vram[baseAddr | tileY << 5 | tileX];
}

uint8_t PPU::bgGetTileData(uint8_t tile, uint8_t y, uint8_t bitLoHi) {
  uint8_t fineY = (y + scy) & 0x07;
  uint16_t baseAddr = (!(lcdc & 0x10) && !(tile & 0x80)) ? 0x1000 : 0x0000;
  return vram[baseAddr | tile << 4 | fineY << 1 | bitLoHi];
}

uint8_t PPU::winReadTilemap(uint8_t x, uint8_t y) {
  uint8_t tileY = yWinCount >> 3;
  uint8_t tileX = (uint8_t)(x + 7 - wx) >> 3;
  uint16_t baseAddr = (lcdc & 0x40) ? 0x1c00 : 0x1800;
  return vram[baseAddr | tileY << 5 | tileX];
}

uint8_t PPU::winGetTileData(uint8_t tile, uint8_t y, uint8_t bitLoHi) {
  uint8_t fineY = yWinCount & 0x07;
  uint16_t baseAddr = (!(lcdc & 0x10) && !(tile & 0x80)) ? 0x1000 : 0x0000;
  return vram[baseAddr | tile << 4 | fineY << 1 | bitLoHi];
}

void PPU::renderBackground(uint8_t y) {
  for(uint8_t x = 0; x < 160; x++) {
    // determine sub-tile X-position
    uint8_t fineX = (x + scx) & 0x07;

    // render pixel
    uint8_t tile = bgReadTilemap(x, y);
    uint8_t tileDataLo = bgGetTileData(tile, y, 0);
    uint8_t tileDataHi = bgGetTileData(tile, y, 1);
    uint8_t pxLo = (tileDataLo >> (fineX ^ 0x07)) & 1;
    uint8_t pxHi = (tileDataHi >> (fineX ^ 0x07)) & 1;
    bgBuffer[x] = pxHi << 1 | pxLo;
  }
}

void PPU::renderWindow(uint8_t y) {
  if(!(lcdc & 0x20)) return;
  if(y < wy) return;
  for(uint8_t x = 0; x < 160; x++) {
    if((x + 7) < wx) continue;

    // determine sub-tile X-position
    uint8_t fineX = (x + 7 - wx) & 0x07;

    // render pixel
    uint8_t tile = winReadTilemap(x, y);
    uint8_t tileDataLo = winGetTileData(tile, y, 0);
    uint8_t tileDataHi = winGetTileData(tile, y, 1);
    uint8_t pxLo = (tileDataLo >> (fineX ^ 0x07)) & 1;
    uint8_t pxHi = (tileDataHi >> (fineX ^ 0x07)) & 1;
    bgBuffer[x] = pxHi << 1 | pxLo;
  }
  if(wx < 167) yWinCount++;
}

void PPU::renderSprites(uint8_t y) {
  // calculate sprite height
  uint8_t spriteHeight = (lcdc & 0x04) ? 16 : 8;

  // perform OAM scan
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

  for(int x = -8; x < 160; x++) {
    // check x-coordinate of all buffered sprites
    for(uint8_t i = 0; i < bufSize; i += 4) {
      uint8_t dataX = spriteBuffer[i + 1];
      if((x + 8) == dataX) {
        // fetch tile data
        uint8_t dataY = y + 16 - spriteBuffer[i + 0];
        uint8_t tile = spriteBuffer[i + 2];
        uint8_t attributes = spriteBuffer[i + 3];
        uint8_t fineY = dataY & (spriteHeight - 1);
        if(attributes & 0x40) fineY ^= (spriteHeight - 1);
        if(lcdc & 0x04) tile &= ~1;  // mask low bit of tile ID for 8x16 sprites
        uint16_t tileAddr = tile << 4 | fineY << 1;
        uint8_t tileDataLo = vram[tileAddr | 0];
        uint8_t tileDataHi = vram[tileAddr | 1];

        // draw sprite
        for(uint8_t fineX = 0; fineX < 8; fineX++) {
          int outX = x + fineX;
          if(outX < 0 || outX >= 160) continue;

          // render pixel
          uint8_t outFineX = fineX;
          if(!(attributes & 0x20)) outFineX ^= 0x07;
          uint8_t pxLo = (tileDataLo >> outFineX) & 1;
          uint8_t pxHi = (tileDataHi >> outFineX) & 1;
          uint8_t palette = pxHi << 1 | pxLo;
          if(palette && !(objBuffer[outX])) {
            objBuffer[outX] = palette;
            attrBuffer[outX] = attributes;
          }
        }
      }
    }
  }
}

