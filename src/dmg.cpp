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

  // reset APU
  nr52 = false;
}

uint8_t DMG::JOYP() {
  // todo: is JOYP = 0x00 handled correctly?
  // todo: are bits 7 and 6 handled correctly?
  uint8_t data = 0xcf | joyp;
  if(!(joyp & 0x20)) data &= pollButtons();
  if(!(joyp & 0x10)) data &= pollDpad();
  return data;
}

uint8_t DMG::NR52() {
  uint8_t data = 0x70;
  if(nr52) data |= 0x80;
  if(ch4.active()) data |= 0x08;
  if(ch3.active()) data |= 0x04;
  if(ch2.active()) data |= 0x02;
  if(ch1.active()) data |= 0x01;
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

uint8_t DMG::readAPU(uint16_t addr) {
  if(addr == 0xff25) return nr51;  // NR51
  if(addr == 0xff26) return NR52();  // NR52
  if(addr >= 0xff30 && addr < 0xff40) return ch3.readRAM(addr);  // wave RAM
  printf("TODO: Reading address 0x%04x\n", addr);
  return 0xff;
}

void DMG::writeAPU(uint16_t addr, uint8_t data) {
//  if(addr == 0xff10) { printf("TODO: NR10 write\n"); return; }
  if(addr == 0xff11) { ch1.writeNRx1(data); return; }
  if(addr == 0xff12) { ch1.writeNRx2(data); return; }
  if(addr == 0xff13) { ch1.writeNRx3(data); return; }
  if(addr == 0xff14) { ch1.writeNRx4(data); return; }
  if(addr == 0xff16) { ch2.writeNRx1(data); return; }
  if(addr == 0xff17) { ch2.writeNRx2(data); return; }
  if(addr == 0xff18) { ch2.writeNRx3(data); return; }
  if(addr == 0xff19) { ch2.writeNRx4(data); return; }
  if(addr == 0xff1a) { ch3.writeNRx0(data); return; }
  if(addr == 0xff1b) { ch3.writeNRx1(data); return; }
  if(addr == 0xff1c) { ch3.writeNRx2(data); return; }
  if(addr == 0xff1d) { ch3.writeNRx3(data); return; }
  if(addr == 0xff1e) { ch3.writeNRx4(data); return; }
  if(addr == 0xff20) { ch4.writeNRx1(data); return; }
  if(addr == 0xff21) { ch4.writeNRx2(data); return; }
  if(addr == 0xff22) { ch4.writeNRx3(data); return; }
  if(addr == 0xff23) { ch4.writeNRx4(data); return; }
//  if(addr == 0xff24) { printf("TODO: NR50 write\n"); return; }
  if(addr == 0xff25) { nr51 = data; return; }  // NR51
  if(addr == 0xff26) { nr52 = data & 0x80; return; }  // NR52
  if(addr >= 0xff30 && addr < 0xff40) { ch3.writeRAM(addr, data); return; }  // wave RAM
}

void DMG::apuTick() {
  int16_t sample = 0;
  if(nr52) {
    sample += ch1.tick();
    sample += ch2.tick();
    sample += ch3.tick(); ch3.tick();  // channel 3 runs twice as fast
    sample += ch4.tick();
  }
  emitSample(sample);
}

void DMG::divAPU() {
  static uint8_t subdiv = 0;
  if(!(subdiv & 0x01)) {
    ch1.clockLength();
    ch2.clockLength();
    ch3.clockLength();
    ch4.clockLength();
  }
  if(!(subdiv & 0x07)) {
    ch1.clockEnvelope();
    ch2.clockEnvelope();
    ch4.clockEnvelope();
  }
  subdiv++;
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
  if(addr >= 0xff10 && addr < 0xff40) return readAPU(addr);  // APU I/O
  if(addr >= 0xff40 && addr < 0xff46) return ppuReadIO(addr);  // PPU I/O
  if(addr == 0xff46) return dma;  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) return ppuReadIO(addr);  // PPU I/O
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
  if(addr >= 0xff10 && addr < 0xff40) { writeAPU(addr, data); return; }  // APU I/O
  if(addr >= 0xff40 && addr < 0xff46) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff46) { DMA(data); return; }  // DMA
  if(addr >= 0xff47 && addr < 0xff4c) { ppuWriteIO(addr, data); return; }  // PPU I/O
  if(addr == 0xff50) { boot |= (data & 0x01); return; }  // BOOT
  if(addr >= 0xff80 && addr < 0xffff) { hram[addr & 0x7f] = data; return; }  // HRAM
  if(addr == 0xffff) { setIE(data); return; }  // IE
}

void DMG::cycle() {
  ppuTick();
  apuTick();

  // run DIV-APU event if applicable
  if(!(div & 0x07ff)) divAPU();

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

