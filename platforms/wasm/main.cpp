#include <iostream>
#include <SDL2/SDL.h>
#include <chrono>
#include <unistd.h>
#define __EMSCRIPTEN__ 1
#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../../src/gearboy.h"
#include "Wasm_Sound_Queue.h"

static GearboyCore gearboyCore;
static Wasm_Sound_Queue soundQueue;
static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* buffer;

static int width = 160;
static int height = 140;
static const int GBC_WIDTH = 160;
static const int GBC_HEIGHT = 144;
static GB_Color frameBuffer[GBC_WIDTH * GBC_HEIGHT];
static SDL_Rect renderWindowRect;
static int16_t sampleBuffer[AUDIO_BUFFER_SIZE];
static bool running = true;
static bool romLoaded = false;
static std::vector<uint8_t> cartridge;

struct palette_color
{
    int red;
    int green;
    int blue;
    int alpha;
};

static palette_color dmg_palette[4];

void setup_colors()
{

    dmg_palette[0].red = 0xEF;
    dmg_palette[0].green = 0xF3;
    dmg_palette[0].blue = 0xD5;
    dmg_palette[0].alpha = 0xFF;

    dmg_palette[1].red = 0xA3;
    dmg_palette[1].green = 0xB6;
    dmg_palette[1].blue = 0x7A;
    dmg_palette[1].alpha = 0xFF;

    dmg_palette[2].red = 0x37;
    dmg_palette[2].green = 0x61;
    dmg_palette[2].blue = 0x3B;
    dmg_palette[2].alpha = 0xFF;

    dmg_palette[3].red = 0x04;
    dmg_palette[3].green = 0x1C;
    dmg_palette[3].blue = 0x16;
    dmg_palette[3].alpha = 0xFF;

    GB_Color color1;
    GB_Color color2;
    GB_Color color3;
    GB_Color color4;

    color1.red = dmg_palette[0].red;
    color1.green = dmg_palette[0].green;
    color1.blue = dmg_palette[0].blue;
    color2.red = dmg_palette[1].red;
    color2.green = dmg_palette[1].green;
    color2.blue = dmg_palette[1].blue;
    color3.red = dmg_palette[2].red;
    color3.green = dmg_palette[2].green;
    color3.blue = dmg_palette[2].blue;
    color4.red = dmg_palette[3].red;
    color4.green = dmg_palette[3].green;
    color4.blue = dmg_palette[3].blue;

    gearboyCore.SetDMGPalette(color1, color2, color3, color4);
}

void init()
{
    gearboyCore.Init();
    soundQueue.start(44100, 2);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to init: " << SDL_GetError() << std::endl;
    }

    if(SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer) != 0)
    {
        std::cerr << "Failed ot make window and renderer!" << std::endl;
    }

    renderWindowRect.x = 0;
    renderWindowRect.y = 0;
    
    //TODO: Adjust these values given the size of the canvas.
    renderWindowRect.h = height;
    renderWindowRect.w = width;

    //NOTE: This pixel format needs to match whatever GB_Color in definitions compiles to!
    buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if(buffer == nullptr)
    {
        std::cerr << "Failed to create render texture!" << SDL_GetError() << std::endl;
    }

    for(int i = 0; i < width * height; ++i)
    {
        frameBuffer[i].alpha = UINT8_MAX;
        frameBuffer[i].red = 0;
        frameBuffer[i].green = 0;
        frameBuffer[i].blue = 0;
    }
}

void render()
{
    void* pixels = nullptr;
    int pitch = -1;
    if(SDL_LockTexture(buffer, NULL, &pixels, &pitch) != 0)
    {
        std::cerr << "SDL_LockTexture failed with error: " << SDL_GetError() << std::endl;
    }

    memcpy(pixels, frameBuffer, width * height * sizeof(GB_Color));
    SDL_UnlockTexture(buffer);
    if(SDL_RenderClear(renderer) != 0)
    {
        std::cerr << "SDL_RenderClear failed with error: " << SDL_GetError() << std::endl;
    }

    if(SDL_RenderCopy(renderer, buffer, NULL, &renderWindowRect) != 0)
    {
        std::cerr << "SDL_RenderCopy failed with error: " << SDL_GetError() << std::endl;
    }

    SDL_RenderPresent(renderer);
}

void translate_keyboard_event(const SDL_Event& key_event, const bool keydown)
{
    if(keydown)
    {
        switch(key_event.key.keysym.sym)
        {
            case SDLK_LEFT:
                gearboyCore.KeyPressed(Left_Key);
                break;
            case SDLK_RIGHT:
                gearboyCore.KeyPressed(Right_Key);                
                break;
            case SDLK_UP:
                gearboyCore.KeyPressed(Up_Key);
                break;
            case SDLK_DOWN:
                gearboyCore.KeyPressed(Down_Key);
                break;
            case SDLK_a:
                gearboyCore.KeyPressed(A_Key);
                break;
            case SDLK_s:
                gearboyCore.KeyPressed(B_Key);
                break;
            case SDLK_RETURN:
                gearboyCore.KeyPressed(Start_Key);
                break;
            case SDLK_SPACE:
                gearboyCore.KeyPressed(Select_Key);
                break;
            default:
                break;
        }
    }
    else
    {
        switch(key_event.key.keysym.sym)
        {
            case SDLK_LEFT:
                gearboyCore.KeyReleased(Left_Key);
                break;
            case SDLK_RIGHT:
                gearboyCore.KeyReleased(Right_Key);                
                break;
            case SDLK_UP:
                gearboyCore.KeyReleased(Up_Key);
                break;
            case SDLK_DOWN:
                gearboyCore.KeyReleased(Down_Key);
                break;
            case SDLK_a:
                gearboyCore.KeyReleased(A_Key);
                break;
            case SDLK_s:
                gearboyCore.KeyReleased(B_Key);
                break;
            case SDLK_RETURN:
                gearboyCore.KeyReleased(Start_Key);
                break;
            case SDLK_SPACE:
                gearboyCore.KeyReleased(Select_Key);
                break;
            default:
                break;
        }
    }
}

void update()
{
    SDL_Event key_event;
    while(SDL_PollEvent(&key_event))
    {
        switch(key_event.type)
        {
            case SDL_QUIT:
                running = false;
            case SDL_KEYDOWN:
                translate_keyboard_event(key_event, true);
                break;
            case SDL_KEYUP:
                translate_keyboard_event(key_event, false);    
                break;
            default:
                break;
        }
    }

    int sampleCount = 0;
    gearboyCore.RunToVBlank(frameBuffer, sampleBuffer, &sampleCount);
    if(sampleCount > 0) {
        soundQueue.write(sampleBuffer, sampleCount);
    }
}

void shutdown()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#ifdef __EMSCRIPTEN__
extern "C" {

void EMSCRIPTEN_KEEPALIVE save_state() 
{
    //Saves the state of the emulator to a constant location.
    gearboyCore.SaveState(0);
}

void EMSCRIPTEN_KEEPALIVE load_state()
{
    gearboyCore.LoadState(0);
}

void EMSCRIPTEN_KEEPALIVE load_rom()
{
    const std::string path = "current_rom.gbc";
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    for(unsigned int i = 0; i < contents.size(); ++i)
    {
        cartridge.push_back(contents[i]);
    }

    if(gearboyCore.LoadROMFromBuffer(cartridge.data(), contents.size(), false))
    {
        setup_colors();
        gearboyCore.LoadRam();
        //Toggle the main loop.
        romLoaded = true;
    }
}

EM_JS(int, canvas_get_width, (), {
    return canvas.width;
});

EM_JS(int, canvas_get_height, (), {
    return canvas.height;
});

void EMSCRIPTEN_KEEPALIVE resize_window()
{
    renderWindowRect.w = canvas_get_width();
    renderWindowRect.h = canvas_get_height();
}

void EMSCRIPTEN_KEEPALIVE mainloop()
{
    if(romLoaded)
    {
        update();
        render();
    }
}

int EMSCRIPTEN_KEEPALIVE main(int argc, char* argv[])
{
    init();
    resize_window();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 60, 1);
#else

    if(argc > 1)
    {
        load_rom(argv[1]);
    }
    while(running)
    {
        mainloop();
    }
#endif
    shutdown();
    return 0;
}

};
#else
void load_rom(const char* path)
{
    std::cout << "PATH: " << path << std::endl;
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    for(unsigned int i = 0; i < contents.size(); ++i)
    {
        cartridge.push_back(contents[i]);
    }

    std::cout <<  "CART: " << std::to_string(cartridge.size());

    if(gearboyCore.LoadROMFromBuffer(cartridge.data(), contents.size(), false))
    {
        std::cout << "Load rom completed!" << std::endl;
        setup_colors();
        gearboyCore.LoadRam();
        //Toggle the main loop.
        romLoaded = true;
    }
}

int main(int argc, char* argv[])
{
    init();
    if(argc > 1)
    {
        load_rom(argv[1]);
    }

    while(running)
    {
        if(romLoaded)
        {
            auto now = std::chrono::system_clock::now();
            update();
            render();
            auto end = std::chrono::system_clock::now();
            auto sleepMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - now);
            //Sleep for microseconds remaining.
            auto sleepDuration = 16666.66667 - sleepMicroseconds.count();
            std::cout << "Sleep duration: " << std::to_string(sleepDuration) << std::endl;
        }
    }

    std::cout << "Shutdown!" << std::endl;
    shutdown();

    return 0;
}

#endif