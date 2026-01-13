#include "cart.hpp"

void Cart::loadROM(char* fname) {
  // load ROM from file
  FILE* f = fopen(fname, "rb");
  if(!f) {
    printf("ERROR: %s is not a valid file path\n", fname);
    exit(0);
  }
  int fsize = fread(rom, sizeof(uint8_t), maxRomSize, f);
  fclose(f);
  printf("Loaded %s\n", fname);

  // pre-mirror ROM to fill 2MiB address space
  for(int i = 0; (i + fsize) <= maxRomSize; i += fsize) memcpy(rom + i, rom, fsize);

  // init mapper (todo: support non-MBC1 mappers)
  ramg = false;
  bank1 = 0x01;
  bank2 = 0x00;
  mode = false;
}

uint8_t Cart::readROM(uint16_t addr) {
  uint32_t romAddr = addr & 0x3fff;
  if(addr & 0x4000) romAddr |= bank1 << 14;
  if(mode || addr & 0x4000) romAddr |= bank2 << 19;
  return rom[romAddr];
}

void Cart::writeROM(uint16_t addr, uint8_t data) {
  switch(addr & 0xe000) {
  case 0x0000:
    // RAMG
    ramg = ((data & 0x0f) == 0x0a);
    return;
  case 0x2000:
    // BANK1
    bank1 = data & 0x1f;
    if(!bank1) bank1 = 0x01;
    return;
  case 0x4000:
    // BANK2
    bank2 = data & 0x03;
    return;
  case 0x6000:
    // MODE
    mode = data & 0x01;
    return;
  }
}

uint8_t Cart::readRAM(uint16_t addr) {
  if(!ramg) return 0xff;
  uint16_t ramAddr = addr & 0x1fff;
  if(mode) ramAddr |= bank2 << 13;
  return ram[ramAddr];
}

void Cart::writeRAM(uint16_t addr, uint8_t data) {
  if(!ramg) return;
  uint16_t ramAddr = addr & 0x1fff;
  if(mode) ramAddr |= bank2 << 13;
  ram[ramAddr] = data;
}

