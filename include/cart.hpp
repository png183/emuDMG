#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class Cart {
public:
  Cart() {
    rom = new uint8_t[maxRomSize];
    ram = new uint8_t[0x8000];  //todo: don't hardcode RAM to 32KiB
  }

  ~Cart() {
    delete[] rom;
    delete[] ram;
  }

  void loadROM(char* fname);
  uint8_t readROM(uint16_t addr);
  void writeROM(uint16_t addr, uint8_t data);
  uint8_t readRAM(uint16_t addr);
  void writeRAM(uint16_t addr, uint8_t data);

private:
  const int maxRomSize = 0x200000;  // MBC1 maximum ROM size
  uint8_t* rom;
  uint8_t* ram;

  bool ramg;
  uint8_t bank1;
  uint8_t bank2;
  bool mode;
};

