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
    static const int32_t GridWidth = 32;
    static const int32_t GridHeight = 32;
} GameConstants;

typedef struct SSDLState
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool quit = false;
} SDLState;

class Asset
{
public:
    int32_t getX() { return _x; };
    int32_t getY() { return _y; };
    void setX(int32_t x) { _x = x; };
    void setY(int32_t y) { _y = y; };

    int32_t getWidth() { return _width; };
    int32_t getHeight() { return _height; };

    int32_t getAlpha() { return _alpha; };
    void setAlpha(int32_t a) { _alpha = a; };
protected:
    int32_t _x = 0;
    int32_t _y = 0;
    int32_t _width = 0;
    int32_t _height = 0;
    int32_t _alpha = 0;
};

class CardAsset : public Asset
{
public:
    CardAsset(SDLState * in_sdl, const char * in_filename)
    : sdl(in_sdl),
    image(nullptr),
    texture(nullptr)
    {
        image = IMG_Load(in_filename);
        texture = SDL_CreateTextureFromSurface(sdl->renderer, image);
        _width = image->w;
        _height = image->h;
    }
    ~CardAsset()
    {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
    }
    void render()
    {
        SDL_Rect src;
        src.x = 0;
        src.y = 0;
        src.w = _width;
        src.h = _height;

        SDL_Rect dst;
        dst.x = _x;
        dst.y = _y;
        dst.w = _width;
        dst.h = _height;
        
        SDL_SetTextureAlphaMod(texture, _alpha);
        SDL_RenderCopy(sdl->renderer, texture, &src, &dst);
    }
private:
    SDLState * sdl = nullptr;
    SDL_Surface * image;
    SDL_Texture * texture;
};

class PlayerState : public Asset
{
public:
    PlayerState(SDLState * in_sdl)
    : sdl(in_sdl),
    image(nullptr),
    texture(nullptr)
    {
        image = IMG_Load("assets/hero.png");
        texture = SDL_CreateTextureFromSurface(sdl->renderer, image);
        _width = GameConstants::GridWidth;
        _height = GameConstants::GridHeight;
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
        dst.x = _x;
        dst.y = _y;
        dst.w = _width;
        dst.h = _height;
        
        SDL_RenderCopy(sdl->renderer, texture, &src, &dst);
        //SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0x00, 0x00);
        //SDL_RenderDrawRect(sdl->renderer, &dst);
    }
private:
    SDLState * sdl = nullptr;
    SDL_Surface * image;
    SDL_Texture * texture;
};

class GameScene
{
public:
    virtual ~GameScene() {};
    virtual void render() = 0;
    virtual GameScene * advance() = 0;
};

class DevScene : public GameScene
{
public:
    DevScene(SDLState * in_sdl, PlayerState * in_player)
    : sdl(in_sdl), player(in_player)
    {
        
    }
    virtual ~DevScene()
    {
        
    }
    virtual void render()
    {
        player->render();
    }
    virtual GameScene * advance()
    {
        return nullptr;
    }
private:
    SDLState * sdl;
    PlayerState * player;
};

class SplashScene : public GameScene
{
public:
    SplashScene(SDLState * in_sdl, PlayerState * in_player)
    : sdl(in_sdl), player(in_player), firstTick(0), completed(false)
    {
        logo = new CardAsset(sdl, "assets/dbgj2019splash.png");
        logo->setX((GameConstants::ScreenWidth  - logo->getWidth())/2);
        logo->setY((GameConstants::ScreenHeight - logo->getHeight())/2);
    }
    virtual ~SplashScene()
    {
        delete logo;
    }
    virtual void render()
    {
        if(firstTick==0)
        {
            firstTick = SDL_GetTicks();
            return;
        }
        uint32_t deltaTicks =  SDL_GetTicks() - firstTick;

        // (0-1000]    Ramp up
        // (1000-2000] Full up
        // (2000,3000] Ramp down
        // (3000,] completed = true
        uint32_t alpha = 0;
        if(deltaTicks<1000)
        {
            alpha = (uint32_t)((deltaTicks/1000.f)*255);
        }
        else if(deltaTicks<2000)
        {
            alpha = 255;
        }
        else if(deltaTicks<3000)
        {
            alpha = (uint32_t)(((1000-(deltaTicks-2000))/1000.f)*255);
        }
        else
        {
            alpha = 0;
            completed = true;
            return;
        }
        
        logo->setAlpha(alpha);
        logo->render();
    }
    virtual GameScene * advance()
    {
        if(completed)
        {
            return new DevScene(sdl,player);
        }
        return nullptr;
    }
private:
    SDLState * sdl;
    PlayerState * player;
    CardAsset * logo;
    uint32_t firstTick;
    bool completed;
};

class GameState
{
public:
    GameState(SDLState * in_sdl)
    : sdl(in_sdl)
    {
        player = new PlayerState(sdl);
        scene = new SplashScene(sdl,player);
    }
    ~GameState()
    {
        delete player;
        if(scene)
        {
            delete scene;
            scene = nullptr;
        }
    }

    void render()
    {
        SDL_SetRenderDrawColor( sdl->renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear(sdl->renderer);

        if(scene)
        {
            scene->render();
        }

        SDL_RenderPresent(sdl->renderer);

        GameScene * newScene = scene->advance();
        if(nullptr != newScene)
        {
            GameScene * oldScene = scene;
            scene = newScene;
            delete oldScene;
        }
    }
    PlayerState * player = nullptr;
    SDLState * sdl = nullptr;
private:
    GameScene * scene = nullptr;
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
            PlayerState * p = s_state->player;
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
            {
                if (p->getY() >= GameConstants::GridHeight)
                {
                    p->setY(p->getY()-GameConstants::GridHeight);
                }
                break;
            }
            case SDLK_DOWN:
            {
                if (p->getY()+p->getHeight()<GameConstants::ScreenHeight)
                {
                    p->setY(p->getY()+GameConstants::GridHeight);
                }
                break;
            }
            case SDLK_LEFT:
            {
                if (p->getX() >= GameConstants::GridWidth)
                {
                    p->setX(p->getX()-GameConstants::GridWidth);
                }
                break;
            }
            case SDLK_RIGHT:
            {
                if (p->getX()+p->getWidth()<GameConstants::ScreenWidth)
                {
                    p->setX(p->getX()+GameConstants::GridWidth);
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
