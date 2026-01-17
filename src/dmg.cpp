#include "dmg.hpp"

void DMG::loadROM(char* fnameBootROM, char* fnameCartROM) {
  // load boot ROM and cartridge ROM
  FILE* f = fopen(fnameBootROM, "rb");
  fread(rom, sizeof(uint8_t), 0x100, f);
  fclose(f);
  cart.loadROM(fnameCartROM);

  // reset I/O
  joyp = 0x00;
  boot = false;

  // reset DMA state
  dmaActive = false;
  dmaPending[0] = false;
  dmaPending[1] = false;
}

uint8_t DMG::JOYP() {
  // todo: is JOYP = 0x00 handled correctly?
  // todo: are bits 7 and 6 handled correctly?
  uint8_t data = 0xcf | joyp;
  if(!(joyp & 0x20)) data &= pollButtons();
  if(!(joyp & 0x10)) data &= pollDpad();
  return data;
}

void DMG::DMA(uint8_t data) {
  dma = data;
  dmaPending[1] = true;
  dmaPendingAddr[1] = dma << 8;
}

uint8_t DMG::readDMA(uint16_t addr) {
  // OAM DMA uses a simpler address decoding scheme
  if(addr < 0x8000) return cart.readROM(addr);
  if(addr < 0xa000) return vram[addr & 0x1fff];
  if(addr < 0xc000) return cart.readRAM(addr);
  return wram[addr & 0x1fff];
}

void DMG::idle() {
  cycle();
}

uint8_t DMG::read8(uint16_t addr) {
  cycle();

  if(addr < 0x0100 && !boot) return rom[addr & 0xff];  // boot ROM
  if(addr < 0x8000) return cart.readROM(addr);  // cartridge ROM region
  if(addr < 0xa000) return vram[addr & 0x1fff];  // VRAM
  if(addr < 0xc000) return cart.readRAM(addr);  // cartridge RAM region
  if(addr < 0xfe00) return wram[addr & 0x1fff];  // WRAM and echo RAM regions
  if(addr < 0xfea0) return dmaActive ? 0xff : oam[addr & 0xff];  // OAM (todo: more accurately limit accessible regions while DMA is active)
  if(addr < 0xff00) return 0x00;  // unused part of OAM region

  // I/O region
  if(addr == 0xff00) return JOYP();  // JOYP
  if(addr == 0xff01) return 0x00;  // todo: SB
  if(addr == 0xff02) return 0x7e;  // todo: SC
  if(addr == 0xff04) return div >> 6;  // DIV
  if(addr == 0xff05) return tima;  // TIMA
  if(addr == 0xff06) return tma;  // TMA
  if(addr == 0xff07) return 0xf8 | tac;  // TAC
  if(addr == 0xff0f) return IF();  // IF
  if(addr >= 0xff10 && addr < 0xff40) { printf("TODO: Reading address 0x%04x\n", addr); return 0xff; }  // todo: APU
  if(addr >= 0xff40 && addr < 0xff46) return ppuReadIO(addr);
  if(addr == 0xff46) return dma;  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) return ppuReadIO(addr);
  if(addr >= 0xff80 && addr < 0xffff) return hram[addr & 0x7f];  // HRAM
  if(addr == 0xffff) return IE();  // IE
  return 0xff;
}

void DMG::write8(uint16_t addr, uint8_t data) {
  cycle();

  if(addr < 0x8000) { cart.writeROM(addr, data); return; }  // cartridge ROM region
  if(addr < 0xa000) { vram[addr & 0x1fff] = data; return; }  // VRAM
  if(addr < 0xc000) { cart.writeRAM(addr, data); return; }  // cartridge RAM region
  if(addr < 0xfe00) { wram[addr & 0x1fff] = data; return; }  // WRAM and echo RAM regions
  if(addr < 0xfea0) { if(!dmaActive) oam[addr & 0xff] = data; return; }  // OAM (todo: more accurately limit accessible regions while DMA is active)
  if(addr < 0xff00) return;  // unused part of OAM region

  // I/O region
  if(addr == 0xff00) { joyp = data & 0x30; return; }  // JOYP
  if(addr == 0xff01) return;  // todo: SB
  if(addr == 0xff02) return;  // todo: SC
  if(addr == 0xff04) { div = 0x0000; return; }  // DIV
  if(addr == 0xff05) { tima = data; return; }  // TIMA
  if(addr == 0xff06) { tma = data; return; }  // TMA
  if(addr == 0xff07) { tac = data & 0x07; return; }  // TAC
  if(addr == 0xff0f) { setIF(data); return; }  // IF
  if(addr >= 0xff10 && addr < 0xff40) return;  // todo: APU
  if(addr >= 0xff40 && addr < 0xff46) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff46) { DMA(data); return; }  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff50) { boot |= (data & 0x01); return; }  // BOOT
  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }  // HRAM
  if(addr == 0xffff) { setIE(data); return; }  // IE
}

void DMG::cycle() {
  ppuTick();

  // run 1 byte of DMA transfer, if active
  if(dmaActive) {
    oam[dmaAddr & 0xff] = readDMA(dmaAddr);
    dmaAddr++;
    if((dmaAddr & 0xff) >= 0xa0) dmaActive = false;
  }

  // start DMA, if pending
  if(dmaPending[0]) {
    dmaActive = true;
    dmaAddr = dmaPendingAddr[0];
  }
  dmaPendingAddr[0] = dmaPendingAddr[1];
  dmaPending[0] = dmaPending[1];
  dmaPending[1] = false;

  // run 1 M-cycle
  div++;

  // find new state of timer clocking signal
  bool clkTimerNew = false;
  switch(tac & 0x07) {
  case 0x04: clkTimerNew = div & 0x80; break;
  case 0x05: clkTimerNew = div & 0x02; break;
  case 0x06: clkTimerNew = div & 0x08; break;
  case 0x07: clkTimerNew = div & 0x20; break;
  }

  // clock timer on falling edge
  if(clkTimer && !clkTimerNew) {
    tima++;
    if(!tima) {
      tima = tma;
      setIF(IF() | 0x04);
    }
  }
  clkTimer = clkTimerNew;
}

