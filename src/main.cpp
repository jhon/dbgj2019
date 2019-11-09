#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <queue>

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
    static const int32_t Margin = 32;
} GameConstants;

typedef struct SGameContent
{
    inline static const char * IntroText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. \
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. \
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. \
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\
\n\
Curabitur pretium tincidunt lacus. Nulla gravida orci a odio. \
Nullam varius, turpis et commodo pharetra, est eros bibendum elit, nec luctus magna felis sollicitudin mauris. \
Integer in mauris eu nibh euismod gravida. Duis ac tellus et risus vulputate vehicula. \
Donec lobortis risus a elit. Etiam tempor. Ut ullamcorper, ligula eu tempor congue, eros est euismod turpis, id tincidunt sapien risus a quam. \
Maecenas fermentum consequat mi. Donec fermentum. Pellentesque malesuada nulla a mi. Duis sapien sem, aliquet nec, commodo eget, consequat quis, neque. \
Aliquam faucibus, elit ut dictum aliquet, felis nisl adipiscing sapien, sed malesuada diam lacus eget erat. Cras mollis scelerisque nunc. Nullam arcu. \
Aliquam consequat. Curabitur augue lorem, dapibus quis, laoreet et, pretium ac, nisi. Aenean magna nisl, mollis quis, molestie eu, feugiat in, orci. \
In hac habitasse platea dictumst.";
} GameContent;

typedef struct SSDLState
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *title_font = nullptr;
    TTF_Font *exposition_font = nullptr;
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
    virtual bool advance() = 0;
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
    virtual bool advance()
    {
        return false;
    }
private:
    SDLState * sdl;
    PlayerState * player;
};

class SplashScene : public GameScene
{
public:
    SplashScene(SDLState * in_sdl)
    : sdl(in_sdl), firstTick(0), completed(false)
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
    virtual bool advance()
    {
        return completed;
    }
private:
    SDLState * sdl;
    CardAsset * logo;
    uint32_t firstTick;
    bool completed;
};

enum class DBShift
{
    DawnGuard,
    AlphaFlight,
    NightWatch,
    ZetaShift,
};

class BannerScene : public GameScene
{
public:
    BannerScene(SDLState * in_sdl, DBShift in_shift, const char * in_title)
    : sdl(in_sdl), firstTick(0), completed(false)
    {
        switch(in_shift)
        {
            case DBShift::DawnGuard:
                banner = new CardAsset(sdl, "assets/banner_dawnguard.png");
                banner->setX(0);
                break;
            case DBShift::AlphaFlight:
                banner = new CardAsset(sdl, "assets/banner_alphaflight.png");
                banner->setX((12*GameConstants::ScreenWidth/30) - (banner->getWidth()/2));
                break;
            case DBShift::NightWatch:
                banner = new CardAsset(sdl, "assets/banner_nightwatch.png");
                banner->setX((18*GameConstants::ScreenWidth/30) - (banner->getWidth()/2));
                break;
            case DBShift::ZetaShift:
                banner = new CardAsset(sdl, "assets/banner_zetashift.png");
                banner->setX(GameConstants::ScreenWidth - banner->getWidth());
                break;
            default:
                // This will AV in a second :(
                break;
        }
        banner->setY((GameConstants::ScreenHeight - banner->getHeight())/2);

        text_source.x = 0;
        text_source.y = 0;
        TTF_SizeText(sdl->title_font,in_title,&text_source.w,&text_source.h);
        text_dest.x = (GameConstants::ScreenWidth - text_source.w)/2;
        text_dest.y = GameConstants::ScreenHeight - text_source.h - 32;
        text_dest.w = text_source.w;
        text_dest.h = text_source.h;
        text_surface = TTF_RenderText_Blended(sdl->title_font,in_title,SDL_Color{0xff,0xff,0xff,0xff});
        text_texture = SDL_CreateTextureFromSurface(sdl->renderer, text_surface);
    }
    virtual ~BannerScene()
    {
        delete banner;
        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);
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
        
        banner->setAlpha(alpha);
        banner->render();

        SDL_SetTextureAlphaMod(text_texture, alpha);
        SDL_RenderCopy(sdl->renderer, text_texture, &text_source, &text_dest);
    }
    virtual bool advance()
    {
        return completed;
    }
private:
    SDLState * sdl;
    CardAsset * banner;
    SDL_Surface * text_surface;
    SDL_Texture * text_texture;
    SDL_Rect text_source;
    SDL_Rect text_dest;
    uint32_t firstTick;
    bool completed;
};

class TextCardScene : public GameScene
{
public:
    TextCardScene(SDLState * in_sdl, const char * in_text)
    : sdl(in_sdl), firstTick(0), completed(false)
    {
        text_source.x = 0;
        text_source.y = 0;
        text_dest.x = GameConstants::Margin;
        text_dest.y = GameConstants::Margin;
        text_surface = TTF_RenderText_Blended_Wrapped(sdl->exposition_font,in_text,SDL_Color{0xff,0xff,0xff,0xff},GameConstants::ScreenWidth-(GameConstants::Margin*2));
        text_source.w = text_dest.w = text_surface->w;
        text_source.h = text_dest.h = text_surface->h;
        text_texture = SDL_CreateTextureFromSurface(sdl->renderer, text_surface);
    }
    virtual ~TextCardScene()
    {
        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);
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

        SDL_SetTextureAlphaMod(text_texture, alpha);
        SDL_RenderCopy(sdl->renderer, text_texture, &text_source, &text_dest);
    }
    virtual bool advance()
    {
        return completed;
    }
private:
    SDLState * sdl;
    SDL_Surface * text_surface;
    SDL_Texture * text_texture;
    SDL_Rect text_source;
    SDL_Rect text_dest;
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
        scene_queue.push(new SplashScene(sdl));
        scene_queue.push(new TextCardScene(sdl,GameContent::IntroText));
        scene_queue.push(new BannerScene(sdl,DBShift::DawnGuard,"Dawn Guard"));
        scene_queue.push(new BannerScene(sdl,DBShift::AlphaFlight,"Alpha Flight"));
        scene_queue.push(new BannerScene(sdl,DBShift::NightWatch,"Night Watch"));
        scene_queue.push(new BannerScene(sdl,DBShift::ZetaShift,"Zeta Shift"));
        scene_queue.push(new DevScene(sdl,player));
    }
    ~GameState()
    {
        delete player;
        while(!scene_queue.empty())
        {
            delete scene_queue.front();
            scene_queue.pop();
        }
    }

    void render()
    {
        SDL_SetRenderDrawColor( sdl->renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear(sdl->renderer);

        if(!scene_queue.empty())
        {
            scene_queue.front()->render();
            if(scene_queue.front()->advance())
            {
                delete scene_queue.front();
                scene_queue.pop();
            }
        }

        SDL_RenderPresent(sdl->renderer);
    }
    PlayerState * player = nullptr;
    SDLState * sdl = nullptr;
private:
    std::queue<GameScene *> scene_queue;
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
    TTF_Init();

    SDLState * sdl = new SDLState;

    sdl->window = SDL_CreateWindow(
        "WEBASM",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        GameConstants::ScreenWidth, GameConstants::ScreenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0xff, 0xff);

    sdl->title_font      = TTF_OpenFont("assets/kenney_pixel-webfont.ttf",64);
    sdl->exposition_font = TTF_OpenFont("assets/kenney_pixel-webfont.ttf",32);

    s_state = new GameState(sdl);

    main_loop();

    delete s_state;
    s_state = nullptr;

    TTF_CloseFont(sdl->title_font);
    TTF_CloseFont(sdl->exposition_font);

    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    delete sdl;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
