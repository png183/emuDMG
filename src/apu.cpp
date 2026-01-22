#include "apu.hpp"

void CH2::writeNRx1(uint8_t data) {
  dutyCycle = data >> 6;
  initLength = data & 0x3f;
  length = initLength;
}

void CH2::writeNRx2(uint8_t data) {
  dacOn = data & 0xf8;
  initVolume = data >> 4;
  crescendo = data & 0x08;
  sweepPace = data & 0x07;
  if(!dacOn) channelOn = false;
}

void CH2::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
}

void CH2::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
  if(dacOn && (data & 0x80)) trigger();
}

void CH2::trigger() {
  channelOn = true;
  volume = initVolume;
  dutyTimer = period;
}

bool CH2::active() {
  return channelOn;
}

int16_t CH2::tick() {
  int16_t sample = 0;
  if(channelOn) {
    dutyTimer++;
    if(dutyTimer == 0x0800) {
      dutyTimer = period;
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

void CH2::clockLength() {
  if(lengthEnable) {
    length = (length + 1) & 0x3f;
    if(!length) channelOn = false;
  }
}

void CH2::clockEnvelope() {
  if(channelOn && sweepPace) {
    sweepStep = (sweepStep + 1) & 0x07;
    if(sweepStep == sweepPace) {
      sweepStep = 0;
      if(crescendo) {
        if(volume != 0x0f) volume++;
      } else {
        if(volume) volume--;
      }
    }
  }
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

void CH4::writeNRx1(uint8_t data) {
  initLength = data & 0x3f;
  length = initLength;
}

void CH4::writeNRx2(uint8_t data) {
  dacOn = data & 0xf8;
  initVolume = data >> 4;
  crescendo = data & 0x08;
  sweepPace = data & 0x07;
  if(!dacOn) channelOn = false;
}

void CH4::writeNRx3(uint8_t data) {
  // todo: bit 3
  clockShift = data >> 4;
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
      if(!(((lfsr >> 1) ^ lfsr) & 0x01)) lfsr |= 0x8000;
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
  if(channelOn && sweepPace) {
    sweepStep = (sweepStep + 1) & 0x07;
    if(sweepStep == sweepPace) {
      sweepStep = 0;
      if(crescendo) {
        if(volume != 0x0f) volume++;
      } else {
        if(volume) volume--;
      }
    }
  }
}

