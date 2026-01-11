#include <cstdint>
#include <cstdio>
#include <cstdlib>

class SM83 {
public:
  void reset();
  void instruction();
  void setIF(uint8_t data) { _if = data & 0x1f; }
  void setIE(uint8_t data) { _ie = data & 0x1f; }
  uint8_t IF() { return _if; }
  uint8_t IE() { return _ie; }
  virtual void idle() { return; }
  virtual uint8_t read8(uint16_t addr) { return 0xff; }
  virtual void write8(uint16_t addr, uint8_t data) { return; }

private:
  void instructionCB();
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

  uint8_t INC(uint8_t data);
  uint8_t DEC(uint8_t data);
  void RLCA();
  void RRCA();
  void RLA();
  void RRA();
  void ADD16(uint16_t data);
  void JR(bool cond);
  void DAA();
  void CPL();
  void SCF();
  void CCF();

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

  uint8_t RLC(uint8_t data);
  uint8_t RRC(uint8_t data);
  uint8_t RL(uint8_t data);
  uint8_t RR(uint8_t data);
  uint8_t SLA(uint8_t data);
  uint8_t SRA(uint8_t data);
  uint8_t SWAP(uint8_t data);
  uint8_t SRL(uint8_t data);
  void BIT(uint8_t bit, uint8_t data);
  uint8_t RES(uint8_t bit, uint8_t data);
  uint8_t SET(uint8_t bit, uint8_t data);

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
  bool ime;
  uint8_t _if;
  uint8_t _ie;
};

