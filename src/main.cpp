#include <SDL2/SDL.h>
#include "dmg.hpp"

class Emulator : public DMG {
public:
  Emulator() {
    SDL_Init(SDL_INIT_EVERYTHING);
    framebuffer = new uint32_t[width * height]();
    window = SDL_CreateWindow("emuDMG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * scale, height * scale, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    //init audio
    SDL_AudioSpec audioSpecRequested;
    SDL_AudioSpec audioSpec;
    audioSpecRequested.freq = 32768;
    audioSpecRequested.format = AUDIO_S16SYS;
    audioSpecRequested.channels = 1;
    audioSpecRequested.samples = audioBufferSize;
    audioSpecRequested.callback = NULL;  //no callback
    audioSpecRequested.userdata = NULL;  //no parameter to callback
    audioOut = SDL_OpenAudioDevice(NULL, 0, &audioSpecRequested, &audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    SDL_PauseAudioDevice(audioOut, 0);
  }

  ~Emulator() {
    SDL_CloseAudioDevice(audioOut);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    delete[] framebuffer;
    delete[] savePath;
    delete cart;
  }

  void loadCart(char* fname) {
    // load cartridge ROM
    const int maxRomSize = 0x800000;  // MBC5 maximum ROM size (8MiB)
    uint8_t* cartRom = new uint8_t[maxRomSize];
    FILE* fc = fopen(fname, "rb");
    if(!fc) {
      printf("ERROR: %s is not a valid file path\n", fname);
      exit(0);
    }
    int fsize = fread(cartRom, sizeof(uint8_t), maxRomSize, fc);
    fclose(fc);
    printf("Loaded %s\n", fname);

    // pre-mirror cartridge ROM to fill 8MiB address space
    for(int i = 0; (i + fsize) <= maxRomSize; i += fsize) memcpy(cartRom + i, cartRom, fsize);

    // initialize mapper
    uint8_t mapper = cartRom[0x0147];
    bool hasRam = false;
    switch(mapper) {
    case 0x00:                cart = new Cart(); break;
    case 0x01:                cart = new MBC1(); break;
    case 0x02: hasRam = true; cart = new MBC1(); break;
    case 0x03: hasRam = true; cart = new MBC1(); break;  // todo: has battery
    case 0x19:                cart = new MBC5(); break;
    case 0x1a: hasRam = true; cart = new MBC5(); break;
    case 0x1b: hasRam = true; cart = new MBC5(); break;  // todo: has battery
    case 0x1c:                cart = new MBC5(); break;  // todo: has rumble
    case 0x1d: hasRam = true; cart = new MBC5(); break;  // todo: has rumble
    case 0x1e: hasRam = true; cart = new MBC5(); break;  // todo: has battery and rumble
    default:
      printf("ERROR: Unsupported mapper (0x%02x)\n", mapper);
      exit(0);
      break;
    }
    printf("Mapper: 0x%02x\n", mapper);

    // load cartridge RAM
    uint8_t* cartRam = NULL;
    uint32_t cartRamMask = 0x00000;
    if(hasRam) {
      switch(cartRom[0x0149]) {
      case 0x02: cartRamMask = 0x01fff; break;
      case 0x03: cartRamMask = 0x07fff; break;
      case 0x04: cartRamMask = 0x1ffff; break;
      case 0x05: cartRamMask = 0x0ffff; break;
      default:
        printf("Warning: Cartridge header specifies RAM without quantity (0x%02x)\n");
        hasRam = false;
        break;
      }
    }
    if(hasRam) cartRam = new uint8_t[0x20000];  // maximum RAM size (128KiB)

    // load save file, if present
    // todo: only load save data if cart has battery
    savePath = new char[strlen(fname) + 5];
    sprintf(savePath, "%s.sav", fname);
    if(hasRam) {
      FILE* fs = fopen(savePath, "rb");
      if(fs) {
        fread(cartRam, sizeof(uint8_t), cartRamMask + 1, fs);
        fclose(fs);
      }
    }

    // load cartridge
    cart->load(cartRom, cartRam, cartRamMask);
  }

  void save() {
    // todo: only write save data if cart has battery
    uint8_t* saveData = cart->getRAM();
    if(saveData) {
      FILE* f = fopen(savePath, "wb");
      fwrite(saveData, sizeof(uint8_t), cart->getSizeRAM(), f);
      fclose(f);
    }
  }

  void frame() override {
    //draw frame
    SDL_UpdateTexture(texture, NULL, framebuffer, 4 * width);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    //check for window closing
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
        save();
        exit(0);
        return;
      }
    }
  }

  void run() {
    insertCart(cart);
    for(;;) instruction();
  }

  uint8_t pollButtons() override {
    //todo: support alternate key bindings
    uint8_t data = 0xff;
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_RETURN]) data &= ~0x08;  //START
    if(keys[SDL_SCANCODE_SPACE ]) data &= ~0x04;  //SELECT
    if(keys[SDL_SCANCODE_S     ]) data &= ~0x02;  //B
    if(keys[SDL_SCANCODE_D     ]) data &= ~0x01;  //A
    return data;
  }

  uint8_t pollDpad() override {
    //todo: support alternate key bindings
    uint8_t data = 0xff;
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_DOWN]) {
      data &= ~0x08;  //DOWN
    } else if(keys[SDL_SCANCODE_UP]) {
      data &= ~0x04;  //UP
    }
    if(keys[SDL_SCANCODE_LEFT]) {
      data &= ~0x02;  //LEFT
    } else if(keys[SDL_SCANCODE_RIGHT]) {
      data &= ~0x01;  //RIGHT
    }
    return data;
  }

  void plotPixel(int x, int y, uint8_t data) override {
    uint32_t colour = data ^ 0x03;  //convert palette value to colour
    colour *= 0x555555;  //scale 2-bit colour to 24-bit
    framebuffer[160 * y + x] = 0xff000000 | colour;
  }

  void emitSample(int16_t sample) override {
    static int outCnt = 0;
    outCnt++;
    if(!(outCnt & 0x1f)) {
      while(SDL_GetQueuedAudioSize(audioOut) > (audioBufferSize * 2)) {
        SDL_Delay(1);  //prevent running too far ahead of audio
      }
      SDL_QueueAudio(audioOut, &sample, sizeof(int16_t));
    }
  }

private:
  const int scale = 3;
  const int width = 160;
  const int height = 144;
  const int audioBufferSize = 1024;  //must be power of 2
  uint32_t* framebuffer;
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  SDL_AudioDeviceID audioOut;

  char* savePath;
  Cart* cart;
};

int main(int argc, char** argv) {
  if(argc != 3) {
    printf("Usage: dmg [BIOS_PATH] [CART_PATH]\n");
    exit(0);
  }

  Emulator emulator;
  emulator.loadBootROM(argv[1]);
  emulator.loadCart(argv[2]);
  emulator.run();

  return 0;
}

