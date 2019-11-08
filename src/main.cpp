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

class PlayerState
{
public:
    PlayerState(SDLState * in_sdl)
    : sdl(in_sdl),
    image(nullptr),
    texture(nullptr)
    {
        image = IMG_Load("assets/hero.png");
        texture = SDL_CreateTextureFromSurface(sdl->renderer, image);
    }
    ~PlayerState()
    {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
    }
    void render()
    {
        SDL_Rect src;
        src.x = 16+((SDL_GetTicks()/250)%4)*16;
        src.y = 0;
        src.w = 16;
        src.h = 16;

        SDL_Rect dst;
        dst.x = x;
        dst.y = y;
        dst.w = width;
        dst.h = height;
        
        SDL_RenderCopy(sdl->renderer, texture, &src, &dst);
        //SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0x00, 0x00);
        //SDL_RenderDrawRect(sdl->renderer, &dst);
    }

    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 32;
    int32_t height = 32;
private:
    SDLState * sdl = nullptr;
    SDL_Surface * image;
    SDL_Texture * texture;
};

class GameState
{
public:
    GameState(SDLState * in_sdl)
    : sdl(in_sdl)
    {
        player = new PlayerState(sdl);
    }
    ~GameState()
    {
        delete player;
    }

    void render()
    {
        SDL_SetRenderDrawColor( sdl->renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear(sdl->renderer);

        player->render();

        SDL_RenderPresent(sdl->renderer);
    }
    PlayerState * player;
    SDLState * sdl;
};

GameState * s_state;

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
            s_state->sdl->quit = true;
            break;
        }
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
            {
                if (s_state->player->y >= 32)
                {
                    s_state->player->y -= 32;
                }
                break;
            }
            case SDLK_DOWN:
            {
                if (s_state->player->y+s_state->player->height<GameConstants::ScreenHeight)
                {
                    s_state->player->y += 32;
                }
                break;
            }
            case SDLK_LEFT:
            {
                if (s_state->player->x >= 32)
                {
                    s_state->player->x -= 32;
                }
                break;
            }
            case SDLK_RIGHT:
            {
                if (s_state->player->x+s_state->player->width<GameConstants::ScreenWidth)
                {
                    s_state->player->x += 32;
                }
                break;
            }
            }
            break;
        }
        }

    }

    s_state->render();
    SDL_UpdateWindowSurface(s_state->sdl->window);

#if !__EMSCRIPTEN__
    return 0;
#endif
}

void main_loop()
{

#if __EMSCRIPTEN__
    emscripten_set_main_loop(main_tick, -1, 1);
#else
    while (!s_state->sdl->quit)
    {
        main_tick();
    }
#endif
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDLState * sdl = new SDLState;

    sdl->window = SDL_CreateWindow(
        "WEBASM",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        GameConstants::ScreenWidth, GameConstants::ScreenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0xff, 0xff);

    s_state = new GameState(sdl);

    main_loop();

    delete s_state;
    s_state = nullptr;

    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    delete sdl;

    IMG_Quit();
    SDL_Quit();

    return 0;
}
