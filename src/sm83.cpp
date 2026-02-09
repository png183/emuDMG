#include "sm83.hpp"

void SM83::reset() {
  pc = 0x0000;
  ime[0] = false;
  ime[1] = false;
  setIF(0x00);
  setIE(0x00);
}

void SM83::instruction() {
  if(ime[0] && (_if & _ie)) {
    ime[0] = false;
    ime[1] = false;
    if(_if & _ie & 0x01) { _if &= ~0x01; runISR(0x0040); return; }
    if(_if & _ie & 0x02) { _if &= ~0x02; runISR(0x0048); return; }
    if(_if & _ie & 0x04) { _if &= ~0x04; runISR(0x0050); return; }
    if(_if & _ie & 0x08) { _if &= ~0x08; runISR(0x0058); return; }
    if(_if & _ie & 0x10) { _if &= ~0x10; runISR(0x0060); return; }
  }
  ime[0] = ime[1];

  ir = fetch8();
  switch(ir) {

  // 00-3f: misc. ops
  case 0x00:                              return;  // NOP
  case 0x01: c = fetch8(); b = fetch8();  return;  // LD BC,nn
  case 0x02: cycleWrite(b << 8 | c, a);   return;  // LD (BC),A
  case 0x03: if(!(++c)) b++; cycleIdle(); return;  // INC BC
  case 0x04: return INC();
  case 0x05: return DEC();
  case 0x06: return regDstWrite(fetch8());
  case 0x07: return RLCA();
  case 0x08: write16(fetch16(), sp);      return;  // LD (nn),SP
  case 0x09: ADD16(b << 8 | c);           return;  // ADD HL,BC
  case 0x0a: a = cycleRead(b << 8 | c);   return;  // LD A,(BC)
  case 0x0b: cycleIdle(); if(!(c--)) b--; return;  // DEC BC
  case 0x0c: return INC();
  case 0x0d: return DEC();
  case 0x0e: return regDstWrite(fetch8());
  case 0x0f: return RRCA();
  case 0x10: STOP();                      return;  // STOP
  case 0x11: e = fetch8(); d = fetch8();  return;  // LD DE,nn
  case 0x12: cycleWrite(d << 8 | e, a);   return;  // LD (DE),A
  case 0x13: if(!(++e)) d++; cycleIdle(); return;  // INC DE
  case 0x14: return INC();
  case 0x15: return DEC();
  case 0x16: return regDstWrite(fetch8());
  case 0x17: return RLA();
  case 0x18: JR(true);                    return;  // JR e
  case 0x19: ADD16(d << 8 | e);           return;  // ADD HL,DE
  case 0x1a: a = cycleRead(d << 8 | e);   return;  // LD A,(DE)
  case 0x1b: cycleIdle(); if(!(e--)) d--; return;  // DEC DE
  case 0x1c: return INC();
  case 0x1d: return DEC();
  case 0x1e: return regDstWrite(fetch8());
  case 0x1f: return RRA();
  case 0x20: JR(!ZF());                   return;  // JR NZ,e
  case 0x21: l = fetch8(); h = fetch8();  return;  // LD HL,nn
  case 0x22: cycleWrite(incHL(), a);      return;  // LD (HL+),A
  case 0x23: if(!(++l)) h++; cycleIdle(); return;  // INC HL
  case 0x24: return INC();
  case 0x25: return DEC();
  case 0x26: return regDstWrite(fetch8());
  case 0x27: return DAA();
  case 0x28: JR(ZF());                    return;  // JR Z,e
  case 0x29: ADD16(HL());                 return;  // ADD HL,HL
  case 0x2a: a = cycleRead(incHL());      return;  // LD A,(HL+)
  case 0x2b: cycleIdle(); if(!(l--)) h--; return;  // DEC HL
  case 0x2c: return INC();
  case 0x2d: return DEC();
  case 0x2e: return regDstWrite(fetch8());
  case 0x2f: return CPL();
  case 0x30: JR(!CF());                   return;  // JR NC,e
  case 0x31: sp = fetch16();              return;  // LD SP,nn
  case 0x32: cycleWrite(decHL(), a);      return;  // LD (HL-),A
  case 0x33: sp++; cycleIdle();           return;  // INC SP
  case 0x34: return INC();
  case 0x35: return DEC();
  case 0x36: return regDstWrite(fetch8());
  case 0x37: return SCF();
  case 0x38: JR(CF());                    return;  // JR C,e
  case 0x39: ADD16(sp);                   return;  // ADD HL,SP
  case 0x3a: a = cycleRead(decHL());      return;  // LD A,(HL-)
  case 0x3b: cycleIdle(); sp--;           return;  // DEC SP
  case 0x3c: return INC();
  case 0x3d: return DEC();
  case 0x3e: return regDstWrite(fetch8());
  case 0x3f: return CCF();

  // 40-7f: LD instruction
  case 0x40 ... 0x75: return regDstWrite(regSrcRead());
  case 0x76:          return HALT();
  case 0x77 ... 0x7f: return regDstWrite(regSrcRead());

  // 80-bf: ALU ops
  case 0x80 ... 0x87: return ADD(regSrcRead());
  case 0x88 ... 0x8f: return ADC(regSrcRead());
  case 0x90 ... 0x97: return SUB(regSrcRead());
  case 0x98 ... 0x9f: return SBC(regSrcRead());
  case 0xa0 ... 0xa7: return AND(regSrcRead());
  case 0xa8 ... 0xaf: return XOR(regSrcRead());
  case 0xb0 ... 0xb7: return  OR(regSrcRead());
  case 0xb8 ... 0xbf: return  CP(regSrcRead());

  // c0-ff: control flow
  case 0xc0: cycleIdle(); RET(!ZF());           return;  // RET NZ
  case 0xc1: c = pop8(); b = pop8();            return;  // POP BC
  case 0xc2: JP(!ZF());                         return;  // JP NZ,nn
  case 0xc3: JP(true);                          return;  // JP nn
  case 0xc4: CALL(!ZF());                       return;  // CALL NZ,nn
  case 0xc5: cycleIdle(); push8(b); push8(c);   return;  // PUSH BC
  case 0xc6: ADD(fetch8());                     return;  // ADD n
  case 0xc7: RST(0x0000);                       return;  // RST 0x00
  case 0xc8: cycleIdle(); RET(ZF());            return;  // RET Z
  case 0xc9: RET(true);                         return;  // RET
  case 0xca: JP(ZF());                          return;  // JP Z,nn
  case 0xcb: instructionCB();                   return;  // CB-prefixed instruction
  case 0xcc: CALL(ZF());                        return;  // CALL Z,nn
  case 0xcd: CALL(true);                        return;  // CALL nn
  case 0xce: ADC(fetch8());                     return;  // ADC n
  case 0xcf: RST(0x0008);                       return;  // RST 0x08
  case 0xd0: cycleIdle(); RET(!CF());           return;  // RET NC
  case 0xd1: e = pop8(); d = pop8();            return;  // POP DE
  case 0xd2: JP(!CF());                         return;  // JP NC,nn
  case 0xd3: HCF();                             return;
  case 0xd4: CALL(!CF());                       return;  // CALL NC,nn
  case 0xd5: cycleIdle(); push8(d); push8(e);   return;  // PUSH DE
  case 0xd6: SUB(fetch8());                     return;  // SUB n
  case 0xd7: RST(0x0010);                       return;  // RST 0x10
  case 0xd8: cycleIdle(); RET(CF());            return;  // RET C
  case 0xd9: RETI();                            return;  // RETI
  case 0xda: JP(CF());                          return;  // JP C,nn
  case 0xdb: HCF();                             return;
  case 0xdc: CALL(CF());                        return;  // CALL C,nn
  case 0xdd: HCF();                             return;
  case 0xde: SBC(fetch8());                     return;  // SBC n
  case 0xdf: RST(0x0018);                       return;  // RST 0x18
  case 0xe0: cycleWrite(0xff00 + fetch8(), a);  return;  // LDH (n),A
  case 0xe1: l = pop8(); h = pop8();            return;  // POP HL
  case 0xe2: cycleWrite(0xff00 + c, a);         return;  // LDH (C),A
  case 0xe3: HCF();                             return;
  case 0xe4: HCF();                             return;
  case 0xe5: cycleIdle(); push8(h); push8(l);   return;  // PUSH HL
  case 0xe6: AND(fetch8());                     return;  // AND n
  case 0xe7: RST(0x0020);                       return;  // RST 0x20
  case 0xe8: sp = addSP(); cycleIdle();         return;  // ADD SP,e
  case 0xe9: pc = HL();                         return;  // JP HL
  case 0xea: cycleWrite(fetch16(), a);          return;  // LD (nn),A
  case 0xeb: HCF();                             return;
  case 0xec: HCF();                             return;
  case 0xed: HCF();                             return;
  case 0xee: XOR(fetch8());                     return;  // XOR n
  case 0xef: RST(0x0028);                       return;  // RST 0x28
  case 0xf0: a = cycleRead(0xff00 + fetch8());  return;  // LDH A,(n)
  case 0xf1: f = pop8() & 0xf0; a = pop8();     return;  // POP AF
  case 0xf2: a = cycleRead(0xff00 + c);         return;  // LDH A,(C)
  case 0xf3: DI();                              return;  // DI
  case 0xf4: HCF();                             return;
  case 0xf5: cycleIdle(); push8(a); push8(f);   return;  // PUSH AF
  case 0xf6: OR(fetch8());                      return;  // OR n
  case 0xf7: RST(0x0030);                       return;  // RST 0x30
  case 0xf8: setHL(addSP());                    return;  // LD HL,SP+e
  case 0xf9: sp = HL(); cycleIdle();            return;  // LD SP,HL
  case 0xfa: a = cycleRead(fetch16());          return;  // LD A,(nn)
  case 0xfb: EI();                              return;  // EI
  case 0xfc: HCF();                             return;
  case 0xfd: HCF();                             return;
  case 0xfe: CP(fetch8());                      return;  // CP n
  case 0xff: RST(0x0038);                       return;  // RST 0x38

  }

  // unreachable
}

void SM83::instructionCB() {
  ir = fetch8();
  switch(ir) {
  case 0x00 ... 0x07: return RLC();
  case 0x08 ... 0x0f: return RRC();
  case 0x10 ... 0x17: return RL();
  case 0x18 ... 0x1f: return RR();
  case 0x20 ... 0x27: return SLA();
  case 0x28 ... 0x2f: return SRA();
  case 0x30 ... 0x37: return SWAP();
  case 0x38 ... 0x3f: return SRL();
  case 0x40 ... 0x7f: return BIT();
  case 0x80 ... 0xbf: return RES();
  case 0xc0 ... 0xff: return SET();
  }

  // unreachable
}

uint8_t SM83::regSrcRead() {
  switch(ir & 0x07) {
  case 0x00: return b;
  case 0x01: return c;
  case 0x02: return d;
  case 0x03: return e;
  case 0x04: return h;
  case 0x05: return l;
  case 0x06: return cycleRead(HL());
  case 0x07: return a;
  }

  // unreachable
  return 0xff;
}

uint8_t SM83::regDstRead() {
  switch((ir >> 3) & 0x07) {
  case 0x00: return b;
  case 0x01: return c;
  case 0x02: return d;
  case 0x03: return e;
  case 0x04: return h;
  case 0x05: return l;
  case 0x06: return cycleRead(HL());
  case 0x07: return a;
  }

  // unreachable
  return 0xff;
}

void SM83::regSrcWrite(uint8_t data) {
  switch(ir & 0x07) {
  case 0x00: b = data;               return;
  case 0x01: c = data;               return;
  case 0x02: d = data;               return;
  case 0x03: e = data;               return;
  case 0x04: h = data;               return;
  case 0x05: l = data;               return;
  case 0x06: cycleWrite(HL(), data); return;
  case 0x07: a = data;               return;
  }

  // unreachable
}

void SM83::regDstWrite(uint8_t data) {
  switch((ir >> 3) & 0x07) {
  case 0x00: b = data;               return;
  case 0x01: c = data;               return;
  case 0x02: d = data;               return;
  case 0x03: e = data;               return;
  case 0x04: h = data;               return;
  case 0x05: l = data;               return;
  case 0x06: cycleWrite(HL(), data); return;
  case 0x07: a = data;               return;
  }

  // unreachable
}

void SM83::runISR(uint16_t addr) {
  cycleIdle();
  cycleIdle();
  push16(pc);
  cycleIdle();
  pc = addr;
}

void SM83::write16(uint16_t addr, uint16_t data) {
  cycleWrite(addr, data);
  cycleWrite(addr + 1, data >> 8);
}

uint8_t SM83::fetch8() {
  return cycleRead(pc++);
}

uint16_t SM83::fetch16() {
  uint16_t data = cycleRead(pc++);
  return cycleRead(pc++) << 8 | data;
}

void SM83::push8(uint8_t data) {
  cycleWrite(--sp, data);
}

void SM83::push16(uint16_t data) {
  push8(data >> 8);
  push8(data);
}

uint8_t SM83::pop8() {
  return cycleRead(sp++);
}

uint16_t SM83::pop16() {
  uint16_t data = pop8();
  return pop8() << 8 | data;
}

uint16_t SM83::HL() {
  return h << 8 | l;
}

void SM83::setHL(uint16_t data) {
  h = data >> 8;
  l = data;
}

uint16_t SM83::incHL() {
  uint16_t hlPrev = HL();
  uint16_t hl = hlPrev + 1;
  l = hl;
  h = hl >> 8;
  return hlPrev;
}

uint16_t SM83::decHL() {
  uint16_t hlPrev = HL();
  uint16_t hl = hlPrev - 1;
  l = hl;
  h = hl >> 8;
  return hlPrev;
}

uint16_t SM83::addSP() {
  uint16_t data = (int16_t)(int8_t)fetch8();
  uint8_t findH = (sp & 0x000f) + (data & 0x000f);
  uint16_t findC = (sp & 0x00ff) + (data & 0x00ff);
  uint16_t result = sp + data;
  setZ(false);
  setN(false);
  setH(findH & 0x10);
  setC(findC & 0x100);
  cycleIdle();
  return result;
}

void SM83::setZ(bool cond) {
  f &= 0x7f;
  if(cond) f |= 0x80;
}

void SM83::setN(bool cond) {
  f &= 0xbf;
  if(cond) f |= 0x40;
}

void SM83::setH(bool cond) {
  f &= 0xdf;
  if(cond) f |= 0x20;
}

void SM83::setC(bool cond) {
  f &= 0xef;
  if(cond) f |= 0x10;
}

bool SM83::ZF() {
  return f & 0x80;
}

bool SM83::NF() {
  return f & 0x40;
}

bool SM83::HF() {
  return f & 0x20;
}

bool SM83::CF() {
  return f & 0x10;
}

void SM83::INC() {
  uint8_t data = regDstRead();
  uint8_t dataPrev = data++;
  setZ(data == 0);
  setN(false);
  setH((dataPrev & 0x10) != (data & 0x10));
  regDstWrite(data);
}

void SM83::DEC() {
  uint8_t data = regDstRead();
  uint8_t dataPrev = data--;
  setZ(data == 0);
  setN(true);
  setH((dataPrev & 0x10) != (data & 0x10));
  regDstWrite(data);
}

void SM83::RLCA() {
  bool carry = a & 0x80;
  a <<= 1;
  if(carry) a |= 0x01;
  setZ(false);
  setN(false);
  setH(false);
  setC(carry);
}

void SM83::RRCA() {
  bool carry = a & 0x01;
  a >>= 1;
  if(carry) a |= 0x80;
  setZ(false);
  setN(false);
  setH(false);
  setC(carry);
}

void SM83::RLA() {
  bool carry = a & 0x80;
  a <<= 1;
  if(CF()) a |= 0x01;
  setZ(false);
  setN(false);
  setH(false);
  setC(carry);
}

void SM83::RRA() {
  bool carry = a & 0x01;
  a >>= 1;
  if(CF()) a |= 0x80;
  setZ(false);
  setN(false);
  setH(false);
  setC(carry);
}

void SM83::ADD16(uint16_t data) {
  uint16_t partial = (HL() & 0x0fff) + (data & 0x0fff);
  uint32_t result = HL() + data;
  setN(false);
  setH(partial & 0x1000);
  setC(result & 0x10000);
  h = result >> 8;
  l = result;
  cycleIdle();
}

void SM83::STOP() {
  // todo: implement cases where STOP acts differently from a NOP
}

void SM83::JR(bool cond) {
  int16_t displacement = (int8_t)fetch8();
  if(cond) {
    pc += displacement;
    cycleIdle();
  }
}

void SM83::DAA() {
  uint8_t data = 0x00;
  if(NF()) {
    if(HF()) data += 0x06;
    if(CF()) data += 0x60;
    a += ~data + 1;
    setZ(a == 0);
    setH(0);
  } else {
    if(HF() || ((a & 0x0f) > 0x09)) data += 0x06;
    if(CF() || (a > 0x99)) data += 0x60;
    uint16_t result = a + data;
    a = result;
    setZ(a == 0);
    setH(0);
    setC(CF() || (result & 0x0100));
  }
}

void SM83::CPL() {
  a = ~a;
  setN(true);
  setH(true);
}

void SM83::SCF() {
  setN(false);
  setH(false);
  setC(true);
}

void SM83::CCF() {
  setN(false);
  setH(false);
  setC(!CF());
}

void SM83::HALT() {
  while(!(_if & _ie)) cycleIdle();
}

void SM83::ADD(uint8_t data) {
  uint8_t partial = (a & 0x0f) + (data & 0x0f);
  uint16_t result = a + data;
  setZ((uint8_t)result == 0);
  setN(false);
  setH(partial & 0x10);
  setC(result & 0x100);
  a = result;
}

void SM83::ADC(uint8_t data) {
  uint8_t partial = (a & 0x0f) + (data & 0x0f);
  uint16_t result = a + data;
  if(CF()) {
    partial++;
    result++;
  }
  setZ((uint8_t)result == 0);
  setN(false);
  setH(partial & 0x10);
  setC(result & 0x100);
  a = result;
}

void SM83::SUB(uint8_t data) {
  data = ~data;
  uint8_t partial = (a & 0x0f) + (data & 0x0f) + 1;
  uint16_t result = a + data + 1;
  setZ((uint8_t)result == 0);
  setN(true);
  setH(!(partial & 0x10));
  setC(!(result & 0x100));
  a = result;
}

void SM83::SBC(uint8_t data) {
  data = ~data;
  uint8_t partial = (a & 0x0f) + (data & 0x0f);
  uint16_t result = a + data;
  if(!CF()) {
    partial++;
    result++;
  }
  setZ((uint8_t)result == 0);
  setN(true);
  setH(!(partial & 0x10));
  setC(!(result & 0x100));
  a = result;
}

void SM83::AND(uint8_t data) {
  a &= data;
  setZ(a == 0);
  setN(false);
  setH(true);
  setC(false);
}

void SM83::XOR(uint8_t data) {
  a ^= data;
  setZ(a == 0);
  setN(false);
  setH(false);
  setC(false);
}

void SM83::OR(uint8_t data) {
  a |= data;
  setZ(a == 0);
  setN(false);
  setH(false);
  setC(false);
}

void SM83::CP(uint8_t data) {
  data = ~data;
  uint8_t partial = (a & 0x0f) + (data & 0x0f) + 1;
  uint16_t result = a + data + 1;
  setZ((uint8_t)result == 0);
  setN(true);
  setH(!(partial & 0x10));
  setC(!(result & 0x100));
}

void SM83::RET(bool cond) {
  if(cond) {
    pc = pop16();
    cycleIdle();
  }
}

void SM83::JP(bool cond) {
  uint16_t target = fetch16();
  if(cond) {
    pc = target;
    cycleIdle();
  }
}

void SM83::CALL(bool cond) {
  uint16_t target = fetch16();
  if(cond) {
    cycleIdle();
    push16(pc);
    pc = target;
  }
}

void SM83::RST(uint16_t addr) {
  cycleIdle();
  push16(pc);
  pc = addr;
}

void SM83::RETI() {
  ime[0] = true;
  ime[1] = true;
  pc = pop16();
  cycleIdle();
}

void SM83::DI() {
  ime[0] = false;
  ime[1] = false;
}

void SM83::EI() {
  ime[1] = true;
}

void SM83::RLC() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x80;
  data <<= 1;
  if(carry) data |= 0x01;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::RRC() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x01;
  data >>= 1;
  if(carry) data |= 0x80;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::RL() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x80;
  data <<= 1;
  if(CF()) data |= 0x01;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::RR() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x01;
  data >>= 1;
  if(CF()) data |= 0x80;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::SLA() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x80;
  data <<= 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::SRA() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x01;
  data = (int8_t)data >> 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::SWAP() {
  uint8_t data = regSrcRead();
  data = data << 4 | data >> 4;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(false);
  regSrcWrite(data);
}

void SM83::SRL() {
  uint8_t data = regSrcRead();
  bool carry = data & 0x01;
  data >>= 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  regSrcWrite(data);
}

void SM83::BIT() {
  setZ(!(regSrcRead() & (1 << ((ir >> 3) & 0x07))));
  setN(false);
  setH(true);
}

void SM83::RES() {
  regSrcWrite(regSrcRead() & ~(1 << ((ir >> 3) & 0x07)));
}

void SM83::SET() {
  regSrcWrite(regSrcRead() | 1 << ((ir >> 3) & 0x07));
}

void SM83::HCF() {
  for(;;) cycleIdle();
}

