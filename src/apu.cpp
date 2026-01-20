#include "apu.hpp"

void Channel::writeNRx1(uint8_t data) {
  // todo: implement bits 7-6
  initLength = data & 0x3f;
}

void Channel::writeNRx2(uint8_t data) {
  // todo
}

void Channel::writeNRx3(uint8_t data) {
  period &= 0xff00;
  period |= data;
}

void Channel::writeNRx4(uint8_t data) {
  // todo: implement bit 6
  channelOn = data & 0x80;
  period &= 0x00ff;
  period |= (data & 0x07) << 8;
}

void Channel::start() {
  length = initLength;
  dutyTimer = period;
}

int16_t Channel::tick() {
  static const int16_t envelope[8] = { 0x0800, 0x0800, 0x0800, 0x0800, ~0x0800, ~0x0800, ~0x0800, ~0x0800 };
  int16_t sample = 0;
  if(channelOn) {
    dutyTimer++;
    if(dutyTimer == 0x0800) {
      dutyTimer = period;
      dutyStep = (dutyStep + 1) & 0x07;
    }
    sample = envelope[dutyStep];
  }
  return sample;
}

void Channel::clockLength() {
  if(channelOn) {
    length = (length + 1) & 0x3f;
    channelOn = length;
  }
}
