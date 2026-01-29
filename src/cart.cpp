#include "cart.hpp"

uint8_t MBC1::readROM(uint16_t addr) {
  uint32_t romAddr = addr & 0x3fff;
  if(addr & 0x4000) romAddr |= bank1 << 14;
  if(mode || addr & 0x4000) romAddr |= bank2 << 19;
  return rom[romAddr];
}

void MBC1::writeROM(uint16_t addr, uint8_t data) {
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

uint8_t MBC1::readRAM(uint16_t addr) {
  if(!ram || !ramg) return 0xff;
  uint16_t ramAddr = addr & 0x1fff;
  if(mode) ramAddr |= bank2 << 13;
  return ram[ramAddr & ramMask];
}

void MBC1::writeRAM(uint16_t addr, uint8_t data) {
  if(!ram || !ramg) return;
  uint16_t ramAddr = addr & 0x1fff;
  if(mode) ramAddr |= bank2 << 13;
  ram[ramAddr & ramMask] = data;
}

uint8_t MBC5::readROM(uint16_t addr) {
  uint32_t romAddr = addr & 0x3fff;
  if(addr & 0x4000) {
    romAddr |= romb0 << 14;
    romAddr |= romb1 << 22;
  }
  return rom[romAddr];
}

void MBC5::writeROM(uint16_t addr, uint8_t data) {
  switch(addr & 0xf000) {
  case 0x0000:
  case 0x1000:
    // RAMG
    ramg = (data == 0x0a);
    return;
  case 0x2000:
    // ROMB0
    romb0 = data;
    return;
  case 0x3000:
    // ROMB1
    romb1 = data & 0x01;
    return;
  case 0x4000:
  case 0x5000:
    // RAMB
    ramb = data & 0x0f;
    return;
  }
}

uint8_t MBC5::readRAM(uint16_t addr) {
  if(!ram || !ramg) return 0xff;
  uint16_t ramAddr = (ramb << 13) | (addr & 0x1fff);
  return ram[ramAddr & ramMask];
}

void MBC5::writeRAM(uint16_t addr, uint8_t data) {
  if(!ram || !ramg) return;
  uint16_t ramAddr = (ramb << 13) | (addr & 0x1fff);
  ram[ramAddr & ramMask] = data;
}

