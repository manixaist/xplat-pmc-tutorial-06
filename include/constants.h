#pragma once
#include "SDL.h"

namespace XplatGameTutorial
{
namespace PacManClone
{
    class Constants
    {
    public:
        static const Uint16 ScreenWidth = 800;
        static const Uint16 ScreenHeight = 600;
        static const Uint32 FramesPerSecond = 60;
        static const Uint32 TicksPerFrame;
        static const SDL_Color SDLColorGrey;
        static const SDL_Color SDLColorMagenta;
        static const SDL_Color RenderDrawColor;
        static const SDL_Color BlinkyDrawColor;
        static const SDL_Color PinkyDrawColor;
        static const SDL_Color InkyDrawColor;
        static const SDL_Color ClydeDrawColor;
        static const char * const WindowTitle;
        static const Uint16 MapRows = 36;
        static const Uint16 MapCols = 28;
        static const Uint16 TileTextureWidth = 192;
        static const Uint16 TileTextureHeight = 192;
        static const Uint16 TileWidth = 16;
        static const Uint16 TileHeight = 16;
        static const Uint16 PlayerSpriteWidth = 32;
        static const Uint16 PlayerSpriteHeight = 32;
        static const Uint16 GhostSpriteWidth = 32;
        static const Uint16 GhostSpriteHeight = 32;
        static const Uint16 PlayerStartRow = 26;
        static const Uint16 PlayerStartCol = 13;
        static const Uint16 TotalPellets = 244;
        static const Uint32 LevelLoadDelay = 3000;
        static const Uint32 LevelCompleteDelay = 6000;
        static const Uint16 WarpRow = 17;
        static const Uint16 WarpColPlayerLeft = 0;
        static const Uint16 WarpColPlayerRight = 27;
        static const Uint16 WarpColGhostLeft = 1;
        static const Uint16 WarpColGhostRight = 26;
        static const Uint16 GhostPenRowExit = 14;
        static const Uint16 GhostPenRow = 17;
        static const Uint16 GhostPenCol = 13;
        static const Uint16 BlinkyScatterRow = 0;
        static const Uint16 BlinkyScatterCol = 25;
        static const Uint16 PinkyScatterRow = 0;
        static const Uint16 PinkyScatterCol = 2;
        static const Uint16 InkyScatterRow = 35;
        static const Uint16 InkyScatterCol = 27;
        static const Uint16 ClydeScatterRow = 35;
        static const Uint16 ClydeScatterCol = 0;

        static const double PlayerMaxSpeed;
        static const double GhostBaseSpeed;

        // Indices to tiles that make up the map - for your own sanity use a level editor (several free ones exist) or better
        // yet develop your own tool early in the design process
        //  We just have this one level we'll reuse, so just and paste as long as you don't change the order of the tiles.png
        static Uint16 MapIndicies[MapRows * MapCols];
        static Uint16 CollisionMap[MapRows * MapCols];

        static double CosineTable[1440];
        static double SineTable[1440];

        static const Uint16 PlayerAnimationSpeed = 5;
        static const Uint16 GhostAnimationSpeed = 8;

        // Animations
        static const Uint16 AnimationIndexUp = 0;
        static const Uint16 AnimationIndexDown = 1;
        static const Uint16 AnimationIndexLeft = 2;
        static const Uint16 AnimationIndexRight = 3;
        static const Uint16 AnimationIndexDeath = 4;
        static const Uint16 AnimationIndexFright = 4;  // Skipping death for ghosts since there are multiple
        static const Uint16 AnimationIndexScared = 5;
        static const Uint16 AnimationIndexDeathUp = 6;
        static const Uint16 AnimationIndexDeathDown = 7;
        static const Uint16 AnimationIndexDeathLeft = 8;
        static const Uint16 AnimationIndexDeathRight = 9;


        static const Uint16 PlayerTotalFrameCount = 20;
        static const Uint16 PlayerTotalAnimationCount = 5;
        static const Uint16 PlayerAnimationFrameCount = 4;
        static const Uint16 PlayerAnimationDeathFrameCount = 11;
        static int PlayerAnimation_UP[PlayerAnimationFrameCount];
        static int PlayerAnimation_DOWN[PlayerAnimationFrameCount];
        static int PlayerAnimation_LEFT[PlayerAnimationFrameCount];
        static int PlayerAnimation_RIGHT[PlayerAnimationFrameCount];
        static int PlayerAnimation_DEATH[PlayerAnimationDeathFrameCount];

        static const Uint16 GhostTotalFrameCount = 16;
        static const Uint16 GhostTotalAnimationCount = 10;
        static const Uint16 GhostMovingAnimationFrameCount = 2;
        static const Uint16 GhostScaredAnimationFrameCount = 4;
        static int GhostAnimation_UP[GhostMovingAnimationFrameCount];
        static int GhostAnimation_DOWN[GhostMovingAnimationFrameCount];
        static int GhostAnimation_LEFT[GhostMovingAnimationFrameCount];
        static int GhostAnimation_RIGHT[GhostMovingAnimationFrameCount];
        static int GhostAnimation_FRIGHT[GhostMovingAnimationFrameCount];
        static int GhostAnimation_SCARED[GhostScaredAnimationFrameCount];
        static int GhostAnimation_DEATHUP[GhostMovingAnimationFrameCount];
        static int GhostAnimation_DEATHDOWN[GhostMovingAnimationFrameCount];
        static int GhostAnimation_DEATHLEFT[GhostMovingAnimationFrameCount];
        static int GhostAnimation_DEATHRIGHT[GhostMovingAnimationFrameCount];

        // Strings
        static const char * const TilesImage;
        static const char * const SpritesImage;
        static const char * const TitleImage;

    private:
        static const Uint32 c_msPerSecond = 1000;
        static const Uint32 c_msPerFrame = (c_msPerSecond / FramesPerSecond);
    };
}
}
