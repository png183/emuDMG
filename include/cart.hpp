#include <cstdint>

class Cart {
public:
  ~Cart() {
    delete[] rom;
  }

  void loadROM(uint8_t* cartRom) { rom = cartRom; return; }
  virtual uint8_t readROM(uint16_t addr) { return rom[addr & 0x7fff]; }
  virtual void writeROM(uint16_t addr, uint8_t data) { rom[addr & 0x7fff] = data; return; }
  virtual uint8_t readRAM(uint16_t addr) { return 0xff; }
  virtual void writeRAM(uint16_t addr, uint8_t data) { return; }

protected:
  uint8_t* rom;
};

class MBC1 : public Cart {
public:
  MBC1() {
    ram = new uint8_t[0x8000];  // todo: don't hardcode RAM to 32KiB
    ramg = false;
    bank1 = 0x01;
    bank2 = 0x00;
    mode = false;
  }

  ~MBC1() {
    delete[] ram;
  }

  uint8_t readROM(uint16_t addr) override;
  void writeROM(uint16_t addr, uint8_t data) override;
  uint8_t readRAM(uint16_t addr) override;
  void writeRAM(uint16_t addr, uint8_t data) override;

private:
  uint8_t* ram;

  bool ramg;
  uint8_t bank1;
  uint8_t bank2;
  bool mode;
};

class MBC5 : public Cart {
public:
  MBC5() {
    ram = new uint8_t[0x20000];  // todo: don't hardcode RAM to 128KiB
    ramg = false;
    romb0 = 0x01;
    romb1 = 0x00;
    ramb = 0x00;
  }

  ~MBC5() {
    delete[] ram;
  }

  uint8_t readROM(uint16_t addr) override;
  void writeROM(uint16_t addr, uint8_t data) override;
  uint8_t readRAM(uint16_t addr) override;
  void writeRAM(uint16_t addr, uint8_t data) override;

private:
  uint8_t* ram;

  bool ramg;
  uint8_t romb0;
  uint8_t romb1;
  uint8_t ramb;
};

