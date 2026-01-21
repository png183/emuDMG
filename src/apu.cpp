#include "apu.hpp"

void Channel::writeNRx1(uint8_t data) {
  dutyCycle = data >> 6;
  initLength = data & 0x3f;
  length = initLength;
}

void Channel::writeNRx2(uint8_t data) {
  dacOn = data & 0xf8;
  initVolume = data >> 4;
  crescendo = data & 0x08;
  sweepPace = data & 0x07;
  if(!dacOn) channelOn = false;
}

void Channel::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
}

void Channel::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
  if(dacOn && (data & 0x80)) trigger();
}

void Channel::trigger() {
  channelOn = true;
  volume = initVolume;
  dutyTimer = period;
}

bool Channel::active() {
  return channelOn;
}

int16_t Channel::tick() {
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
    sample = volume * 0x0080 * (isHigh ? 1 : -1);
  }
  return sample;
}

void Channel::clockLength() {
  if(lengthEnable) {
    length = (length + 1) & 0x3f;
    if(!length) channelOn = false;
  }
}

void Channel::clockEnvelope() {
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
