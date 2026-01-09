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

  if(addr == 0xff44) return 144;  //todo: don't hardcode
  if(addr >= 0xff80 && addr < 0xffff) return hram[addr & 0x7f];
  printf("TODO: Reading address 0x%04x\n", addr);
//  exit(0);
  return 0xff;
}

void DMG::write8(uint16_t addr, uint8_t data) {
  if(addr < 0x8000) {
    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
    exit(0);
    return;
  }
  if(addr < 0xa000) { vram[addr & 0x1fff] = data; return; }
  if(addr < 0xc000) {
    printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
    exit(0);
    return;
  }
  if((addr & 0xe000) == 0xc000) { wram[addr & 0x1fff] = data; return; }

  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }
  printf("TODO: Writing 0x%02x to address 0x%04x\n", data, addr);
}

void DMG::frame() {
  //run CPU
  for(int i = 0; i < 70224; i++) instruction();

  //run PPU
  for(uint8_t tileY = 0; tileY < 18; tileY++) {
    for(uint8_t tileX = 0; tileX < 20; tileX++) {
      //render tile
      uint8_t tile = vram[0x1800 | tileY << 5 | tileX];
      for(uint8_t fineY = 0; fineY < 8; fineY++) {
        uint8_t y = tileY << 3 | fineY;
        uint8_t tileDataLo = vram[tile << 4 | fineY << 1 | 0];
        uint8_t tileDataHi = vram[tile << 4 | fineY << 1 | 1];
        for(uint8_t fineX = 0; fineX < 8; fineX++) {
          uint8_t x = tileX << 3 | fineX;
          uint8_t pxLo = (tileDataLo >> (fineX ^ 0x07)) & 1;
          uint8_t pxHi = (tileDataHi >> (fineX ^ 0x07)) & 1;
          plotPixel(x, y, pxHi << 1 | pxLo);
        }
      }
    }
  }
}

