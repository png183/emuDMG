#include "dmg.hpp"

void DMG::loadBootROM(char* fname) {
  // load boot ROM
  FILE* fb = fopen(fname, "rb");
  if(!fb) {
    printf("ERROR: %s is not a valid file path\n", fname);
    exit(0);
  }
  fread(rom, sizeof(uint8_t), 0x100, fb);
  fclose(fb);
}

void DMG::SC(uint8_t data) {
  sc = data & 0x81;
  if(sc == 0x81) serialBits = 8;
}

void DMG::DMA(uint8_t data) {
  dma = data;
  dmaPending[1] = true;
  dmaPendingAddr[1] = dma << 8;
}

uint8_t DMG::readBus(uint16_t addr) {
  if(addr < 0x8000) return cart->readROM(addr);
  if(addr < 0xa000) return vram[addr & 0x1fff];
  if(addr < 0xc000) return cart->readRAM(addr);
  return wram[addr & 0x1fff];
}

void DMG::writeBus(uint16_t addr, uint8_t data) {
  if(addr < 0x8000) return cart->writeROM(addr, data);
  if(addr < 0xa000) { vram[addr & 0x1fff] = data; return; }
  if(addr < 0xc000) return cart->writeRAM(addr, data);
  wram[addr & 0x1fff] = data;
  return;
}

void DMG::idle() {
  cycle();
}

uint8_t DMG::read8(uint16_t addr) {
  cycle();

  if(addr < 0x0100 && !boot) return rom[addr & 0xff];
  if(addr < 0xfe00) return dmaActive ? 0xff : readBus(addr);
  if(addr < 0xfea0) return dmaActive ? 0xff : oam[addr & 0xff];
  if(addr < 0xff00) return 0x00;  // unused part of OAM region

  // I/O region
  if(addr == 0xff00) return 0xc0 | joyp;  // JOYP
  if(addr == 0xff01) return sb;  // SB
  if(addr == 0xff02) return sc | 0x7e;  // SC
  if(addr == 0xff04) return div >> 6;  // DIV
  if(addr == 0xff05) return tima;  // TIMA
  if(addr == 0xff06) return tma;  // TMA
  if(addr == 0xff07) return 0xf8 | tac;  // TAC
  if(addr == 0xff0f) return IF();  // IF
  if(addr >= 0xff10 && addr < 0xff40) return apuReadIO(addr);  // APU I/O
  if(addr >= 0xff40 && addr < 0xff46) return ppuReadIO(addr);  // PPU I/O
  if(addr == 0xff46) return dma;  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) return ppuReadIO(addr);  // PPU I/O
  if(addr >= 0xff80 && addr < 0xffff) return hram[addr & 0x7f];  // HRAM
  if(addr == 0xffff) return IE();  // IE
  return 0xff;
}

void DMG::write8(uint16_t addr, uint8_t data) {
  cycle();

  if(addr < 0xfe00) { if(!dmaActive) writeBus(addr, data); return; }
  if(addr < 0xfea0) { if(!dmaActive) oam[addr & 0xff] = data; return; }
  if(addr < 0xff00) return;  // unused part of OAM region

  // I/O region
  if(addr == 0xff00) { joyp &= 0xcf; joyp |= data & 0x30; return; }  // JOYP
  if(addr == 0xff01) { sb = data; return; }  // SB
  if(addr == 0xff02) { SC(data); return; }  // SC
  if(addr == 0xff04) { div = 0x0000; return; }  // DIV
  if(addr == 0xff05) { tima = data; return; }  // TIMA
  if(addr == 0xff06) { tma = data; return; }  // TMA
  if(addr == 0xff07) { tac = data & 0x07; return; }  // TAC
  if(addr == 0xff0f) { setIF(data); return; }  // IF
  if(addr >= 0xff10 && addr < 0xff40) { apuWriteIO(addr, data); return; }  // APU I/O
  if(addr >= 0xff40 && addr < 0xff46) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff46) { DMA(data); return; }  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff50) { boot |= (data & 0x01); return; }  // BOOT
  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }  // HRAM
  if(addr == 0xffff) { setIE(data); return; }  // IE
}

void DMG::joypadTick() {
  // determine new JOYP state
  // todo: is (joyp & 0x30) == 0x00 handled correctly?
  uint8_t data = 0x0f;
  if(!(joyp & 0x20)) data &= pollButtons();
  if(!(joyp & 0x10)) data &= pollDpad();

  // check for interrupt
  if((joyp & data) != (joyp & 0x0f)) setIF(IF() | 0x10);

  // update JOYP
  joyp &= 0xf0;
  joyp |= data;
}

void DMG::cycle() {
  ppuTick();
  apuTick();
  joypadTick();

  // run DIV-APU event, if applicable
  if(!(div & 0x07ff)) divAPU();

  // clock serial port, if active
  if(!(div & 0x007f)) {
    if(serialBits && sc == 0x81) {
      sb <<= 1;
      sb |= 0x01;  // if serial port is disconnected, always read 1
      serialBits--;
      if(!serialBits) {
        sc &= 0x7f;
        setIF(IF() | 0x08);
      }
    }
  }

  // run 1 byte of DMA transfer, if active
  if(dmaActive) {
    oam[dmaAddr & 0xff] = readBus(dmaAddr);
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

