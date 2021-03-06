/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tracteurblinde@gmail.com> wrote this file.  As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 *                                                 Jhon Adams (Tracteur Blinde)
 * ----------------------------------------------------------------------------
 */
#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cinttypes>
#include <cstdarg>
#include <queue>
#include <random>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

constexpr float lerp(int32_t a, int32_t b, int32_t n, int32_t d)
{
    float r = (float)n/d;
    if(r>1.f) r = 1.f;
    return (a*(1-r))+(b*r);
}

std::mt19937_64 RandomEngine;

typedef struct SLevelSettings
{
    int32_t numSpirits = 20;
    int32_t numScorpions = 20;
    int32_t numSkeletons = 0;
    bool isNight = false;
} LevelSettings;

typedef struct SGameConstants
{
    static const int32_t ScreenWidth = 1280;
    static const int32_t ScreenHeight = 720;
    static const int32_t GridWidth = 32;
    static const int32_t GridHeight = 32;
    static const int32_t Margin = 32;
    static const int32_t MapWidth = 128;
    static const int32_t MapHeight = 128;

    static const int32_t MapStartX = 0;
    static const int32_t MapStartY = 50;

    // Derived Constants
    static const int32_t NumTilesWide = (ScreenWidth/GridWidth)+2;
    static const int32_t NumTilesHigh = (ScreenHeight/GridHeight)+2;
    static const int32_t NumTilesWide2 = NumTilesWide/2;
    static const int32_t NumTilesHigh2 = NumTilesHigh/2;
    static const int32_t TopLeftX = (ScreenWidth/2)-(GridWidth/2)-(GridWidth*NumTilesWide2);
    static const int32_t TopLeftY = (ScreenHeight/2)-(GridHeight/2)-(GridWidth*NumTilesHigh2);
} GameConstants;

typedef struct SGameContent
{
    inline static const char * Title = "The Desert Styx";
    inline static const char * Subtitle = "press any key";
    inline static const char * ContinueExposition = "press any key";
    inline static const char * GameOverText = "Game Over";
    inline static const char * ScoreText = "Spirits Collected: %i";

    inline static const LevelSettings LevelOneSettings   = {.numSpirits = 20, .numScorpions = 20, .numSkeletons= 0,  .isNight = false };
    inline static const LevelSettings LevelTwoSettings   = {.numSpirits = 20, .numScorpions = 20, .numSkeletons= 10, .isNight = true  };
    inline static const LevelSettings LevelThreeSettings = {.numSpirits = 20, .numScorpions = 20, .numSkeletons= 20, .isNight = true  };
    inline static const LevelSettings LevelFourSettings  = {.numSpirits = 20, .numScorpions = 20, .numSkeletons= 40, .isNight = false };

    inline static const char * AlphaFlightExposition = "Alpha Flight\n\
\n\
For generations, the Ferryman guided the dead across the River Styx, from this world to the next.\n\
\n\
As the world grew hotter, the river dried up. The Ferryman traded his transport for something more fitting of the desert: A Bus.\n\
\n\
Gather the Souls from the area surrounding The Meadows ((Las Vegas)) and bring them to your bus that you may cross the desert styx to The Base of the Black Hills ((Tucson)).\n\
\n\
Good Luck, Busdriver.";
    inline static const char * NightWatchExposition = "Night Watch\n\
\n\
The sun set.\n\
\n\
As night fell, those not ready to give up their old lives grew in power.\n\
\n\
Good Luck, Busdriver.";
    inline static const char * ZetaShiftExposition = "Zeta Shift\n\
\n\
The dead of night.\n\
\n\
The creatures that go bump in the night grow even stronger.\n\
\n\
Good Luck, Busdriver.";
    inline static const char * DawnGuardExposition = "Dawn Guard\n\
\n\
The sky begins to glow as the sun threatens to dawn.\n\
\n\
The creatures make a last attempt to stop you as you collect spirits.\n\
\n\
Good Luck, Busdriver.";
    inline static const char * Controls = "Arrow keys control the character.\n\
\n\
Enemies move the turn after you.\n\
\n\
If you move into an enemy's square, the enemy dies.\n\
If an enemy moves into your square, you die.\n\
\n\
Max score is 150.\
\n\
\n\
CAVEATS:\n\
- There are no sounds\n\
\n\
CREDITS:\n\
Code: Jhon Adams (Tracteur Blinde)\n\
Pixel Art: Henry Software (henrysoftware.itch.io)";
    inline static const char * LoremIpsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. \
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. \
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. \
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\
\n\n\
Curabitur pretium tincidunt lacus. Nulla gravida orci a odio. \
Nullam varius, turpis et commodo pharetra, est eros bibendum elit, nec luctus magna felis sollicitudin mauris. \
Integer in mauris eu nibh euismod gravida. Duis ac tellus et risus vulputate vehicula. \
Donec lobortis risus a elit. Etiam tempor. Ut ullamcorper, ligula eu tempor congue, eros est euismod turpis, id tincidunt sapien risus a quam. \
Maecenas fermentum consequat mi. Donec fermentum. Pellentesque malesuada nulla a mi. Duis sapien sem, aliquet nec, commodo eget, consequat quis, neque. \
Aliquam faucibus, elit ut dictum aliquet, felis nisl adipiscing sapien, sed malesuada diam lacus eget erat. Cras mollis scelerisque nunc. Nullam arcu. \
Aliquam consequat. Curabitur augue lorem, dapibus quis, laoreet et, pretium ac, nisi. Aenean magna nisl, mollis quis, molestie eu, feugiat in, orci. \
In hac habitasse platea dictumst.";
} GameContent;

enum class DBShift
{
    DawnGuard,
    AlphaFlight,
    NightWatch,
    ZetaShift,
    NONE
};

enum class GameStage : int32_t
{
    Splash,
    Title,
    Controls,
    AlphaFlightExposition,
    LevelOne,
    NightWatchExposition,
    LevelTwo,
    ZetaShiftExposition,
    LevelThree,
    DawnGuardExposition,
    LevelFour,
    GameOver, // Returned from a stage on a failure
    Reset,   
    Advance, // Returned from a stage on a success
};

enum class GamePhase
{
    Input,
    Player,
    Enemy,
};

enum class MobType
{
    None,
    Spirit,
    Scorpion,
    Skeleton,
};

enum class ParticleType
{
    None,
    Slash,
    Consume,
    Warp,
};

typedef struct SSDLState
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *title_font = nullptr;
    TTF_Font *exposition_font = nullptr;
    bool quit = false;
} SDLState;

typedef struct GameState
{
    GameState(SDLState * in_sdl) : sdl(in_sdl) {}
    SDLState * sdl;
    GameStage stage = GameStage::Splash;
    GamePhase phase = GamePhase::Player;
    uint32_t phaseChange = 0;
} GameState;

GameState * s_state;

class Asset
{
public:
    int32_t getX() { return _x; };
    int32_t getY() { return _y; };
    void setX(int32_t x) { _x = x; };
    void setY(int32_t y) { _y = y; };

    int32_t getGridX() { return _grid_x; };
    int32_t getGridY() { return _grid_y; };
    void setGridX(int32_t x) { _target_grid_x = _grid_x = x; };
    void setGridY(int32_t y) { _target_grid_y = _grid_y = y; };

    float getGridLerpX() { return gridlerp(_grid_x,_target_grid_x); }
    float getGridLerpY() { return gridlerp(_grid_y, _target_grid_y); }
    float getGridOffsetX()
    {
        return gridlerp(0, _target_grid_x - _grid_x);
    }
    float getGridOffsetY()
    {
        return gridlerp(0, _target_grid_y - _grid_y);
    }

    int32_t getTargetGridX() { return _target_grid_x; };
    int32_t getTargetGridY() { return _target_grid_y; };
    void setTargetGridX(int32_t x) { _target_grid_x = x; };
    void setTargetGridY(int32_t y) { _target_grid_y = y; };

    int32_t getWidth() { return _width; };
    int32_t getHeight() { return _height; };

    int32_t getAlpha() { return _alpha; };
    void setAlpha(int32_t a) { _alpha = a; };
protected:
    float gridlerp(int32_t a, int32_t b)
    {
        if(a==b) return (float)a;
        return lerp(a,b,SDL_GetTicks()-s_state->phaseChange,200);
    }

    int32_t _x = 0;
    int32_t _y = 0;
    int32_t _width = 0;
    int32_t _height = 0;
    int32_t _alpha = 0xff;
    int32_t _grid_x = 0;
    int32_t _grid_y = 0;
    int32_t _target_grid_x = 0;
    int32_t _target_grid_y = 0;
};

class CardAsset : public Asset
{
public:
    CardAsset(SDLState * in_sdl, const char * in_filename)
    : sdl(in_sdl)
    {
        image = IMG_Load(in_filename);
        stageTwo();
    }
    CardAsset(SDLState * in_sdl, TTF_Font * in_font, const char * in_format, ...)
    : sdl(in_sdl)
    {
        char buffer[4096];
        size_t buffer_size = sizeof(buffer)/sizeof(buffer[0]);

        va_list va_args;
        va_start(va_args, in_format);
        int len = vsprintf(buffer, in_format, va_args);
        va_end(va_args);

        // Ensure the text terminates
        if (len + 1 >= 4096)
        {
            len = 4095;
        }
        buffer[len + 1] = '\0';
        
        image = TTF_RenderText_Blended(in_font,buffer,SDL_Color{0xff,0xff,0xff,0xff});
        stageTwo();
    }
    void stageTwo()
    {
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
        if(_x > GameConstants::ScreenWidth || _x < 0 - _width ||
           _y > GameConstants::ScreenHeight || _y < 0 - _height)
        {
            return;
        }

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

class AnimatedAsset : public Asset
{
public:
    AnimatedAsset(SDLState * in_sdl, const char * in_filename, uint32_t in_timePerFrame, uint32_t in_numFrames, uint32_t in_frameOffset=0, bool in_repeat=true)
    : sdl(in_sdl), timePerFrame(in_timePerFrame), numFrames(in_numFrames), frameOffset(in_frameOffset), repeat(false)
    {
        init(in_filename, in_timePerFrame, in_numFrames, in_frameOffset, in_repeat);
    }
    AnimatedAsset(SDLState * in_sdl)
    : sdl(in_sdl)
    {

    }

    void init(const char * in_filename, uint32_t in_timePerFrame, uint32_t in_numFrames, uint32_t in_frameOffset=0, bool in_repeat=true)
    {
        timePerFrame = in_timePerFrame;
        numFrames = in_numFrames;
        frameOffset = in_frameOffset;
        repeat = in_repeat;
        image = IMG_Load(in_filename);
        texture = SDL_CreateTextureFromSurface(sdl->renderer, image);
        _width = GameConstants::GridWidth;
        _height = (image->h/16) * GameConstants::GridHeight;
        if(!repeat)
        {
            startTime = SDL_GetTicks();
        }
    }
    ~AnimatedAsset()
    {
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
    }

    void render()
    {
        if(_x > GameConstants::ScreenWidth || _x < 0 - GameConstants::GridWidth ||
           _y > GameConstants::ScreenHeight || _y < 0 - GameConstants::GridHeight ||
           completed)
        {
            return;
        }
        SDL_Rect src;
        if(repeat)
        {
            src.x = (((SDL_GetTicks()/timePerFrame)%numFrames)+frameOffset)*16;
        }
        else
        {
            uint32_t frame = (SDL_GetTicks()-startTime)/timePerFrame;
            if(frame > numFrames)
            {
                completed = true;
                return;
            }
            src.x = (frame+frameOffset)*16;
        }
        
        src.y = 0;
        src.w = 16;
        src.h = image->h;

        SDL_Rect dst;
        dst.x = _x;
        dst.y = _y;
        dst.w = _width;
        dst.h = _height;
        
        SDL_RenderCopy(sdl->renderer, texture, &src, &dst);
        //SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0x00, 0x00);
        //SDL_RenderDrawRect(sdl->renderer, &dst);
    }
    bool isComplete() { return completed; }
private:
    SDLState * sdl = nullptr;
    SDL_Surface * image = nullptr;
    SDL_Texture * texture = nullptr;
    uint32_t timePerFrame = 200;
    uint32_t numFrames = 1;
    uint32_t frameOffset = 0;
    uint32_t startTime = 0;
    bool repeat = false;
    bool completed = false;
};

class PlayerState : public AnimatedAsset
{
public:
    PlayerState(SDLState * in_sdl)
    : AnimatedAsset(in_sdl, "assets/hero.png", 250, 4, 1)
    {
    }
    void incScore() { score++; }
    int32_t getScore() { return score; }
private:
    int32_t score = 0;
};

class MobAsset : public AnimatedAsset
{
public:
    MobAsset(SDLState * in_sdl, MobType in_type)
    : AnimatedAsset(in_sdl), mobtype(in_type)
    {
        switch(mobtype)
        {
            case MobType::Spirit:
                init("assets/spirit.png", 200, 4);
                break;
            case MobType::Scorpion:
                init("assets/scorpion.png", 200, 5);
                break;
            case MobType::Skeleton:
                init("assets/skeleton.png", 200, 6);
                break;
            case MobType::None:
                break;
        }
    }
    MobType getMobType() { return mobtype; }
private:
    MobType mobtype;
};

class ParticleAsset : public AnimatedAsset
{
public:
    ParticleAsset(SDLState * in_sdl, ParticleType in_type)
    : AnimatedAsset(in_sdl), particletype(in_type)
    {
        switch(particletype)
        {
            case ParticleType::Slash:
                init("assets/slash.png", 50, 7, 0, false);
                break;
            case ParticleType::Consume:
                init("assets/consume.png", 50, 13, 0, false);
                break;
            case ParticleType::Warp:
                init("assets/warp.png", 50, 9, 0, false);
                break;
            case ParticleType::None:
                break;
        }
    }
    ParticleType getParticleType() { return particletype; }
private:
    ParticleType particletype;
};

class MapAsset : Asset
{
public:
    enum Tile
    {
        DesertBegin     = 0x00,
        Desert0         = 0x00,
		Desert1         = 0x01,
		Desert2         = 0x02,
		Desert3         = 0x03,
		Desert4         = 0x04,
		Desert5         = 0x05,
		Desert6         = 0x06,
        DesertEnd       = 0x06,
		DesertSkull     = 0x07,
        DesertDoodadBegin=0x08,
		DesertDoodad0   = 0x08,
		DesertDoodad1   = 0x09,
		DesertDoodad2   = 0x0a,
		DesertDoodad3   = 0x0b,
		DesertDoodad4   = 0x0c,
		DesertDoodad5   = 0x0d,
		DesertDoodad6   = 0x0e,
		DesertDoodad7   = 0x0f,
		DesertDoodad8   = 0x10,
        DesertDoodadEnd = 0x10,
        DesertGrassBegin= 0x11,
		DesertGrassBack = 0x11,
		DesertGrassFore = 0x12,
        DesertGrassEnd  = 0x12,
		DesertStairsDown= 0x13,
		DesertStairsUp  = 0x14,
		GrassBack       = 0x15,
		GrassFore       = 0x16,
		Hedge0          = 0x17,
		Hedge1          = 0x18,
		Hedge2          = 0x19,
		Hedge3          = 0x1a,
		Ruins0          = 0x1b,
		Ruins1          = 0x1c,
		Ruins2          = 0x1d,
		Ruins3          = 0x1e,
		Ruins4          = 0x1f,
		Ruins5          = 0x20,
        RoadDesertBegin = 0x21,
		RoadDesert0     = 0x21,
		RoadDesert1     = 0x22,
		RoadDesert2     = 0x23,
		RoadDesert3     = 0x24,
		RoadDesert4     = 0x25,
        RoadDesertEnd   = 0x25,
    };
    MapAsset(SDLState * in_sdl, PlayerState * in_player, bool in_isNight)
    : sdl(in_sdl), player(in_player), isNight(in_isNight)
    {
        std::uniform_real_distribution<float> u(0,1.f);
        std::uniform_int_distribution<int32_t> terraingen(Tile::DesertBegin,Tile::DesertEnd);
        std::uniform_int_distribution<int32_t> doodadgen(Tile::DesertDoodadBegin,Tile::DesertDoodadEnd);
        std::uniform_int_distribution<int32_t> grassgen(Tile::DesertGrassBegin,Tile::DesertGrassEnd);
        tiles = new uint16_t[GameConstants::MapHeight*GameConstants::MapWidth];
        for(int32_t i=0;i<(GameConstants::MapHeight*GameConstants::MapWidth);++i)
        {
            float doodadroll = u(RandomEngine);
            uint16_t doodad = 0;
            if(doodadroll < 0.1) // 10% chance of doodad
            {
                doodad = doodadgen(RandomEngine);
            }
            else if(doodadroll < 0.6) // 50% of grass
            {
                doodad = grassgen(RandomEngine);
            }
            // Bottom half is terrain, top half is doodads
            tiles[i] = (doodad<<8) | (0xff & terraingen(RandomEngine));
        }
        
        image = IMG_Load("assets/terrain.png");
        texture = SDL_CreateTextureFromSurface(sdl->renderer, image);

        createPath();
    }
    ~MapAsset()
    {
        delete tiles;
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
    }
    void createPath()
    {
        std::uniform_real_distribution<float> u(0.f,1.f);
        std::uniform_int_distribution<int32_t> uroad(Tile::RoadDesertBegin,Tile::RoadDesertEnd);

        int32_t y = GameConstants::MapStartY;
        float ybias = (u(RandomEngine)*1.f)-0.5f;
        for(int32_t x=GameConstants::MapStartX;x<GameConstants::MapWidth;++x)
        {
            
                   //  Random Component + Center Bias
            ybias += ((u(RandomEngine)-0.5f)/2.f) + (((y-50)*-1.f)/500.f);
            if(ybias>0.33)
            {
                ybias = MIN(ybias,1.f);
                y = MIN(y+1,GameConstants::MapHeight);
                tiles[(y*GameConstants::MapWidth)+x] = (uint16_t)uroad(RandomEngine);
            }
            else if (ybias<-0.33)
            {
                ybias = MAX(ybias,-5.f);
                y = MAX(y-1,0);
                tiles[(y*GameConstants::MapWidth)+x] = (uint16_t)uroad(RandomEngine);
            }
            else
            {
                tiles[(y*GameConstants::MapWidth)+x] = (uint16_t)uroad(RandomEngine);    
            }
            road[x] = y;
        }

        // Put a stair in the last slot
        int32_t x = GameConstants::MapWidth - 1;
        y = road[x];
        tiles[(y*GameConstants::MapWidth)+x] = (uint16_t)Tile::DesertStairsDown;
    }
    void render()
    {
        for(int32_t y=0;y<GameConstants::NumTilesHigh+1;++y)
        {
            int32_t tiley = y+player->getGridY()-GameConstants::NumTilesHigh2;
            if(tiley<0)
            {
                continue;
            }
            if((int32_t)tiley>=GameConstants::MapHeight)
            {
                break;
            }
            for(int32_t x=0;x<GameConstants::NumTilesWide+1;++x)
            {
                int32_t tilex = (int32_t)(x+player->getGridX()-GameConstants::NumTilesWide2);
                if(tilex<0)
                {
                    continue;
                }
                if((int32_t)tilex>=GameConstants::MapWidth)
                {
                    break;
                }

                int16_t tileValue = at(tilex,tiley);

                SDL_Rect src;
                src.x = 16*(tileValue&0xff);
                src.y = (isNight)?16:0;
                src.w = 16;
                src.h = 16;
        
                SDL_Rect dst;
                dst.x = (int32_t)(GameConstants::TopLeftX+((x+(player->getGridOffsetX()*-1.f))*GameConstants::GridWidth));
                dst.y = (int32_t)(GameConstants::TopLeftY+((y+(player->getGridOffsetY()*-1.f))*GameConstants::GridHeight));
                dst.w = GameConstants::GridWidth;
                dst.h = GameConstants::GridHeight;
                SDL_RenderCopy(sdl->renderer, texture, &src, &dst);

                // Render Doodad
                if((tileValue&0xff00) != 0)
                {
                    tileValue >>= 8;
                    src.x = 16*(tileValue&0xff);
                    SDL_RenderCopy(sdl->renderer, texture, &src, &dst);
                }
            }
        }
    }
    int16_t at(int32_t x, int32_t y)
    {
        return tiles[(y*GameConstants::MapWidth)+x];
    }
    int32_t getRoadY(int32_t x)
    {
        if(x < 0 || x > GameConstants::MapWidth)
        {
            return -1;
        }
        return road[x];
    }
    void toScreenCoords(float grid_x, float grid_y, int32_t & pixel_x, int32_t & pixel_y)
    {
        grid_x -= player->getGridLerpX()-GameConstants::NumTilesWide2;
        grid_y -= player->getGridLerpY()-GameConstants::NumTilesHigh2;
        pixel_x = (int32_t)(GameConstants::TopLeftX+(grid_x*GameConstants::GridWidth));
        pixel_y = (int32_t)(GameConstants::TopLeftY+(grid_y*GameConstants::GridHeight));
    }
    bool canMove(int32_t x, int32_t y)
    {
        if(x < 0 || y < 0 || x >= GameConstants::MapWidth || y >= GameConstants::MapHeight)
        {
            return false;
        }
        int16_t tileValue = at(x,y);
        int8_t terrain = (int8_t)(tileValue & 0xff);
        int8_t doodad  = (int8_t)(tileValue >> 8);

        bool validTile = false;

        if((terrain >= Tile::DesertBegin && terrain <= Tile::DesertEnd) ||
           (terrain >= Tile::RoadDesertBegin && terrain <= Tile::RoadDesertEnd) ||
           terrain == Tile::DesertStairsDown)
        {
            validTile = true;
        }

        switch(doodad)
        {
            case Tile::DesertDoodad0: // Cactus
		    case Tile::DesertDoodad1: // Cactus
		    case Tile::DesertDoodad2: // Rock
		    case Tile::DesertDoodad3: // Rock
                validTile = false;
                break;
		    case Tile::DesertDoodad4: // Flower
		    case Tile::DesertDoodad5: // Flower
                break;
		    case Tile::DesertDoodad6: // Cactus
		    case Tile::DesertDoodad7: // Cactus
		    case Tile::DesertDoodad8: // Cactus
                validTile = false;
                break;
            default:
                break;
        }

        return validTile;
    }
private:
    SDLState * sdl = nullptr;
    PlayerState * player = nullptr;
    SDL_Surface * image = nullptr;
    SDL_Texture * texture = nullptr;
    uint16_t * tiles;
    int32_t road[GameConstants::MapWidth];
    bool isNight = false;
};

class GameScene
{
public:
    virtual ~GameScene() {};
    virtual void render() = 0;
    virtual GameStage advance(GameStage currentStage) = 0;
    virtual void keydown(SDL_Keycode keycode) = 0;
};

class DesertLevel : public GameScene
{
public:
    DesertLevel(SDLState * in_sdl, PlayerState * in_player, LevelSettings in_settings)
    : sdl(in_sdl), player(in_player), settings(in_settings)
    {
        map = new MapAsset(sdl,player,settings.isNight);
        createMobs();
        createScoreText();
    }
    virtual ~DesertLevel()
    {
        delete map;
        map = nullptr;
        deleteMobs();
        delete score;
        score = nullptr;
    }
    void createMobs()
    {
        std::uniform_int_distribution<int32_t> ux(0,GameConstants::MapWidth-1);
        std::uniform_int_distribution<int32_t> uy(0,GameConstants::MapHeight-1);
        // We're going to create TotalSpirits spirits randomly,
        // but we want them to them to be within 20 of the path
        // 1) Generate a random X
        // 2) Get That Road's Y
        // 3) random y between max(0,y-20) min(mapheight,y+20)
        int32_t x, y;
        for(int32_t i = 0; i < settings.numSpirits; ++i)
        {
            x = ux(RandomEngine);
            y = map->getRoadY(x);
            std::uniform_int_distribution<int32_t> getNewY(MAX(0,y-20),MIN(GameConstants::MapHeight,y+20));
            int32_t newY = -1;
            while(!map->canMove(x,newY))
            {
                newY = getNewY(RandomEngine);
            }
            mobs.push_back(new MobAsset(sdl,MobType::Spirit));
            mobs.back()->setGridX(x);
            mobs.back()->setGridY(newY);
        }

        for(int32_t i = 0; i < settings.numScorpions; ++i)
        {
            x = -1;
            y = -1;
            while(!map->canMove(x,y) && !isMobAtLocation(x,y))
            {
                x = ux(RandomEngine);
                y = uy(RandomEngine);
            }
            mobs.push_back(new MobAsset(sdl,MobType::Scorpion));
            mobs.back()->setGridX(x);
            mobs.back()->setGridY(y);
        }

        for(int32_t i = 0; i < settings.numSkeletons; ++i)
        {
            x = -1;
            y = -1;
            while(!map->canMove(x,y) && !isMobAtLocation(x,y))
            {
                x = ux(RandomEngine);
                y = uy(RandomEngine);
            }
            mobs.push_back(new MobAsset(sdl,MobType::Skeleton));
            mobs.back()->setGridX(x);
            mobs.back()->setGridY(y);
        }
    }
    void deleteMobs()
    {
        for(auto m : mobs)
        {
            delete m;
        }
        mobs.clear();
    }
    void renderMobs()
    {
        int32_t x = 0;
        int32_t y = 0;
        for(auto m : mobs)
        {    
            map->toScreenCoords(m->getGridLerpX(),m->getGridLerpY(),x,y);
            m->setX(x); m->setY(y);
            m->render();
        }
    }
    std::vector<MobAsset*>::iterator __findMobAtLocation(int32_t x, int32_t y)
    {
        std::vector<MobAsset*>::iterator it;
        for(it = mobs.begin(); it != mobs.end(); ++it)
        {
            if((*it)->getGridX()==x && (*it)->getGridY()==y)
            {
                return it;
            }
        }
        return it;
    }
    bool isMobAtLocation(int32_t x, int32_t y)
    {
        return __findMobAtLocation(x,y)!=mobs.end();
    }
    MobType detectMobCollision(int32_t x, int32_t y)
    {
        MobType result = MobType::None;

        std::vector<MobAsset*>::iterator it = __findMobAtLocation(x,y);
        if(it!=mobs.end())
        {
            result = (*it)->getMobType();
            mobs.erase(it);

            // Skeletons result in a spirit spawning
            if(result==MobType::Skeleton)
            {
                mobs.push_back(new MobAsset(sdl,MobType::Spirit));
                mobs.back()->setGridX(x);
                mobs.back()->setGridY(y);
            }
        }
        return result;
    }
    bool isMobAt(int32_t x, int32_t y)
    {
        for(auto m : mobs)
        {
            if(m->getTargetGridX()==x && m->getTargetGridY()==y)
            {
                return true;
            }
        }
        return false;
    }
    bool moveMobs()
    {
        // Create a grid representing the screen
        //  We're then going to calculate dijkstra across it
        //  (The default values we should be less than a thousand grid elements)
        int8_t djGrid[GameConstants::NumTilesHigh*GameConstants::NumTilesWide];
        for(int32_t i=0;i<GameConstants::NumTilesHigh*GameConstants::NumTilesWide;++i)
        {
            djGrid[i] = -1;
        }
        djGrid[(GameConstants::NumTilesHigh2*GameConstants::NumTilesWide) + GameConstants::NumTilesWide2] = 0;
        std::queue<int16_t> unvisited;
        unvisited.push((GameConstants::NumTilesWide2 << 8) | GameConstants::NumTilesHigh2);
        while(!unvisited.empty())
        {
            int32_t i = unvisited.front();
            unvisited.pop();

            int32_t x = (i >> 8) & 0xff;
            int32_t y = i & 0xff;

            if(x < 0 || x > GameConstants::NumTilesWide ||
               y < 0 || y > GameConstants::NumTilesHigh)
            {
                break;
            }

            // This lambda looks at the tile.
            //  If it's unwalkable, just trash it
            //  if it's walkable, set its value to min(cv,ourvalue+1);
            // If it has a value that's 's been visited and is > ourvalue+1, add it to the unvisited list
            auto searchTile = [&djGrid,&unvisited,this](int32_t screenx, int32_t screeny, int32_t playerx, int32_t playery, int32_t newValue)
            {
                // This x/y is in screen space, translate to world space
                int32_t worldx = screenx+playerx-GameConstants::NumTilesWide2;
                int32_t worldy = screeny+playery-GameConstants::NumTilesHigh2;
                
                // If this is outside the map, feel sad :(
                if(worldx < 0 || worldx > GameConstants::MapWidth ||
                worldy < 0 || worldy > GameConstants::MapHeight)
                {
                    return;
                }

                // See if the tile is walkable
                if(!map->canMove(worldx, worldy))
                {
                    return;
                }
            
                uint32_t i = (GameConstants::NumTilesWide*screeny)+screenx;
                if(i>=GameConstants::NumTilesHigh*GameConstants::NumTilesWide)
                {
                    return;
                }
                int32_t cv = djGrid[i];
                if(cv==-1)
                {
                    cv = newValue;
                    unvisited.push((int16_t)((screenx << 8) | screeny));
                }
                djGrid[(GameConstants::NumTilesWide*screeny)+screenx] = MIN(cv,newValue);
            };

            // Look up, down, left, right
            int32_t value = djGrid[(GameConstants::NumTilesWide*y)+x]+1;
            int32_t playerx = player->getGridX();
            int32_t playery = player->getGridY();
            searchTile(x,y-1,playerx,playery,value);
            searchTile(x,y+1,playerx,playery,value);
            searchTile(x+1,y,playerx,playery,value);
            searchTile(x-1,y,playerx,playery,value);
        }

        // This code is useful for displaying the dijkstra grid
#if 0
        for(int32_t y=0;y<GameConstants::NumTilesHigh;++y)
        {
            for(int32_t x=0;x<GameConstants::NumTilesWide;++x)
            {
                int32_t centerx = GameConstants::TopLeftX + (x*GameConstants::GridWidth) + (GameConstants::GridWidth/2);
                int32_t centery = GameConstants::TopLeftY + (y*GameConstants::GridHeight) + (GameConstants::GridHeight/2);

                uint8_t value = djGrid[(y*GameConstants::NumTilesWide)+x];

                CardAsset * c = new CardAsset(sdl, sdl->exposition_font, "%x", value);
                c->setX(centerx-(c->getWidth()/2));
                c->setY(centery-(c->getHeight()/2));
                c->render();
                delete c;
                //SDL_SetRenderDrawColor(sdl->renderer, red, 0x00, 0x00, 0x00);
                //SDL_RenderDrawRect(sdl->renderer, &dst);
            }
        }
#endif
        bool result = false;
        // Ignore a real pathfinding for now. Try to move one closer to the protagonist if you're on screen
        for(auto m : mobs)
        {
            // Off screen or a spirit
            int32_t x = m->getX();
            int32_t y = m->getY();
            if(m->getMobType()==MobType::Spirit ||
               x<0 || x>GameConstants::ScreenWidth ||
               y<0 || y>GameConstants::ScreenHeight)
            {
                continue;
            }

            int32_t worldx = m->getGridX();
            int32_t worldy = m->getGridY();

            // Determine the screen coordinates
            int32_t screenx = worldx - (player->getGridX()-GameConstants::NumTilesWide2);
            int32_t screeny = worldy - (player->getGridY()-GameConstants::NumTilesHigh2);

            auto getValue = [&djGrid](int32_t x, int32_t y)
            {
                if((y*GameConstants::NumTilesWide)+x >= GameConstants::NumTilesWide*GameConstants::NumTilesHigh)
                {
                    return (int8_t)255;
                }
                return djGrid[(y*GameConstants::NumTilesWide)+x];
            };

            uint8_t n = getValue(screenx,screeny-1);
            uint8_t s = getValue(screenx,screeny+1);
            uint8_t e = getValue(screenx-1,screeny);
            uint8_t w = getValue(screenx+1,screeny);
            
            int32_t dx = 0, dy = 0;
            if(n <= s && n <= e && n <= w && map->canMove(worldx,worldy-1) && !isMobAt(worldx,worldy-1))
            {
                dy = -1;
            }
            else if(s <= n && s <= e && s <= w && map->canMove(worldx,worldy+1) && !isMobAt(worldx,worldy+1))
            {
                dy = 1;
            }
            else if (e <= n && e <= s && e <= w && map->canMove(worldx-1,worldy) && !isMobAt(worldx-1,worldy))
            {
                dx = -1;
            }
            else if(w <= n && w <= s && w <= e && map->canMove(worldx+1,worldy) && !isMobAt(worldx+1,worldy))
            {
                dx = 1;
            }

            if(dx != 0 || dy != 0)
            {
                m->setTargetGridX(worldx+dx);
                m->setTargetGridY(worldy+dy);
                result = true;
            }
        }
        return result;
    }
    void finalizeMobMovement()
    {
        for(auto m : mobs)
        {
            int32_t x = m->getTargetGridX();
            int32_t y = m->getTargetGridY();
            m->setGridX(x);
            m->setGridY(y);

            // If the player is here... they lose
            if(player->getGridX()==x && player->getGridY()==y)
            {
                gameOver = true;
            }
        }
    }
    void createScoreText()
    {
        if(score)
        {
            delete score;
            score = nullptr;
        }
        score = new CardAsset(sdl,sdl->exposition_font,GameContent::ScoreText,player->getScore());
        score->setX(GameConstants::Margin);
        score->setY(GameConstants::Margin);
    }
    void renderParticles()
    {
        int32_t x = 0;
        int32_t y = 0;
        std::vector<ParticleAsset*>::iterator to_delete = particles.end();
        for(std::vector<ParticleAsset*>::iterator it = particles.begin(); it != particles.end(); ++it)
        {
            if((*it)->isComplete())
            {
                to_delete = it;
            }

            map->toScreenCoords((*it)->getGridLerpX(),(*it)->getGridLerpY(),x,y);
            (*it)->setX(x); (*it)->setY(y);
            (*it)->render();
        }

        if(to_delete != particles.end())
        {
            particles.erase(to_delete);
        }
    }
    virtual void render()
    {
        if(firstRender)
        {
            player->setGridX(GameConstants::MapStartX);
            player->setGridY(GameConstants::MapStartY);
            player->setX((GameConstants::ScreenWidth/2)-(GameConstants::GridWidth/2));
            player->setY((GameConstants::ScreenHeight/2)-(GameConstants::GridHeight/2));
            firstRender = false;
        }

        if(SDL_GetTicks()-s_state->phaseChange>200)
        {
            uint16_t tile;
            switch(s_state->phase)
            {
                case GamePhase::Player:
                    // Make sure the player mob's grid coordinates are updated
                    player->setGridX(player->getTargetGridX());
                    player->setGridY(player->getTargetGridY());

                    tile = map->at((int32_t)player->getGridX(),(int32_t)player->getGridY());
                    if (tile == MapAsset::Tile::DesertStairsDown)
                    {
                        levelComplete = true;
                    }

                    s_state->phase = GamePhase::Enemy;
                    s_state->phaseChange = SDL_GetTicks();

                    if(!moveMobs())
                    {
                        // If no mobs moved, skip the enemy phase
                        s_state->phase = GamePhase::Input;
                    }
                    break;
                case GamePhase::Enemy:
                    finalizeMobMovement();
                    s_state->phase = GamePhase::Input;
                    s_state->phaseChange = SDL_GetTicks();
                    break;
                case GamePhase::Input:
                    break;
            }
        }

        map->render();
        player->render();

        renderMobs();
        renderParticles();

        score->render();
    }
    virtual GameStage advance(GameStage currentStage)
    {
        if(levelComplete)
        {
            return GameStage::Advance;
        }
        if(gameOver)
        {
            return GameStage::GameOver;
        }
        return currentStage;
    }
    virtual void keydown(SDL_Keycode keycode)
    {
        if(s_state->phase != GamePhase::Input)
        {
            return;
        }
        int32_t playerx = (int32_t) player->getGridX();
        int32_t playery = (int32_t) player->getGridY();
        switch (keycode)
        {
        case SDLK_UP:
        {
            if(map->canMove(playerx,playery-1))
            {
                playery--;
            }
            break;
        }
        case SDLK_DOWN:
        {
            if(map->canMove(playerx,playery+1))
            {
                playery++;
            }
            break;
        }
        case SDLK_LEFT:
        {
            if(map->canMove(playerx-1,playery))
            {
                playerx--;
            }
            break;
        }
        case SDLK_RIGHT:
        {
            if(map->canMove(playerx+1,playery))
            {
                playerx++;
            }
            break;
        }
#ifndef __EMSCRIPTEN__
        case SDLK_F5:
        {
            delete map;
            map = new MapAsset(sdl,player,settings.isNight);
            deleteMobs();
            createMobs();
            break;
        }
#endif
        }
        MobType collision = detectMobCollision(playerx,playery);
        if(collision == MobType::Spirit)
        {
            player->incScore();
            createScoreText();

            // Create a sword consume animation at the target location
            particles.push_back(new ParticleAsset(sdl, ParticleType::Consume));
            particles.back()->setGridX(playerx);
            particles.back()->setGridY(playery-2);
        }
        else if(collision != MobType::None)
        {
            // Create a sword swipe at the target location
            particles.push_back(new ParticleAsset(sdl, ParticleType::Slash));
            particles.back()->setGridX(playerx);
            particles.back()->setGridY(playery);

            playerx = (int32_t) player->getGridX();
            playery = (int32_t) player->getGridY();
        }

        player->setTargetGridX(playerx);
        player->setTargetGridY(playery);

        s_state->phase = GamePhase::Player;
        s_state->phaseChange = SDL_GetTicks();

        int16_t tile = map->at(playerx,playery);
        if (tile == MapAsset::Tile::DesertStairsDown)
        {
            particles.push_back(new ParticleAsset(sdl, ParticleType::Warp));
            particles.back()->setGridX(playerx);
            particles.back()->setGridY(playery-4);
        }
    }
private:
    SDLState * sdl = nullptr;
    PlayerState * player = nullptr;
    MapAsset * map = nullptr;
    CardAsset * score = nullptr;
    std::vector<MobAsset *> mobs;
    std::vector<ParticleAsset *> particles;
    bool levelComplete = false;
    bool gameOver = false;
    bool firstRender = true;
    LevelSettings settings;
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
    virtual GameStage advance(GameStage currentStage)
    {
        if(completed)
        {
            return GameStage::Advance;
        }
        return currentStage;
    }
    virtual void keydown(SDL_Keycode keycode) {}
private:
    SDLState * sdl;
    CardAsset * logo;
    uint32_t firstTick;
    bool completed;
};

class TextCardScene : public GameScene
{
public:
    TextCardScene(SDLState * in_sdl, const char * in_text, DBShift in_shift = DBShift::NONE)
    : sdl(in_sdl), firstTick(0), lastTick(0), completed(false), banner(nullptr)
    {
        text_source.x = 0;
        text_source.y = 0;
        text_dest.x = GameConstants::Margin;
        text_dest.y = GameConstants::Margin;
        text_surface = TTF_RenderText_Blended_Wrapped(sdl->exposition_font,in_text,SDL_Color{0xff,0xff,0xff,0xff},GameConstants::ScreenWidth-(GameConstants::Margin*2));
        text_source.w = text_dest.w = text_surface->w;
        text_source.h = text_dest.h = text_surface->h;
        text_texture = SDL_CreateTextureFromSurface(sdl->renderer, text_surface);

        continueAsset = new CardAsset(sdl,sdl->exposition_font,GameContent::ContinueExposition);
        continueAsset->setX(GameConstants::ScreenWidth - continueAsset->getWidth() - GameConstants::Margin);
        continueAsset->setY(GameConstants::ScreenHeight - continueAsset->getHeight() - GameConstants::Margin);

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
            case DBShift::NONE:
            default:
                break;
        }
        if(nullptr!=banner)
        {
            banner->setY((GameConstants::ScreenHeight - banner->getHeight())/2);
        }
    }
    virtual ~TextCardScene()
    {
        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);
        delete continueAsset;
    }
    virtual void render()
    {
        if(firstTick==0)
        {
            firstTick = SDL_GetTicks();
            return;
        }

        uint32_t alpha = 0;
        if(lastTick == 0 || (lastTick - firstTick)<1000)
        {
            // (0-1000]    Ramp up
            // (1000,] Full on
            uint32_t deltaTicks =  SDL_GetTicks() - firstTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)((deltaTicks/1000.f)*255);
            }
            else
            {
                alpha = 255;
            }
            lastTick = 0;
        }
        else
        {
            // (0-1000]    Ramp down
            // (1000,]      We out
            uint32_t deltaTicks = SDL_GetTicks() - lastTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)(((1000-deltaTicks)/1000.f)*255);
            }
            else
            {
                completed = true;
                return;
            }
        }

        if(nullptr != banner)
        {
            banner->setAlpha(alpha/3);
            banner->render();
        }

        SDL_SetTextureAlphaMod(text_texture, alpha);
        SDL_RenderCopy(sdl->renderer, text_texture, &text_source, &text_dest);

        continueAsset->setAlpha(alpha);
        continueAsset->render();
    }
    virtual GameStage advance(GameStage currentStage)
    {
        if(completed)
        {
            return GameStage::Advance;
        }
        return currentStage;
    }
    virtual void keydown(SDL_Keycode keycode)
    {
        if(lastTick == 0)
        {
            lastTick = SDL_GetTicks();
        }
    }
private:
    SDLState * sdl;
    CardAsset * banner;
    SDL_Surface * text_surface;
    SDL_Texture * text_texture;
    SDL_Rect text_source;
    SDL_Rect text_dest;
    CardAsset * continueAsset;
    uint32_t firstTick;
    uint32_t lastTick;
    bool completed;
};

class TitleCardScene : public GameScene
{
public:
    TitleCardScene(SDLState * in_sdl)
    : sdl(in_sdl), firstTick(0), lastTick(0), completed(false)
    {
        title = new CardAsset(sdl, sdl->title_font,GameContent::Title);
        title->setX((GameConstants::ScreenWidth - title->getWidth())/2);
        title->setY((GameConstants::ScreenHeight - title->getHeight())/2);

        subtitle = new CardAsset(sdl, sdl->exposition_font,GameContent::Subtitle);
        subtitle->setX((GameConstants::ScreenWidth - subtitle->getWidth())/2);
        subtitle->setY(title->getY() + title->getHeight());
    }
    virtual ~TitleCardScene()
    {
        delete subtitle;
        delete title;
    }
    virtual void render()
    {
        if(firstTick==0)
        {
            firstTick = SDL_GetTicks();
            return;
        }
        
        uint32_t alpha = 0;
        if(lastTick == 0 || (lastTick - firstTick)<1000)
        {
            // (0-1000]    Ramp up
            // (1000,] Full on
            uint32_t deltaTicks =  SDL_GetTicks() - firstTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)((deltaTicks/1000.f)*255);
            }
            else
            {
                alpha = 255;
            }
            lastTick = 0;
        }
        else
        {
            // (0-1000]    Ramp down
            // (1000,]      We out
            uint32_t deltaTicks = SDL_GetTicks() - lastTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)(((1000-deltaTicks)/1000.f)*255);
            }
            else
            {
                completed = true;
                return;
            }
        }
        
        title->setAlpha(alpha);
        title->render();
        subtitle->setAlpha(alpha);
        subtitle->render();
    }
    virtual GameStage advance(GameStage currentStage)
    {
        if(completed)
        {
            return GameStage::Advance;
        }
        return currentStage;
    }
    virtual void keydown(SDL_Keycode keycode)
    {
        if(lastTick == 0)
        {
            lastTick = SDL_GetTicks();
        }
    }
private:
    SDLState * sdl;
    CardAsset * title;
    CardAsset * subtitle;
    uint32_t firstTick;
    uint32_t lastTick;
    bool completed;
};

class EndGameScene : public GameScene
{
public:
    EndGameScene(SDLState * in_sdl, PlayerState * in_player)
    : sdl(in_sdl), player(in_player), firstTick(0), lastTick(0), completed(false)
    {
        gameover = new CardAsset(sdl, sdl->title_font,GameContent::GameOverText);
        gameover->setX((GameConstants::ScreenWidth - gameover->getWidth())/2);
        gameover->setY((GameConstants::ScreenHeight - gameover->getHeight())/2);

        score = new CardAsset(sdl, sdl->title_font,GameContent::ScoreText, player->getScore());
        score->setX((GameConstants::ScreenWidth - score->getWidth())/2);
        score->setY(gameover->getY() + gameover->getHeight());
    }
    virtual ~EndGameScene()
    {
        delete gameover;
        delete score;
    }
    virtual void render()
    {
        if(firstTick==0)
        {
            firstTick = SDL_GetTicks();
            return;
        }
        
        uint32_t alpha = 0;
        if(lastTick == 0 || (lastTick - firstTick)<1000)
        {
            // (0-1000]    Ramp up
            // (1000,] Full on
            uint32_t deltaTicks =  SDL_GetTicks() - firstTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)((deltaTicks/1000.f)*255);
            }
            else
            {
                alpha = 255;
            }
            lastTick = 0;
        }
        else
        {
            // (0-1000]    Ramp down
            // (1000,]      We out
            uint32_t deltaTicks = SDL_GetTicks() - lastTick;
            if(deltaTicks<1000)
            {
                alpha = (uint32_t)(((1000-deltaTicks)/1000.f)*255);
            }
            else
            {
                completed = true;
                return;
            }
        }
        
        gameover->setAlpha(alpha);
        gameover->render();
        score->setAlpha(alpha);
        score->render();
    }
    virtual GameStage advance(GameStage currentStage)
    {
        if(completed)
        {
            return GameStage::Advance;
        }
        return currentStage;
    }
    virtual void keydown(SDL_Keycode keycode)
    {
        if(lastTick == 0)
        {
            lastTick = SDL_GetTicks();
        }
    }
private:
    SDLState * sdl;
    PlayerState * player;
    CardAsset * gameover;
    CardAsset * score;
    uint32_t firstTick;
    uint32_t lastTick;
    bool completed;
};

class GameManager
{
public:
    GameManager(SDLState * in_sdl)
    : sdl(in_sdl)
    {
        player = new PlayerState(sdl);
        scene = new SplashScene(sdl);
    }
    ~GameManager()
    {
        delete player;
        if(nullptr == scene)
        {
            delete scene;
        }
    }

    void render()
    {
        SDL_SetRenderDrawColor( sdl->renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear(sdl->renderer);

        if(nullptr == scene)
        {
            switch(s_state->stage)
            {
                case GameStage::Splash:
                    scene = new SplashScene(sdl);
                    break;
                case GameStage::Title:
                    scene = new TitleCardScene(sdl);
                    break;
                case GameStage::Controls:
                    scene = new TextCardScene(sdl,GameContent::Controls,DBShift::NONE);
                    break;
                case GameStage::AlphaFlightExposition:
                    scene = new TextCardScene(sdl,GameContent::AlphaFlightExposition,DBShift::AlphaFlight);
                    break;
                case GameStage::NightWatchExposition:
                    scene = new TextCardScene(sdl,GameContent::NightWatchExposition,DBShift::NightWatch);
                    break;
                case GameStage::ZetaShiftExposition:
                    scene = new TextCardScene(sdl,GameContent::ZetaShiftExposition,DBShift::ZetaShift);
                    break;
                case GameStage::DawnGuardExposition:
                    scene = new TextCardScene(sdl,GameContent::DawnGuardExposition,DBShift::DawnGuard);
                    break;
                case GameStage::LevelOne:
                    scene = new DesertLevel(sdl,player,GameContent::LevelOneSettings); 
                    break;
                case GameStage::LevelFour:
                    scene = new DesertLevel(sdl,player,GameContent::LevelTwoSettings);
                    break;
                case GameStage::LevelTwo:
                    scene = new DesertLevel(sdl,player,GameContent::LevelThreeSettings);
                    break;
                case GameStage::LevelThree:
                    scene = new DesertLevel(sdl,player,GameContent::LevelFourSettings);
                    break;
                case GameStage::GameOver:
                    scene = new EndGameScene(sdl,player);
                    break;
                case GameStage::Reset:
                case GameStage::Advance:
                default:
                    // Ignored
                    break;
            }
        }

        if(nullptr != scene)
        {
            scene->render();
            GameStage nextStage = scene->advance(s_state->stage);
            if(nextStage != s_state->stage)
            {
                delete scene;
                scene = nullptr;
            }
            if(nextStage==GameStage::Advance)
            {
                s_state->stage = (GameStage)((int32_t)(s_state->stage)+1);
                nextStage = s_state->stage;
            }
            else
            {
                s_state->stage = nextStage;
            }
            
            if(nextStage==GameStage::Reset)
            {
                delete player;
                player = new PlayerState(sdl);
                s_state->stage = GameStage::Title;
            }
        }

        SDL_RenderPresent(sdl->renderer);
    }

    void keydown(SDL_Keycode keycode)
    {
        if(nullptr != scene)
        {
            scene->keydown(keycode);
        }
    }

    PlayerState * player = nullptr;
    SDLState * sdl = nullptr;
private:
    GameScene * scene = nullptr;
};

GameManager * s_manager;

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
            s_manager->keydown(event.key.keysym.sym);
            break;
        }
        }

    }

    s_manager->render();
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
    RandomEngine.seed(time(0));
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDLState * sdl = new SDLState;

    sdl->window = SDL_CreateWindow(
        GameContent::Title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        GameConstants::ScreenWidth, GameConstants::ScreenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(sdl->renderer, 0xff, 0xff, 0xff, 0xff);

    sdl->title_font      = TTF_OpenFont("assets/kenney_pixel-webfont.ttf",64);
    sdl->exposition_font = TTF_OpenFont("assets/kenney_pixel-webfont.ttf",32);

    s_state = new GameState(sdl);
    s_manager = new GameManager(sdl);

    main_loop();

    delete s_manager;
    s_manager = nullptr;

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
