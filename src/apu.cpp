#include "apu.hpp"

void Channel::writeNRx1(uint8_t data) {
  // todo: implement bits 7-6
  initLength = data & 0x3f;
  length = initLength;
}

void Channel::writeNRx2(uint8_t data) {
  // todo: implement bit 3
  initVolume = data >> 4;
  sweepPace = data & 0x07;
}

void Channel::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
}

void Channel::writeNRx4(uint8_t data) {
  lengthEnable = data & 0x40;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
  if(data & 0x80) trigger();
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
  static const int16_t envelope[8] = { 1, 1, 1, 1, -1, -1, -1, -1 };
  int16_t sample = 0;
  if(channelOn) {
    dutyTimer++;
    if(dutyTimer == 0x0800) {
      dutyTimer = period;
      dutyStep = (dutyStep + 1) & 0x07;
    }
    sample = volume * 0x0080 * envelope[dutyStep];
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
  // todo: respect bit 3 of NRx2
  if(channelOn && sweepPace) {
    sweepStep = (sweepStep + 1) & 0x07;
    if(sweepStep == sweepPace) {
      sweepStep = 0;
      if(volume) volume--;
    }
  }
}
