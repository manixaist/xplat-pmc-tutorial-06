#pragma once
#include <stdio.h>
#include "constants.h"
#include "utils.h"
#include "player.h"
#include "blinky.h"
#include "pinky.h"
#include "inky.h"
#include "clyde.h"

namespace XplatGameTutorial
{
namespace PacManClone
{

// Encapsulates the game, tracks state, player, pellets, ghosts, score, etc
// Things that are tightly game sepcific should go here (e.g. PlayerSprite 
// vs 2DTiledMap which is more generic)
class GameHarness
{
public:
    GameHarness() :
        _fInitialized(false),
        _state(GameState::LoadingResources),
        _pSDLRenderer(nullptr),
        _pSDLWindow(nullptr),
        _pTilesTexture(nullptr),
        _pSpriteTexture(nullptr),
        _pMaze(nullptr),
        _pPlayer(nullptr),
        _pBlinky(nullptr),
        _pPinky(nullptr),
        _pInky(nullptr),
        _pClyde(nullptr)
    {
        for (size_t i = 0; i < SDL_arraysize(_pGhosts); i++)
        {
            _pGhosts[i] = nullptr;
        }
    }

    SDL_bool Initialize();  // Needs to be called successfully before Run()
    void Run();             // Main loop

private:
    enum class GameState
    {
        LoadingResources,       // Load resources (textures etc) from disk
        Title,                  // Eventual Title screen
        WaitingToStartLevel,    // Starting animation (gives the player a chance to get bearings)
        Running,                // Playing - most time should be in here! :)
        PlayerDying,            // Got caught by a ghost
        LevelComplete,          // Ate all the pellets on the current level (flashing level animation)
        GameOver,               // All lives are gone - cycles back to title after some time or input
        Exiting                 // App is closing
    };

    // Methods
    void Cleanup();
    void InitializeSprites();
    bool ProcessInput(Direction *pInputDirection);
    Uint16 HandlePelletCollision();
    GameState HandleGhostCollision();
    void Render();
    void RenderAITargets(size_t ghostIndex);
    void InitLevel();
    
    
    // GameState Handlers
    GameState OnLoading();
    GameState OnWaitingToStartLevel();
    GameState OnRunning();
    GameState OnLevelComplete();
    
    // Members
    bool _fInitialized;                 // Tracks if we've started SDL
    GameState _state;                   // current GameState
    SDL_Renderer *_pSDLRenderer;        // SDL renderer object
    SDL_Window *_pSDLWindow;            // SDL window object
    TextureWrapper *_pTilesTexture;     // Texture that holds the maze tiles
    TextureWrapper *_pSpriteTexture;    // Texture that holds the sprite frames
    TextureWrapper *_pTitleTexture;     // Texture that holds the title screen
    Maze *_pMaze;                       // Maze - playing area
    Player *_pPlayer;                   // The player sprite PacManClone
    Blinky *_pBlinky;                   // Blinky
    Pinky  *_pPinky;                    // Pinky
    Inky  *_pInky;                      // Inky
    Clyde *_pClyde;                     // Clyde
    Ghost* _pGhosts[4];                 // Stick our ghosts in here for easy access to common code
};
}
}