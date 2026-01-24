#include <cstdint>

class Cart {
public:
  Cart() {
    ram = new uint8_t[0x8000];  // todo: don't hardcode RAM to 32KiB
  }

  ~Cart() {
    delete[] rom;
    delete[] ram;
  }

  void loadROM(uint8_t* cartRom);
  uint8_t readROM(uint16_t addr);
  void writeROM(uint16_t addr, uint8_t data);
  uint8_t readRAM(uint16_t addr);
  void writeRAM(uint16_t addr, uint8_t data);

private:
  uint8_t* rom;
  uint8_t* ram;

  bool ramg;
  uint8_t bank1;
  uint8_t bank2;
  bool mode;
};

