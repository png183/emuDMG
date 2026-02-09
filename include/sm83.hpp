#include <cstdint>

class SM83 {
public:
  void reset();
  void instruction();
  void setIF(uint8_t data) { _if = data & 0x1f; }
  void setIE(uint8_t data) { _ie = data; }
  uint8_t IF() { return 0xe0 | _if; }
  uint8_t IE() { return _ie; }
  virtual void cycleIdle() { return; }
  virtual uint8_t cycleRead(uint16_t addr) { return 0xff; }
  virtual void cycleWrite(uint16_t addr, uint8_t data) { return; }

private:
  void instructionCB();
  uint8_t regSrcRead();
  uint8_t regDstRead();
  void regSrcWrite(uint8_t data);
  void regDstWrite(uint8_t data);
  void runISR(uint16_t addr);
  void write16(uint16_t addr, uint16_t data);
  uint8_t fetch8();
  uint16_t fetch16();
  void push8(uint8_t data);
  void push16(uint16_t data);
  uint8_t pop8();
  uint16_t pop16();
  uint16_t HL();
  void setHL(uint16_t data);
  uint16_t incHL();
  uint16_t decHL();
  uint16_t addSP();
  void setZ(bool cond);
  void setN(bool cond);
  void setH(bool cond);
  void setC(bool cond);
  bool ZF();
  bool NF();
  bool HF();
  bool CF();

  void INC();
  void DEC();
  void RLCA();
  void RRCA();
  void RLA();
  void RRA();
  void ADD16(uint16_t data);
  void STOP();
  void JR(bool cond);
  void DAA();
  void CPL();
  void SCF();
  void CCF();

  void HALT();

  void ADD(uint8_t data);
  void ADC(uint8_t data);
  void SUB(uint8_t data);
  void SBC(uint8_t data);
  void AND(uint8_t data);
  void XOR(uint8_t data);
  void OR(uint8_t data);
  void CP(uint8_t data);

  void RET(bool cond);
  void JP(bool cond);
  void CALL(bool cond);
  void RST(uint16_t addr);
  void RETI();
  void DI();
  void EI();

  void RLC();
  void RRC();
  void RL();
  void RR();
  void SLA();
  void SRA();
  void SWAP();
  void SRL();
  void BIT();
  void RES();
  void SET();

  void HCF();

  uint8_t a;
  uint8_t f;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;
  uint16_t pc;
  uint16_t sp;
  uint8_t ir;
  bool ime[2];
  uint8_t _if;
  uint8_t _ie;
};

