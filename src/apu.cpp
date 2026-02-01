#include "apu.hpp"

uint8_t APU::NR52() {
  uint8_t data = 0x70;
  if(nr52) data |= 0x80;
  if(ch4.active()) data |= 0x08;
  if(ch3.active()) data |= 0x04;
  if(ch2.active()) data |= 0x02;
  if(ch1.active()) data |= 0x01;
  return data;
}

uint8_t APU::apuReadIO(uint16_t addr) {
  if(addr == 0xff10) return ch1.readNRx0();  // NR10
  if(addr == 0xff11) return ch1.readNRx1();  // NR11
  if(addr == 0xff12) return ch1.readNRx2();  // NR12
  if(addr == 0xff14) return ch1.readNRx4();  // NR14
  if(addr == 0xff16) return ch2.readNRx1();  // NR21
  if(addr == 0xff17) return ch2.readNRx2();  // NR22
  if(addr == 0xff19) return ch2.readNRx4();  // NR24
  if(addr == 0xff1a) return ch3.readNRx0();  // NR30
  if(addr == 0xff1c) return ch3.readNRx2();  // NR32
  if(addr == 0xff1e) return ch3.readNRx4();  // NR34
  if(addr == 0xff21) return ch4.readNRx2();  // NR42
  if(addr == 0xff22) return ch4.readNRx3();  // NR43
  if(addr == 0xff23) return ch4.readNRx4();  // NR44
  if(addr == 0xff24) return nr50;  // NR50
  if(addr == 0xff25) return nr51;  // NR51
  if(addr == 0xff26) return NR52();  // NR52
  if(addr >= 0xff30 && addr < 0xff40) return ch3.readRAM(addr);  // wave RAM
  return 0xff;
}

void APU::apuWriteIO(uint16_t addr, uint8_t data) {
  if(addr == 0xff10) { ch1.writeNRx0(data); return; }  // NR10
  if(addr == 0xff11) { ch1.writeNRx1(data); return; }  // NR11
  if(addr == 0xff12) { ch1.writeNRx2(data); return; }  // NR12
  if(addr == 0xff13) { ch1.writeNRx3(data); return; }  // NR13
  if(addr == 0xff14) { ch1.writeNRx4(data); return; }  // NR14
  if(addr == 0xff16) { ch2.writeNRx1(data); return; }  // NR21
  if(addr == 0xff17) { ch2.writeNRx2(data); return; }  // NR22
  if(addr == 0xff18) { ch2.writeNRx3(data); return; }  // NR23
  if(addr == 0xff19) { ch2.writeNRx4(data); return; }  // NR24
  if(addr == 0xff1a) { ch3.writeNRx0(data); return; }  // NR30
  if(addr == 0xff1b) { ch3.writeNRx1(data); return; }  // NR31
  if(addr == 0xff1c) { ch3.writeNRx2(data); return; }  // NR32
  if(addr == 0xff1d) { ch3.writeNRx3(data); return; }  // NR33
  if(addr == 0xff1e) { ch3.writeNRx4(data); return; }  // NR34
  if(addr == 0xff20) { ch4.writeNRx1(data); return; }  // NR41
  if(addr == 0xff21) { ch4.writeNRx2(data); return; }  // NR42
  if(addr == 0xff22) { ch4.writeNRx3(data); return; }  // NR43
  if(addr == 0xff23) { ch4.writeNRx4(data); return; }  // NR44
  if(addr == 0xff24) { nr50 = data; return; }  // NR50
  if(addr == 0xff25) { nr51 = data; return; }  // NR51
  if(addr == 0xff26) { nr52 = data & 0x80; return; }  // NR52
  if(addr >= 0xff30 && addr < 0xff40) { ch3.writeRAM(addr, data); return; }  // wave RAM
}

void APU::apuTick() {
  int16_t sample = 0;
  if(nr52) {
    sample += ch1.tick();
    sample += ch2.tick();
    sample += ch3.tick(); ch3.tick();  // channel 3 runs twice as fast
    sample += ch4.tick();
  }
  emitSample(sample);
}

void APU::divAPU() {
  static uint8_t subdiv = 0;
  if(!(subdiv & 0x01)) {
    ch1.clockLength();
    ch2.clockLength();
    ch3.clockLength();
    ch4.clockLength();
  }
  if(!(subdiv & 0x03)) ch1.clockSweep();
  if(!(subdiv & 0x07)) {
    ch1.clockEnvelope();
    ch2.clockEnvelope();
    ch4.clockEnvelope();
  }
  subdiv++;
}

uint8_t CH1::readNRx0() {
  uint8_t data = 0x80;
  data |= sweepPace << 4;
  if(sweepDirection) data |= 0x08;
  data |= sweepSize;
  return data;
}

uint8_t CH1::readNRx1() {
  return (dutyCycle << 6) | 0x3f;
}

uint8_t CH1::readNRx2() {
  uint8_t data = 0x00;
  data |= initVolume << 4;
  if(crescendo) data |= 0x08;
  data |= envelopePace;
  return data;
}

uint8_t CH1::readNRx4() {
  uint8_t data = 0xbf;
  if(lengthEnable) data |= 0x40;
  return data;
}

void CH1::writeNRx0(uint8_t data) {
  sweepPace = (data >> 4) & 0x07;
  sweepDirection = data & 0x08;
  sweepSize = data & 0x07;
}

void CH1::writeNRx1(uint8_t data) {
  dutyCycle = data >> 6;
  initLength = data & 0x3f;
  length = initLength;
}

void CH1::writeNRx2(uint8_t data) {
  dacOn = data & 0xf8;
  initVolume = data >> 4;
  crescendo = data & 0x08;
  envelopePace = data & 0x07;
  if(!dacOn) channelOn = false;
}

void CH1::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
  if(!sweepPace) activePeriod = period;
}

void CH1::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
  if(!sweepPace) activePeriod = period;
  if(dacOn && (data & 0x80)) trigger();
}

void CH1::trigger() {
  channelOn = true;
  volume = initVolume;
  activePeriod = period;
  dutyTimer = activePeriod;
}

bool CH1::active() {
  return channelOn;
}

int16_t CH1::tick() {
  int16_t sample = 0;
  if(channelOn) {
    dutyTimer++;
    if(dutyTimer == 0x0800) {
      dutyTimer = activePeriod;
      dutyStep = (dutyStep + 1) & 0x07;
    }
    bool isHigh = false;
    switch(dutyCycle) {
    case 0x00: isHigh = (                 dutyStep != 7); break;
    case 0x01: isHigh = (dutyStep != 0 && dutyStep != 7); break;
    case 0x02: isHigh = (dutyStep >  0 && dutyStep <= 4); break;
    case 0x03: isHigh = (dutyStep == 0 || dutyStep == 7); break;
    }
    uint8_t data = isHigh ? volume : 0;
    sample = ((data << 1) - 0x0f) * 0x0080;
  }
  return sample;
}

void CH1::clockLength() {
  if(lengthEnable) {
    length = (length + 1) & 0x3f;
    if(!length) channelOn = false;
  }
}

void CH1::clockSweep() {
  if(channelOn && sweepPace) {
    sweepStep = (sweepStep + 1) & 0x07;
    if(sweepStep == sweepPace) {
      sweepStep = 0x00;
      uint16_t adjustment = activePeriod >> sweepSize;
      activePeriod = sweepDirection ? (activePeriod - adjustment) : (activePeriod + adjustment);
      if(activePeriod >= 0x0800) channelOn = false;
    }
  }
}

void CH1::clockEnvelope() {
  if(channelOn && envelopePace) {
    envelopeStep = (envelopeStep + 1) & 0x07;
    if(envelopeStep == envelopePace) {
      envelopeStep = 0x00;
      if(crescendo) {
        if(volume != 0x0f) volume++;
      } else {
        if(volume) volume--;
      }
    }
  }
}

uint8_t CH3::readNRx0() {
  return dacOn ? 0xff : 0x7f;
}

uint8_t CH3::readNRx2() {
  return 0x9f | (volume << 5);
}

uint8_t CH3::readNRx4() {
  uint8_t data = 0xbf;
  if(lengthEnable) data |= 0x40;
  return data;
}

void CH3::writeNRx0(uint8_t data) {
  dacOn = data & 0x80;
  if(!dacOn) channelOn = false;
}

void CH3::writeNRx1(uint8_t data) {
  initLength = data;
  length = initLength;
}

void CH3::writeNRx2(uint8_t data) {
  volume = (data >> 5) & 0x03;
}

void CH3::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
}

void CH3::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
  if(dacOn && (data & 0x80)) trigger();
}

uint8_t CH3::readRAM(uint16_t addr) {
  return ram[addr & 0x0f];
}

void CH3::writeRAM(uint16_t addr, uint8_t data) {
  ram[addr & 0x0f] = data;
}

void CH3::trigger() {
  channelOn = true;
  dutyTimer = period;
}

bool CH3::active() {
  return channelOn;
}

int16_t CH3::tick() {
  int16_t sample = 0;
  if(channelOn) {
    dutyTimer++;
    if(dutyTimer == 0x0800) {
      dutyTimer = period;
      index = (index + 1) & 0x1f;
    }
    uint8_t data = (ram[index >> 1] >> ((index & 1) ? 0 : 4)) & 0x0f;
    if(volume == 0x00) data = 0;
    if(volume == 0x02) data >>= 1;
    if(volume == 0x03) data >>= 2;
    sample = ((data << 1) - 0x0f) * 0x0080;
  }
  return sample;
}

void CH3::clockLength() {
  if(lengthEnable) {
    length++;
    if(!length) channelOn = false;
  }
}

uint8_t CH4::readNRx2() {
  uint8_t data = 0x00;
  data |= initVolume << 4;
  if(crescendo) data |= 0x08;
  data |= envelopePace;
  return data;
}

uint8_t CH4::readNRx3() {
  uint8_t data = 0x00;
  data |= clockShift << 4;
  if(lfsrWidth) data |= 0x08;
  data |= clockDivider;
  return data;
}

uint8_t CH4::readNRx4() {
  uint8_t data = 0xbf;
  if(lengthEnable) data |= 0x40;
  return data;
}

void CH4::writeNRx1(uint8_t data) {
  initLength = data & 0x3f;
  length = initLength;
}

void CH4::writeNRx2(uint8_t data) {
  dacOn = data & 0xf8;
  initVolume = data >> 4;
  crescendo = data & 0x08;
  envelopePace = data & 0x07;
  if(!dacOn) channelOn = false;
}

void CH4::writeNRx3(uint8_t data) {
  clockShift = data >> 4;
  lfsrWidth = data & 0x08;
  clockDivider = data & 0x07;
}

void CH4::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  if(dacOn && (data & 0x80)) trigger();
}

void CH4::trigger() {
  channelOn = true;
  volume = initVolume;
  clockTimer = 0x00000000;
  lfsr = 0x0000;
}

bool CH4::active() {
  return channelOn;
}

int16_t CH4::tick() {
  int16_t sample = 0;
  if(channelOn) {
    clockTimer++;
    uint32_t counterTarget = (clockDivider ? (clockDivider << 2) : 2) * (2 << clockShift);
    if(clockTimer >= counterTarget) {
      clockTimer = 0x00000000;
      dutyStep = (dutyStep + 1) & 0x07;
      uint16_t newBit = (!(((lfsr >> 1) ^ lfsr) & 0x01)) ? 0x8000 : 0x0000;
      lfsr |= newBit;
      if(lfsrWidth) {
        lfsr &= 0xff7f;
        lfsr |= newBit >> 8;
      }
      lfsr >>= 1;
    }
    uint8_t data = (lfsr & 0x01) ? volume : 0;
    sample = ((data << 1) - 0x0f) * 0x0080;
  }
  return sample;
}

void CH4::clockLength() {
  if(lengthEnable) {
    length = (length + 1) & 0x3f;
    if(!length) channelOn = false;
  }
}

void CH4::clockEnvelope() {
  if(channelOn && envelopePace) {
    envelopeStep = (envelopeStep + 1) & 0x07;
    if(envelopeStep == envelopePace) {
      envelopeStep = 0x00;
      if(crescendo) {
        if(volume != 0x0f) volume++;
      } else {
        if(volume) volume--;
      }
    }
  }
}

