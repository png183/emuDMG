#include "sm83.hpp"

void SM83::reset() {
  pc = 0x0000;
  ime = false;
  setIF(0x00);
  setIE(0x00);
}

void SM83::instruction() {
  if(ime && (_if & _ie)) {
    ime = false;
    if(_if & _ie & 0x01) { _if &= ~0x01; runISR(0x0040); return; }
    if(_if & _ie & 0x02) { _if &= ~0x02; runISR(0x0048); return; }
    if(_if & _ie & 0x04) { _if &= ~0x04; runISR(0x0050); return; }
    if(_if & _ie & 0x08) { _if &= ~0x08; runISR(0x0058); return; }
    if(_if & _ie & 0x10) { _if &= ~0x10; runISR(0x0060); return; }
  }

  uint8_t ir = fetch8();
  switch(ir) {

  // 00-3f: misc. ops
  case 0x00:                                 return;  // NOP
  case 0x01: c = fetch8(); b = fetch8();     return;  // LD BC,nn
  case 0x02: write8(b << 8 | c, a);          return;  // LD (BC),A
  case 0x03: if(!(++c)) b++; idle();         return;  // INC BC
  case 0x04: b = INC(b);                     return;  // INC B
  case 0x05: b = DEC(b);                     return;  // DEC B
  case 0x06: b = fetch8();                   return;  // LD B,n
  case 0x07: RLCA();                         return;  // RLCA
  case 0x08: write16(fetch16(), sp);         return;  // LD (nn),SP
  case 0x09: ADD16(b << 8 | c);              return;  // ADD HL,BC
  case 0x0a: a = read8(b << 8 | c);          return;  // LD A,(BC)
  case 0x0b: idle(); if(!(c--)) b--;         return;  // DEC BC
  case 0x0c: c = INC(c);                     return;  // INC C
  case 0x0d: c = DEC(c);                     return;  // DEC C
  case 0x0e: c = fetch8();                   return;  // LD C,n
  case 0x0f: RRCA();                         return;  // RRCA
  // TODO: STOP
  case 0x11: e = fetch8(); d = fetch8();     return;  // LD DE,nn
  case 0x12: write8(d << 8 | e, a);          return;  // LD (DE),A
  case 0x13: if(!(++e)) d++; idle();         return;  // INC DE
  case 0x14: d = INC(d);                     return;  // INC D
  case 0x15: d = DEC(d);                     return;  // DEC D
  case 0x16: d = fetch8();                   return;  // LD D,n
  case 0x17: RLA();                          return;  // RLA
  case 0x18: JR(true);                       return;  // JR e
  case 0x19: ADD16(d << 8 | e);              return;  // ADD HL,DE
  case 0x1a: a = read8(d << 8 | e);          return;  // LD A,(DE)
  case 0x1b: idle(); if(!(e--)) d--;         return;  // DEC DE
  case 0x1c: e = INC(e);                     return;  // INC E
  case 0x1d: e = DEC(e);                     return;  // DEC E
  case 0x1e: e = fetch8();                   return;  // LD E,n
  case 0x1f: RRA();                          return;  // RRA
  case 0x20: JR(!ZF());                      return;  // JR NZ,e
  case 0x21: l = fetch8(); h = fetch8();     return;  // LD HL,nn
  case 0x22: write8(incHL(), a);             return;  // LD (HL+),A
  case 0x23: if(!(++l)) h++; idle();         return;  // INC HL
  case 0x24: h = INC(h);                     return;  // INC H
  case 0x25: h = DEC(h);                     return;  // DEC H
  case 0x26: h = fetch8();                   return;  // LD H,n
  case 0x27: DAA();                          return;  // DAA
  case 0x28: JR(ZF());                       return;  // JR Z,e
  case 0x29: ADD16(HL());                    return;  // ADD HL,HL
  case 0x2a: a = read8(incHL());             return;  // LD A,(HL+)
  case 0x2b: idle(); if(!(l--)) h--;         return;  // DEC HL
  case 0x2c: l = INC(l);                     return;  // INC L
  case 0x2d: l = DEC(l);                     return;  // DEC L
  case 0x2e: l = fetch8();                   return;  // LD L,n
  case 0x2f: CPL();                          return;  // CPL
  case 0x30: JR(!CF());                      return;  // JR NC,e
  case 0x31: sp = fetch16();                 return;  // LD SP,nn
  case 0x32: write8(decHL(), a);             return;  // LD (HL-),A
  case 0x33: sp++; idle();                   return;  // INC SP
  case 0x34: write8(HL(), INC(read8(HL()))); return;  // INC (HL)
  case 0x35: write8(HL(), DEC(read8(HL()))); return;  // DEC (HL)
  case 0x36: write8(HL(), fetch8());         return;  // LD (HL),n
  case 0x37: SCF();                          return;  // SCF
  case 0x38: JR(CF());                       return;  // JR C,e
  case 0x39: ADD16(sp);                      return;  // ADD HL,SP
  case 0x3a: a = read8(decHL());             return;  // LD A,(HL-)
  case 0x3b: idle(); sp--;                   return;  // DEC SP
  case 0x3c: a = INC(a);                     return;  // INC A
  case 0x3d: a = DEC(a);                     return;  // DEC A
  case 0x3e: a = fetch8();                   return;  // LD A,n
  case 0x3f: CCF();                          return;  // CCF

  // 40-7f: LD instruction
  case 0x40: b = b;           return;  // LD B,B
  case 0x41: b = c;           return;  // LD B,C
  case 0x42: b = d;           return;  // LD B,D
  case 0x43: b = e;           return;  // LD B,E
  case 0x44: b = h;           return;  // LD B,H
  case 0x45: b = l;           return;  // LD B,L
  case 0x46: b = read8(HL()); return;  // LD B,(HL)
  case 0x47: b = a;           return;  // LD B,A
  case 0x48: c = b;           return;  // LD C,B
  case 0x49: c = c;           return;  // LD C,C
  case 0x4a: c = d;           return;  // LD C,D
  case 0x4b: c = e;           return;  // LD C,E
  case 0x4c: c = h;           return;  // LD C,H
  case 0x4d: c = l;           return;  // LD C,L
  case 0x4e: c = read8(HL()); return;  // LD C,(HL)
  case 0x4f: c = a;           return;  // LD C,A
  case 0x50: d = b;           return;  // LD D,B
  case 0x51: d = c;           return;  // LD D,C
  case 0x52: d = d;           return;  // LD D,D
  case 0x53: d = e;           return;  // LD D,E
  case 0x54: d = h;           return;  // LD D,H
  case 0x55: d = l;           return;  // LD D,L
  case 0x56: d = read8(HL()); return;  // LD D,(HL)
  case 0x57: d = a;           return;  // LD D,A
  case 0x58: e = b;           return;  // LD E,B
  case 0x59: e = c;           return;  // LD E,C
  case 0x5a: e = d;           return;  // LD E,D
  case 0x5b: e = e;           return;  // LD E,E
  case 0x5c: e = h;           return;  // LD E,H
  case 0x5d: e = l;           return;  // LD E,L
  case 0x5e: e = read8(HL()); return;  // LD E,(HL)
  case 0x5f: e = a;           return;  // LD E,A
  case 0x60: h = b;           return;  // LD H,B
  case 0x61: h = c;           return;  // LD H,C
  case 0x62: h = d;           return;  // LD H,D
  case 0x63: h = e;           return;  // LD H,E
  case 0x64: h = h;           return;  // LD H,H
  case 0x65: h = l;           return;  // LD H,L
  case 0x66: h = read8(HL()); return;  // LD H,(HL)
  case 0x67: h = a;           return;  // LD H,A
  case 0x68: l = b;           return;  // LD L,B
  case 0x69: l = c;           return;  // LD L,C
  case 0x6a: l = d;           return;  // LD L,D
  case 0x6b: l = e;           return;  // LD L,E
  case 0x6c: l = h;           return;  // LD L,H
  case 0x6d: l = l;           return;  // LD L,L
  case 0x6e: l = read8(HL()); return;  // LD L,(HL)
  case 0x6f: l = a;           return;  // LD L,A
  case 0x70: write8(HL(), b); return;  // LD (HL),B
  case 0x71: write8(HL(), c); return;  // LD (HL),C
  case 0x72: write8(HL(), d); return;  // LD (HL),D
  case 0x73: write8(HL(), e); return;  // LD (HL),E
  case 0x74: write8(HL(), h); return;  // LD (HL),H
  case 0x75: write8(HL(), l); return;  // LD (HL),L
  case 0x76: HALT();          return;  // HALT
  case 0x77: write8(HL(), a); return;  // LD (HL),A
  case 0x78: a = b;           return;  // LD A,B
  case 0x79: a = c;           return;  // LD A,C
  case 0x7a: a = d;           return;  // LD A,D
  case 0x7b: a = e;           return;  // LD A,E
  case 0x7c: a = h;           return;  // LD A,H
  case 0x7d: a = l;           return;  // LD A,L
  case 0x7e: a = read8(HL()); return;  // LD A,(HL)
  case 0x7f: a = a;           return;  // LD A,A

  // 80-bf: ALU ops
  case 0x80: ADD(b);           return;  // ADD B
  case 0x81: ADD(c);           return;  // ADD C
  case 0x82: ADD(d);           return;  // ADD D
  case 0x83: ADD(e);           return;  // ADD E
  case 0x84: ADD(h);           return;  // ADD H
  case 0x85: ADD(l);           return;  // ADD L
  case 0x86: ADD(read8(HL())); return;  // ADD (HL)
  case 0x87: ADD(a);           return;  // ADD A
  case 0x88: ADC(b);           return;  // ADC B
  case 0x89: ADC(c);           return;  // ADC C
  case 0x8a: ADC(d);           return;  // ADC D
  case 0x8b: ADC(e);           return;  // ADC E
  case 0x8c: ADC(h);           return;  // ADC H
  case 0x8d: ADC(l);           return;  // ADC L
  case 0x8e: ADC(read8(HL())); return;  // ADC (HL)
  case 0x8f: ADC(a);           return;  // ADC A
  case 0x90: SUB(b);           return;  // SUB B
  case 0x91: SUB(c);           return;  // SUB C
  case 0x92: SUB(d);           return;  // SUB D
  case 0x93: SUB(e);           return;  // SUB E
  case 0x94: SUB(h);           return;  // SUB H
  case 0x95: SUB(l);           return;  // SUB L
  case 0x96: SUB(read8(HL())); return;  // SUB (HL)
  case 0x97: SUB(a);           return;  // SUB A
  case 0x98: SBC(b);           return;  // SBC B
  case 0x99: SBC(c);           return;  // SBC C
  case 0x9a: SBC(d);           return;  // SBC D
  case 0x9b: SBC(e);           return;  // SBC E
  case 0x9c: SBC(h);           return;  // SBC H
  case 0x9d: SBC(l);           return;  // SBC L
  case 0x9e: SBC(read8(HL())); return;  // SBC (HL)
  case 0x9f: SBC(a);           return;  // SBC A
  case 0xa0: AND(b);           return;  // AND B
  case 0xa1: AND(c);           return;  // AND C
  case 0xa2: AND(d);           return;  // AND D
  case 0xa3: AND(e);           return;  // AND E
  case 0xa4: AND(h);           return;  // AND H
  case 0xa5: AND(l);           return;  // AND L
  case 0xa6: AND(read8(HL())); return;  // AND (HL)
  case 0xa7: AND(a);           return;  // AND A
  case 0xa8: XOR(b);           return;  // XOR B
  case 0xa9: XOR(c);           return;  // XOR C
  case 0xaa: XOR(d);           return;  // XOR D
  case 0xab: XOR(e);           return;  // XOR E
  case 0xac: XOR(h);           return;  // XOR H
  case 0xad: XOR(l);           return;  // XOR L
  case 0xae: XOR(read8(HL())); return;  // XOR (HL)
  case 0xaf: XOR(a);           return;  // XOR A
  case 0xb0: OR(b);            return;  // OR B
  case 0xb1: OR(c);            return;  // OR C
  case 0xb2: OR(d);            return;  // OR D
  case 0xb3: OR(e);            return;  // OR E
  case 0xb4: OR(h);            return;  // OR H
  case 0xb5: OR(l);            return;  // OR L
  case 0xb6: OR(read8(HL()));  return;  // OR (HL)
  case 0xb7: OR(a);            return;  // OR A
  case 0xb8: CP(b);            return;  // CP B
  case 0xb9: CP(c);            return;  // CP C
  case 0xba: CP(d);            return;  // CP D
  case 0xbb: CP(e);            return;  // CP E
  case 0xbc: CP(h);            return;  // CP H
  case 0xbd: CP(l);            return;  // CP L
  case 0xbe: CP(read8(HL()));  return;  // CP (HL)
  case 0xbf: CP(a);            return;  // CP A

  // c0-ff: control flow
  case 0xc0: idle(); RET(!ZF());            return;  // RET NZ
  case 0xc1: c = pop8(); b = pop8();        return;  // POP BC
  case 0xc2: JP(!ZF());                     return;  // JP NZ,nn
  case 0xc3: JP(true);                      return;  // JP nn
  case 0xc4: CALL(!ZF());                   return;  // CALL NZ,nn
  case 0xc5: idle(); push8(b); push8(c);    return;  // PUSH BC
  case 0xc6: ADD(fetch8());                 return;  // ADD n
  case 0xc7: RST(0x0000);                   return;  // RST 0x00
  case 0xc8: idle(); RET(ZF());             return;  // RET Z
  case 0xc9: RET(true);                     return;  // RET
  case 0xca: JP(ZF());                      return;  // JP Z,nn
  case 0xcb: instructionCB();               return;  // CB-prefixed instruction
  case 0xcc: CALL(ZF());                    return;  // CALL Z,nn
  case 0xcd: CALL(true);                    return;  // CALL nn
  case 0xce: ADC(fetch8());                 return;  // ADC n
  case 0xcf: RST(0x0008);                   return;  // RST 0x08
  case 0xd0: idle(); RET(!CF());            return;  // RET NC
  case 0xd1: e = pop8(); d = pop8();        return;  // POP DE
  case 0xd2: JP(!CF());                     return;  // JP NC,nn
  // TODO: HCF
  case 0xd4: CALL(!CF());                   return;  // CALL NC,nn
  case 0xd5: idle(); push8(d); push8(e);    return;  // PUSH DE
  case 0xd6: SUB(fetch8());                 return;  // SUB n
  case 0xd7: RST(0x0010);                   return;  // RST 0x10
  case 0xd8: idle(); RET(CF());             return;  // RET C
  case 0xd9: RETI();                        return;  // RETI
  case 0xda: JP(CF());                      return;  // JP C,nn
  // TODO: HCF
  case 0xdc: CALL(CF());                    return;  // CALL C,nn
  // TODO: HCF
  case 0xde: SBC(fetch8());                 return;  // SBC n
  case 0xdf: RST(0x0018);                   return;  // RST 0x18
  case 0xe0: write8(0xff00 + fetch8(), a);  return;  // LDH (n),A
  case 0xe1: l = pop8(); h = pop8();        return;  // POP HL
  case 0xe2: write8(0xff00 + c, a);         return;  // LDH (C),A
  // TODO: HCF
  // TODO: HCF
  case 0xe5: idle(); push8(h); push8(l);    return;  // PUSH HL
  case 0xe6: AND(fetch8());                 return;  // AND n
  case 0xe7: RST(0x0020);                   return;  // RST 0x20
  case 0xe8: sp = addSP(); idle();          return;  // ADD SP,e
  case 0xe9: pc = HL();                     return;  // JP HL
  case 0xea: write8(fetch16(), a);          return;  // LD (nn),A
  // TODO: HCF
  // TODO: HCF
  // TODO: HCF
  case 0xee: XOR(fetch8());                 return;  // XOR n
  case 0xef: RST(0x0028);                   return;  // RST 0x28
  case 0xf0: a = read8(0xff00 + fetch8());  return;  // LDH A,(n)
  case 0xf1: f = pop8() & 0xf0; a = pop8(); return;  // POP AF
  case 0xf2: a = read8(0xff00 + c);         return;  // LDH A,(C)
  case 0xf3: DI();                          return;  // DI
  // TODO: HCF
  case 0xf5: idle(); push8(a); push8(f);    return;  // PUSH AF
  case 0xf6: OR(fetch8());                  return;  // OR n
  case 0xf7: RST(0x0030);                   return;  // RST 0x30
  case 0xf8: setHL(addSP());                return;  // LD HL,SP+e
  case 0xf9: sp = HL(); idle();             return;  // LD SP,HL
  case 0xfa: a = read8(fetch16());          return;  // LD A,(nn)
  case 0xfb: EI();                          return;  // EI
  // TODO: HCF
  // TODO: HCF
  case 0xfe: CP(fetch8());                  return;  // CP n
  case 0xff: RST(0x0038);                   return;  // RST 0x38

  }

  printf("Unimplemented instruction 0x%02x\n", ir);
  exit(0);
}

void SM83::instructionCB() {
  uint8_t ir = fetch8();
  switch(ir) {

  //00-3f: shift/rotate instructions
  case 0x00: b = RLC(b);                      return;  // RLC B
  case 0x01: c = RLC(c);                      return;  // RLC C
  case 0x02: d = RLC(d);                      return;  // RLC D
  case 0x03: e = RLC(e);                      return;  // RLC E
  case 0x04: h = RLC(h);                      return;  // RLC H
  case 0x05: l = RLC(l);                      return;  // RLC L
  case 0x06: write8(HL(), RLC(read8(HL())));  return;  // RLC (HL)
  case 0x07: a = RLC(a);                      return;  // RLC A
  case 0x08: b = RRC(b);                      return;  // RRC B
  case 0x09: c = RRC(c);                      return;  // RRC C
  case 0x0a: d = RRC(d);                      return;  // RRC D
  case 0x0b: e = RRC(e);                      return;  // RRC E
  case 0x0c: h = RRC(h);                      return;  // RRC H
  case 0x0d: l = RRC(l);                      return;  // RRC L
  case 0x0e: write8(HL(), RRC(read8(HL())));  return;  // RRC (HL)
  case 0x0f: a = RRC(a);                      return;  // RRC A
  case 0x10: b = RL(b);                       return;  // RL B
  case 0x11: c = RL(c);                       return;  // RL C
  case 0x12: d = RL(d);                       return;  // RL D
  case 0x13: e = RL(e);                       return;  // RL E
  case 0x14: h = RL(h);                       return;  // RL H
  case 0x15: l = RL(l);                       return;  // RL L
  case 0x16: write8(HL(), RL(read8(HL())));   return;  // RL (HL)
  case 0x17: a = RL(a);                       return;  // RL A
  case 0x18: b = RR(b);                       return;  // RR B
  case 0x19: c = RR(c);                       return;  // RR C
  case 0x1a: d = RR(d);                       return;  // RR D
  case 0x1b: e = RR(e);                       return;  // RR E
  case 0x1c: h = RR(h);                       return;  // RR H
  case 0x1d: l = RR(l);                       return;  // RR L
  case 0x1e: write8(HL(), RR(read8(HL())));   return;  // RL (HL)
  case 0x1f: a = RR(a);                       return;  // RR A
  case 0x20: b = SLA(b);                      return;  // SLA B
  case 0x21: c = SLA(c);                      return;  // SLA C
  case 0x22: d = SLA(d);                      return;  // SLA D
  case 0x23: e = SLA(e);                      return;  // SLA E
  case 0x24: h = SLA(h);                      return;  // SLA H
  case 0x25: l = SLA(l);                      return;  // SLA L
  case 0x26: write8(HL(), SLA(read8(HL())));  return;  // SLA (HL)
  case 0x27: a = SLA(a);                      return;  // SLA A
  case 0x28: b = SRA(b);                      return;  // SRA B
  case 0x29: c = SRA(c);                      return;  // SRA C
  case 0x2a: d = SRA(d);                      return;  // SRA D
  case 0x2b: e = SRA(e);                      return;  // SRA E
  case 0x2c: h = SRA(h);                      return;  // SRA H
  case 0x2d: l = SRA(l);                      return;  // SRA L
  case 0x2e: write8(HL(), SRA(read8(HL())));  return;  // SRA (HL)
  case 0x2f: a = SRA(a);                      return;  // SRA A
  case 0x30: b = SWAP(b);                     return;  // SWAP B
  case 0x31: c = SWAP(c);                     return;  // SWAP C
  case 0x32: d = SWAP(d);                     return;  // SWAP D
  case 0x33: e = SWAP(e);                     return;  // SWAP E
  case 0x34: h = SWAP(h);                     return;  // SWAP H
  case 0x35: l = SWAP(l);                     return;  // SWAP L
  case 0x36: write8(HL(), SWAP(read8(HL()))); return;  // SWAP (HL)
  case 0x37: a = SWAP(a);                     return;  // SWAP A
  case 0x38: b = SRL(b);                      return;  // SRL B
  case 0x39: c = SRL(c);                      return;  // SRL C
  case 0x3a: d = SRL(d);                      return;  // SRL D
  case 0x3b: e = SRL(e);                      return;  // SRL E
  case 0x3c: h = SRL(h);                      return;  // SRL H
  case 0x3d: l = SRL(l);                      return;  // SRL L
  case 0x3e: write8(HL(), SRL(read8(HL())));  return;  // SRL (HL)
  case 0x3f: a = SRL(a);                      return;  // SRL A

  //40-7f: BIT instruction
  case 0x40: BIT(0, b);           return;  // BIT 0,B
  case 0x41: BIT(0, c);           return;  // BIT 0,C
  case 0x42: BIT(0, d);           return;  // BIT 0,D
  case 0x43: BIT(0, e);           return;  // BIT 0,E
  case 0x44: BIT(0, h);           return;  // BIT 0,H
  case 0x45: BIT(0, l);           return;  // BIT 0,L
  case 0x46: BIT(0, read8(HL())); return;  // BIT 0,(HL)
  case 0x47: BIT(0, a);           return;  // BIT 0,A
  case 0x48: BIT(1, b);           return;  // BIT 1,B
  case 0x49: BIT(1, c);           return;  // BIT 1,C
  case 0x4a: BIT(1, d);           return;  // BIT 1,D
  case 0x4b: BIT(1, e);           return;  // BIT 1,E
  case 0x4c: BIT(1, h);           return;  // BIT 1,H
  case 0x4d: BIT(1, l);           return;  // BIT 1,L
  case 0x4e: BIT(1, read8(HL())); return;  // BIT 1,(HL)
  case 0x4f: BIT(1, a);           return;  // BIT 1,A
  case 0x50: BIT(2, b);           return;  // BIT 2,B
  case 0x51: BIT(2, c);           return;  // BIT 2,C
  case 0x52: BIT(2, d);           return;  // BIT 2,D
  case 0x53: BIT(2, e);           return;  // BIT 2,E
  case 0x54: BIT(2, h);           return;  // BIT 2,H
  case 0x55: BIT(2, l);           return;  // BIT 2,L
  case 0x56: BIT(2, read8(HL())); return;  // BIT 2,(HL)
  case 0x57: BIT(2, a);           return;  // BIT 2,A
  case 0x58: BIT(3, b);           return;  // BIT 3,B
  case 0x59: BIT(3, c);           return;  // BIT 3,C
  case 0x5a: BIT(3, d);           return;  // BIT 3,D
  case 0x5b: BIT(3, e);           return;  // BIT 3,E
  case 0x5c: BIT(3, h);           return;  // BIT 3,H
  case 0x5d: BIT(3, l);           return;  // BIT 3,L
  case 0x5e: BIT(3, read8(HL())); return;  // BIT 3,(HL)
  case 0x5f: BIT(3, a);           return;  // BIT 3,A
  case 0x60: BIT(4, b);           return;  // BIT 4,B
  case 0x61: BIT(4, c);           return;  // BIT 4,C
  case 0x62: BIT(4, d);           return;  // BIT 4,D
  case 0x63: BIT(4, e);           return;  // BIT 4,E
  case 0x64: BIT(4, h);           return;  // BIT 4,H
  case 0x65: BIT(4, l);           return;  // BIT 4,L
  case 0x66: BIT(4, read8(HL())); return;  // BIT 4,(HL)
  case 0x67: BIT(4, a);           return;  // BIT 4,A
  case 0x68: BIT(5, b);           return;  // BIT 5,B
  case 0x69: BIT(5, c);           return;  // BIT 5,C
  case 0x6a: BIT(5, d);           return;  // BIT 5,D
  case 0x6b: BIT(5, e);           return;  // BIT 5,E
  case 0x6c: BIT(5, h);           return;  // BIT 5,H
  case 0x6d: BIT(5, l);           return;  // BIT 5,L
  case 0x6e: BIT(5, read8(HL())); return;  // BIT 5,(HL)
  case 0x6f: BIT(5, a);           return;  // BIT 5,A
  case 0x70: BIT(6, b);           return;  // BIT 6,B
  case 0x71: BIT(6, c);           return;  // BIT 6,C
  case 0x72: BIT(6, d);           return;  // BIT 6,D
  case 0x73: BIT(6, e);           return;  // BIT 6,E
  case 0x74: BIT(6, h);           return;  // BIT 6,H
  case 0x75: BIT(6, l);           return;  // BIT 6,L
  case 0x76: BIT(6, read8(HL())); return;  // BIT 6,(HL)
  case 0x77: BIT(6, a);           return;  // BIT 6,A
  case 0x78: BIT(7, b);           return;  // BIT 7,B
  case 0x79: BIT(7, c);           return;  // BIT 7,C
  case 0x7a: BIT(7, d);           return;  // BIT 7,D
  case 0x7b: BIT(7, e);           return;  // BIT 7,E
  case 0x7c: BIT(7, h);           return;  // BIT 7,H
  case 0x7d: BIT(7, l);           return;  // BIT 7,L
  case 0x7e: BIT(7, read8(HL())); return;  // BIT 7,(HL)
  case 0x7f: BIT(7, a);           return;  // BIT 7,A

  //80-bf: RES instruction
  case 0x80: b = RES(0, b);                     return;  // RES 0,B
  case 0x81: c = RES(0, c);                     return;  // RES 0,C
  case 0x82: d = RES(0, d);                     return;  // RES 0,D
  case 0x83: e = RES(0, e);                     return;  // RES 0,E
  case 0x84: h = RES(0, h);                     return;  // RES 0,H
  case 0x85: l = RES(0, l);                     return;  // RES 0,L
  case 0x86: write8(HL(), RES(0, read8(HL()))); return;  // RES 0,(HL)
  case 0x87: a = RES(0, a);                     return;  // RES 0,A
  case 0x88: b = RES(1, b);                     return;  // RES 1,B
  case 0x89: c = RES(1, c);                     return;  // RES 1,C
  case 0x8a: d = RES(1, d);                     return;  // RES 1,D
  case 0x8b: e = RES(1, e);                     return;  // RES 1,E
  case 0x8c: h = RES(1, h);                     return;  // RES 1,H
  case 0x8d: l = RES(1, l);                     return;  // RES 1,L
  case 0x8e: write8(HL(), RES(1, read8(HL()))); return;  // RES 1,(HL)
  case 0x8f: a = RES(1, a);                     return;  // RES 1,A
  case 0x90: b = RES(2, b);                     return;  // RES 2,B
  case 0x91: c = RES(2, c);                     return;  // RES 2,C
  case 0x92: d = RES(2, d);                     return;  // RES 2,D
  case 0x93: e = RES(2, e);                     return;  // RES 2,E
  case 0x94: h = RES(2, h);                     return;  // RES 2,H
  case 0x95: l = RES(2, l);                     return;  // RES 2,L
  case 0x96: write8(HL(), RES(2, read8(HL()))); return;  // RES 2,(HL)
  case 0x97: a = RES(2, a);                     return;  // RES 2,A
  case 0x98: b = RES(3, b);                     return;  // RES 3,B
  case 0x99: c = RES(3, c);                     return;  // RES 3,C
  case 0x9a: d = RES(3, d);                     return;  // RES 3,D
  case 0x9b: e = RES(3, e);                     return;  // RES 3,E
  case 0x9c: h = RES(3, h);                     return;  // RES 3,H
  case 0x9d: l = RES(3, l);                     return;  // RES 3,L
  case 0x9e: write8(HL(), RES(3, read8(HL()))); return;  // RES 3,(HL)
  case 0x9f: a = RES(3, a);                     return;  // RES 3,A
  case 0xa0: b = RES(4, b);                     return;  // RES 4,B
  case 0xa1: c = RES(4, c);                     return;  // RES 4,C
  case 0xa2: d = RES(4, d);                     return;  // RES 4,D
  case 0xa3: e = RES(4, e);                     return;  // RES 4,E
  case 0xa4: h = RES(4, h);                     return;  // RES 4,H
  case 0xa5: l = RES(4, l);                     return;  // RES 4,L
  case 0xa6: write8(HL(), RES(4, read8(HL()))); return;  // RES 4,(HL)
  case 0xa7: a = RES(4, a);                     return;  // RES 4,A
  case 0xa8: b = RES(5, b);                     return;  // RES 5,B
  case 0xa9: c = RES(5, c);                     return;  // RES 5,C
  case 0xaa: d = RES(5, d);                     return;  // RES 5,D
  case 0xab: e = RES(5, e);                     return;  // RES 5,E
  case 0xac: h = RES(5, h);                     return;  // RES 5,H
  case 0xad: l = RES(5, l);                     return;  // RES 5,L
  case 0xae: write8(HL(), RES(5, read8(HL()))); return;  // RES 5,(HL)
  case 0xaf: a = RES(5, a);                     return;  // RES 5,A
  case 0xb0: b = RES(6, b);                     return;  // RES 6,B
  case 0xb1: c = RES(6, c);                     return;  // RES 6,C
  case 0xb2: d = RES(6, d);                     return;  // RES 6,D
  case 0xb3: e = RES(6, e);                     return;  // RES 6,E
  case 0xb4: h = RES(6, h);                     return;  // RES 6,H
  case 0xb5: l = RES(6, l);                     return;  // RES 6,L
  case 0xb6: write8(HL(), RES(6, read8(HL()))); return;  // RES 6,(HL)
  case 0xb7: a = RES(6, a);                     return;  // RES 6,A
  case 0xb8: b = RES(7, b);                     return;  // RES 7,B
  case 0xb9: c = RES(7, c);                     return;  // RES 7,C
  case 0xba: d = RES(7, d);                     return;  // RES 7,D
  case 0xbb: e = RES(7, e);                     return;  // RES 7,E
  case 0xbc: h = RES(7, h);                     return;  // RES 7,H
  case 0xbd: l = RES(7, l);                     return;  // RES 7,L
  case 0xbe: write8(HL(), RES(7, read8(HL()))); return;  // RES 7,(HL)
  case 0xbf: a = RES(7, a);                     return;  // RES 7,A

  //c0-ff: SET instruction
  case 0xc0: b = SET(0, b);                     return;  // SET 0,B
  case 0xc1: c = SET(0, c);                     return;  // SET 0,C
  case 0xc2: d = SET(0, d);                     return;  // SET 0,D
  case 0xc3: e = SET(0, e);                     return;  // SET 0,E
  case 0xc4: h = SET(0, h);                     return;  // SET 0,H
  case 0xc5: l = SET(0, l);                     return;  // SET 0,L
  case 0xc6: write8(HL(), SET(0, read8(HL()))); return;  // SET 0,(HL)
  case 0xc7: a = SET(0, a);                     return;  // SET 0,A
  case 0xc8: b = SET(1, b);                     return;  // SET 1,B
  case 0xc9: c = SET(1, c);                     return;  // SET 1,C
  case 0xca: d = SET(1, d);                     return;  // SET 1,D
  case 0xcb: e = SET(1, e);                     return;  // SET 1,E
  case 0xcc: h = SET(1, h);                     return;  // SET 1,H
  case 0xcd: l = SET(1, l);                     return;  // SET 1,L
  case 0xce: write8(HL(), SET(1, read8(HL()))); return;  // SET 1,(HL)
  case 0xcf: a = SET(1, a);                     return;  // SET 1,A
  case 0xd0: b = SET(2, b);                     return;  // SET 2,B
  case 0xd1: c = SET(2, c);                     return;  // SET 2,C
  case 0xd2: d = SET(2, d);                     return;  // SET 2,D
  case 0xd3: e = SET(2, e);                     return;  // SET 2,E
  case 0xd4: h = SET(2, h);                     return;  // SET 2,H
  case 0xd5: l = SET(2, l);                     return;  // SET 2,L
  case 0xd6: write8(HL(), SET(2, read8(HL()))); return;  // SET 2,(HL)
  case 0xd7: a = SET(2, a);                     return;  // SET 2,A
  case 0xd8: b = SET(3, b);                     return;  // SET 3,B
  case 0xd9: c = SET(3, c);                     return;  // SET 3,C
  case 0xda: d = SET(3, d);                     return;  // SET 3,D
  case 0xdb: e = SET(3, e);                     return;  // SET 3,E
  case 0xdc: h = SET(3, h);                     return;  // SET 3,H
  case 0xdd: l = SET(3, l);                     return;  // SET 3,L
  case 0xde: write8(HL(), SET(3, read8(HL()))); return;  // SET 3,(HL)
  case 0xdf: a = SET(3, a);                     return;  // SET 3,A
  case 0xe0: b = SET(4, b);                     return;  // SET 4,B
  case 0xe1: c = SET(4, c);                     return;  // SET 4,C
  case 0xe2: d = SET(4, d);                     return;  // SET 4,D
  case 0xe3: e = SET(4, e);                     return;  // SET 4,E
  case 0xe4: h = SET(4, h);                     return;  // SET 4,H
  case 0xe5: l = SET(4, l);                     return;  // SET 4,L
  case 0xe6: write8(HL(), SET(4, read8(HL()))); return;  // SET 4,(HL)
  case 0xe7: a = SET(4, a);                     return;  // SET 4,A
  case 0xe8: b = SET(5, b);                     return;  // SET 5,B
  case 0xe9: c = SET(5, c);                     return;  // SET 5,C
  case 0xea: d = SET(5, d);                     return;  // SET 5,D
  case 0xeb: e = SET(5, e);                     return;  // SET 5,E
  case 0xec: h = SET(5, h);                     return;  // SET 5,H
  case 0xed: l = SET(5, l);                     return;  // SET 5,L
  case 0xee: write8(HL(), SET(5, read8(HL()))); return;  // SET 5,(HL)
  case 0xef: a = SET(5, a);                     return;  // SET 5,A
  case 0xf0: b = SET(6, b);                     return;  // SET 6,B
  case 0xf1: c = SET(6, c);                     return;  // SET 6,C
  case 0xf2: d = SET(6, d);                     return;  // SET 6,D
  case 0xf3: e = SET(6, e);                     return;  // SET 6,E
  case 0xf4: h = SET(6, h);                     return;  // SET 6,H
  case 0xf5: l = SET(6, l);                     return;  // SET 6,L
  case 0xf6: write8(HL(), SET(6, read8(HL()))); return;  // SET 6,(HL)
  case 0xf7: a = SET(6, a);                     return;  // SET 6,A
  case 0xf8: b = SET(7, b);                     return;  // SET 7,B
  case 0xf9: c = SET(7, c);                     return;  // SET 7,C
  case 0xfa: d = SET(7, d);                     return;  // SET 7,D
  case 0xfb: e = SET(7, e);                     return;  // SET 7,E
  case 0xfc: h = SET(7, h);                     return;  // SET 7,H
  case 0xfd: l = SET(7, l);                     return;  // SET 7,L
  case 0xfe: write8(HL(), SET(7, read8(HL()))); return;  // SET 7,(HL)
  case 0xff: a = SET(7, a);                     return;  // SET 7,A

  }

  //unreachable
}

void SM83::runISR(uint16_t addr) {
  idle();
  idle();
  push16(pc);
  idle();
  pc = addr;
}

void SM83::write16(uint16_t addr, uint16_t data) {
  write8(addr, data);
  write8(addr + 1, data >> 8);
}

uint8_t SM83::fetch8() {
  return read8(pc++);
}

uint16_t SM83::fetch16() {
  uint16_t data = read8(pc++);
  return read8(pc++) << 8 | data;
}

void SM83::push8(uint8_t data) {
  write8(--sp, data);
}

void SM83::push16(uint16_t data) {
  push8(data >> 8);
  push8(data);
}

uint8_t SM83::pop8() {
  return read8(sp++);
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
  idle();
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

uint8_t SM83::INC(uint8_t data) {
  uint8_t dataPrev = data++;
  setZ(data == 0);
  setN(false);
  setH((dataPrev & 0x10) != (data & 0x10));
  return data;
}

uint8_t SM83::DEC(uint8_t data) {
  uint8_t dataPrev = data--;
  setZ(data == 0);
  setN(true);
  setH((dataPrev & 0x10) != (data & 0x10));
  return data;
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
  idle();
}

void SM83::JR(bool cond) {
  int16_t displacement = (int8_t)fetch8();
  if(cond) {
    pc += displacement;
    idle();
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
  while(!(_if & _ie)) idle();
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
    idle();
  }
}

void SM83::JP(bool cond) {
  uint16_t target = fetch16();
  if(cond) {
    pc = target;
    idle();
  }
}

void SM83::CALL(bool cond) {
  uint16_t target = fetch16();
  if(cond) {
    idle();
    push16(pc);
    pc = target;
  }
}

void SM83::RST(uint16_t addr) {
  idle();
  push16(pc);
  pc = addr;
}

void SM83::RETI() {
  ime = true;
  pc = pop16();
  idle();
}

void SM83::DI() {
  ime = false;
}

void SM83::EI() {
  ime = true;
}

uint8_t SM83::RLC(uint8_t data) {
  bool carry = data & 0x80;
  data <<= 1;
  if(carry) data |= 0x01;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::RRC(uint8_t data) {
  bool carry = data & 0x01;
  data >>= 1;
  if(carry) data |= 0x80;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::RL(uint8_t data) {
  bool carry = data & 0x80;
  data <<= 1;
  if(CF()) data |= 0x01;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::RR(uint8_t data) {
  bool carry = data & 0x01;
  data >>= 1;
  if(CF()) data |= 0x80;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::SLA(uint8_t data) {
  bool carry = data & 0x80;
  data <<= 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::SRA(uint8_t data) {
  bool carry = data & 0x01;
  data = (int8_t)data >> 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

uint8_t SM83::SWAP(uint8_t data) {
  data = data << 4 | data >> 4;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(false);
  return data;
}

uint8_t SM83::SRL(uint8_t data) {
  bool carry = data & 0x01;
  data >>= 1;
  setZ(data == 0);
  setN(false);
  setH(false);
  setC(carry);
  return data;
}

void SM83::BIT(uint8_t bit, uint8_t data) {
  data &= 1 << bit;
  setZ(data == 0);
  setN(false);
  setH(true);
}

uint8_t SM83::RES(uint8_t bit, uint8_t data) {
  return data & ~(1 << bit);
}

uint8_t SM83::SET(uint8_t bit, uint8_t data) {
  return data | 1 << bit;
}

