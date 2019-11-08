#include <cstdio>
#include <cstdlib>
#include <cinttypes>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef struct SGameConstants
{
    static const int32_t ScreenWidth = 1280;
    static const int32_t ScreenHeight = 720;
} GameConstants;

typedef struct SSDLState
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool quit = false;
} SDLState;
static SDLState s_sdl;

typedef struct SPlayerState
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 20;
    int32_t height = 20;

    void render()
    {
        SDL_Rect r_scr;
        r_scr.x = x;
        r_scr.y = y;
        r_scr.w = width;
        r_scr.h = height;
        
        SDL_SetRenderDrawColor(s_sdl.renderer, 0xff, 0xff, 0x00, 0x00);
        SDL_RenderDrawRect(s_sdl.renderer, &r_scr);
    }
} PlayerState;

typedef struct SGameState
{
    PlayerState player;

    void render()
    {
        SDL_SetRenderDrawColor( s_sdl.renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear(s_sdl.renderer);

        player.render();

        SDL_RenderPresent(s_sdl.renderer);
    }
} GameState;

GameState s_state;

#if __EMSCRIPTEN__
void main_tick() {
#else
int main_tick() {
#endif

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            s_sdl.quit = true;
            break;
        }
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
            {
                if (s_state.player.y>=20)
                {
                    s_state.player.y-=20;
                }
                break;
            }
            case SDLK_DOWN:
            {
                if (s_state.player.y+s_state.player.height<GameConstants::ScreenHeight)
                {
                    s_state.player.y += 20;
                }
                break;
            }
            case SDLK_LEFT:
            {
                if (s_state.player.x>=20)
                {
                    s_state.player.x-=20;
                }
                break;
            }
            case SDLK_RIGHT:
            {
                if (s_state.player.x+s_state.player.width<GameConstants::ScreenWidth)
                {
                    s_state.player.x += 20;
                }
                break;
            }
            }
            break;
        }
        }

    }

    s_state.render();
    SDL_UpdateWindowSurface(s_sdl.window);

#if !__EMSCRIPTEN__
    return 0;
#endif
}

void main_loop()
{

#if __EMSCRIPTEN__
    emscripten_set_main_loop(main_tick, -1, 1);
#else
    while (!s_sdl.quit)
    {
        main_tick();
    }
#endif
}

int main(int argc, char *argv[])
{
    s_sdl = SDLState();
    s_state = GameState();

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    s_sdl.window = SDL_CreateWindow(
        "WEBASM",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        GameConstants::ScreenWidth, GameConstants::ScreenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    s_sdl.renderer = SDL_CreateRenderer(s_sdl.window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(s_sdl.renderer, 0xff, 0xff, 0xff, 0xff);

    main_loop();

    SDL_DestroyRenderer(s_sdl.renderer);
    SDL_DestroyWindow(s_sdl.window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
