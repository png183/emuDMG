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
  }

  void init(char* fnameBootROM, char* fnameCartROM) {
    reset();
    loadROM(fnameBootROM, fnameCartROM);
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
        exit(0);
        return;
      }
    }
  }

  void run() {
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
    if(keys[SDL_SCANCODE_DOWN ]) data &= ~0x08;  //DOWN
    if(keys[SDL_SCANCODE_UP   ]) data &= ~0x04;  //UP
    if(keys[SDL_SCANCODE_LEFT ]) data &= ~0x02;  //LEFT
    if(keys[SDL_SCANCODE_RIGHT]) data &= ~0x01;  //RIGHT
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
};

int main(int argc, char** argv) {
  Emulator emulator;
  emulator.init(argv[1], argv[2]);
  emulator.run();

  return 0;
}

