#include <cstdint>

class Cart {
public:
  ~Cart() {
    delete[] rom;
    delete[] ram;
  }

  uint8_t* getRAM() { return ram; }
  int getSizeRAM() { return ramMask + 1; }
  void load(uint8_t* cartRom, uint8_t* cartRam, uint32_t cartRamMask) { rom = cartRom; ram = cartRam; ramMask = cartRamMask; return; }
  virtual uint8_t readROM(uint16_t addr) { return rom[addr & 0x7fff]; }
  virtual void writeROM(uint16_t addr, uint8_t data) { return; }
  virtual uint8_t readRAM(uint16_t addr) { return 0xff; }
  virtual void writeRAM(uint16_t addr, uint8_t data) { return; }

protected:
  uint8_t* rom;
  uint8_t* ram;
  uint32_t ramMask;
};

class MBC1 : public Cart {
public:
  MBC1() {
    ramg = false;
    bank1 = 0x01;
    bank2 = 0x00;
    mode = false;
  }

  uint8_t readROM(uint16_t addr) override;
  void writeROM(uint16_t addr, uint8_t data) override;
  uint8_t readRAM(uint16_t addr) override;
  void writeRAM(uint16_t addr, uint8_t data) override;

private:
  bool ramg;
  uint8_t bank1;
  uint8_t bank2;
  bool mode;
};

class MBC5 : public Cart {
public:
  MBC5() {
    ramg = false;
    romb0 = 0x01;
    romb1 = 0x00;
    ramb = 0x00;
  }

  uint8_t readROM(uint16_t addr) override;
  void writeROM(uint16_t addr, uint8_t data) override;
  uint8_t readRAM(uint16_t addr) override;
  void writeRAM(uint16_t addr, uint8_t data) override;

private:
  bool ramg;
  uint8_t romb0;
  uint8_t romb1;
  uint8_t ramb;
};

