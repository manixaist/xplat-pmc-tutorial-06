#include "include/gameharness.h"

using namespace XplatGameTutorial::PacManClone;

// TEMP compile time flags for ghost enabling
#define GHOST_BLINKY
#define GHOST_PINKY
#define GHOST_INKY
#define GHOST_CLYDE

// requires Blinky so make sure we enabled it
// will crash otherwise
#ifdef GHOST_INKY
#ifndef GHOST_BLINKY
#define GHOST_BLINKY
#endif
#endif

// Duplicated code based on class type - perfect for a template function
// This creates an object if it does not already exist, and in all cases
// will Reset() the object
template <class T> void InitGameSprite(T** p, TextureWrapper* pTexture, Maze* pMaze)
{
    if (*p == nullptr)
    {
        *p = new T(pTexture);
        (*p)->Initialize();
    }
    (*p)->Reset(pMaze);
}

// Start up SDL and load our textures - the stuff we'll need for the entire process lifetime
SDL_bool GameHarness::Initialize()
{
    SDL_assert(_fInitialized == false);
    SDL_bool result = SDL_FALSE;
    if (InitializeSDL(&_pSDLWindow, &_pSDLRenderer) == SDL_TRUE)
    {
        // Load our textures
        SDL_Color colorKey = Constants::SDLColorMagenta;
        _pTilesTexture = new TextureWrapper(Constants::TilesImage, SDL_strlen(Constants::TilesImage), _pSDLRenderer, nullptr);
        _pSpriteTexture = new TextureWrapper(Constants::SpritesImage, SDL_strlen(Constants::SpritesImage), _pSDLRenderer, &colorKey);
        _pTitleTexture = new TextureWrapper(Constants::TitleImage, SDL_strlen(Constants::TitleImage), _pSDLRenderer, nullptr);

        if (_pTilesTexture->IsNull() || _pSpriteTexture->IsNull() || _pTitleTexture->IsNull())
        {
            printf("Failed to load one or more textures\n");
        }
        else
        {
            _fInitialized = true;
            result = SDL_TRUE;
        }
    }
    return result;
}

// Main loop, process window messages and dispatch to the current GameState handler
void GameHarness::Run()
{
    SDL_assert(_fInitialized);
    static bool fQuit = false;
    SDL_Event eventSDL;

    Uint32 startTicks;
    while (!fQuit)
    {
        startTicks = SDL_GetTicks();
        while (SDL_PollEvent(&eventSDL) != 0)
        {
            if (eventSDL.type == SDL_QUIT)
            {
                fQuit = true;
            }
        }

        if (!fQuit)
        {
            switch (_state)
            {
            case GameState::Title:
                Direction inputDirection;
                if (ProcessInput(&inputDirection))
                {
                    _state = GameState::Exiting;
                }
                else if (inputDirection != Direction::None)
                {
                    _state = GameState::WaitingToStartLevel;
                }
                break;
            case GameState::LoadingResources:
                // Loads the current maze and the sprites if needed
                _state = OnLoading();
                break;
            case GameState::WaitingToStartLevel:
                // Small delay before level starts
                _state = OnWaitingToStartLevel();
                break;
            case GameState::Running:
                // Normal gameplay
                _state = OnRunning();
                break;
            case GameState::PlayerDying:
                // Death animation, skip for now since no ghosts
                _state = GameState::WaitingToStartLevel;
                break;
            case GameState::LevelComplete:
                // Flashing level animation
                _state = OnLevelComplete();
                break;
            case GameState::GameOver:
                // Final drawing of level, score, etc
                break;
            case GameState::Exiting:
                fQuit = true;
                break;
            }

            // Draw the current frame
            Render();

            // TIMING
            // Fix this at ~c_framesPerSecond
            Uint32 endTicks = SDL_GetTicks();
            Uint32 elapsedTicks = endTicks - startTicks;
            if (elapsedTicks < Constants::TicksPerFrame)
            {
                SDL_Delay(Constants::TicksPerFrame - elapsedTicks);
            }
        }
    }

    // cleanup
    Cleanup();
}

void GameHarness::Cleanup()
{
    SDL_assert(_fInitialized);
    SafeDelete<TextureWrapper>(_pTitleTexture);
    SafeDelete<TextureWrapper>(_pTilesTexture);
    SafeDelete<TextureWrapper>(_pSpriteTexture);
    SafeDelete<Maze>(_pMaze);
    SafeDelete<Player>(_pPlayer);
    SafeDelete<Blinky>(_pBlinky);
    SafeDelete<Pinky>(_pPinky);
    SafeDelete<Inky>(_pInky);
    SafeDelete<Clyde>(_pClyde);

    // The _pGhosts array just holds references to deleted
    // objects, no need to free them

    SDL_DestroyRenderer(_pSDLRenderer);
    _pSDLRenderer = nullptr;

    SDL_DestroyWindow(_pSDLWindow);
    _pSDLWindow = nullptr;

    IMG_Quit();
    SDL_Quit();
    _fInitialized = false;
}

void GameHarness::InitializeSprites()
{
    // In all cases we create a player
    SDL_assert(_fInitialized);
    InitGameSprite(&_pPlayer, _pSpriteTexture, _pMaze);

    // The ghosts are controlled by these flags
#ifdef GHOST_BLINKY
    InitGameSprite(&_pBlinky, _pSpriteTexture, _pMaze);
    _pGhosts[0] = _pBlinky;
#endif

#ifdef GHOST_PINKY
    InitGameSprite(&_pPinky, _pSpriteTexture, _pMaze);
    _pGhosts[1] = _pPinky;
#endif

    // Will also enable blinky as he is needed for Inky's
    // targeting scheme
#ifdef GHOST_INKY
    InitGameSprite(&_pInky, _pSpriteTexture, _pMaze);
    _pInky->SetBlinkyReference(_pBlinky);
    _pGhosts[2] = _pInky;
#endif

#ifdef GHOST_CLYDE
    InitGameSprite(&_pClyde, _pSpriteTexture, _pMaze);
    _pGhosts[3] = _pClyde;
#endif
}

// Record key presses we care about
// returns true if we need to exit
bool GameHarness::ProcessInput(Direction *pInputDirection)
{
    *pInputDirection = Direction::None;
    bool fResult = false;

    // All it takes to get the key states.  The array is valid within SDL while running
    const Uint8 *pCurrentKeyState = SDL_GetKeyboardState(nullptr);

    if (pCurrentKeyState[SDL_SCANCODE_UP] || pCurrentKeyState[SDL_SCANCODE_W])
    {
        *pInputDirection = Direction::Up;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_DOWN] || pCurrentKeyState[SDL_SCANCODE_S])
    {
        *pInputDirection = Direction::Down;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_LEFT] || pCurrentKeyState[SDL_SCANCODE_A])
    {
        *pInputDirection = Direction::Left;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_RIGHT] || pCurrentKeyState[SDL_SCANCODE_D])
    {
        *pInputDirection = Direction::Right;
    }
    else if (pCurrentKeyState[SDL_SCANCODE_ESCAPE])
    {
        printf("ESC hit - exiting main loop...\n");
        fResult = true;
    }
    return fResult;
}

// Detect if the player has entered a pellet tile and remove it, incrementing our counter
// If the pellet is BIG, then trigger the ghost behavior
Uint16 GameHarness::HandlePelletCollision()
{
    Uint16 ret = 0;
    SDL_Point playerPoint = { static_cast<int>(_pPlayer->X()), static_cast<int>(_pPlayer->Y()) };
    Uint16 row = 0;
    Uint16 col = 0;
    _pMaze->GetTileRowCol(playerPoint, row, col);

    if (_pMaze->IsTilePellet(row, col))
    {
        _pMaze->EatPellet(row, col);
        ret++;
    }
    else if (_pMaze->IsTilePowerPellet(row, col))
    {
        _pMaze->EatPellet(row, col);
        ret++;

        for (size_t i = 0; i < SDL_arraysize(_pGhosts); i++)
        {
            if (_pGhosts[i] != nullptr)
            {
                _pGhosts[i]->OnPowerPelletEaten(_pMaze);
            }
        }
    }
    return ret;
}

// Detect if the player has collided with a ghost (i.e. they are in the 
// same cell during the same frame) and handle it based on state (whether
// the player has an active power pellet)
GameHarness::GameState GameHarness::HandleGhostCollision()
{
    GameState result = GameState::Running;

    Uint16 ret = 0;
    SDL_Point playerPoint = { static_cast<int>(_pPlayer->X()), static_cast<int>(_pPlayer->Y()) };
    Uint16 row = 0;
    Uint16 col = 0;
    _pMaze->GetTileRowCol(playerPoint, row, col);
    for (size_t i = 0; i < SDL_arraysize(_pGhosts); i++)
    {
        if (_pGhosts[i] != nullptr)
        {
            SDL_Point ghostPoint = { static_cast<int>(_pGhosts[i]->X()), static_cast<int>(_pGhosts[i]->Y()) };
            Uint16 ghostRow = 0;
            Uint16 ghostCol = 0;
            _pMaze->GetTileRowCol(ghostPoint, ghostRow, ghostCol);

            if (ghostRow == row && ghostCol == col)
            {
                if (_pGhosts[i]->OnPlayerCollision())
                {
                    result = GameState::PlayerDying;
                }
            }
        }
    }
    return result;
}

// Tell our object to draw (render their current texture to the renderer)
void GameHarness::Render()
{
    SDL_RenderClear(_pSDLRenderer);

    if (_state == GameState::Title)
    {
        if (_pTitleTexture != nullptr)
        {
            SDL_RenderCopy(
                _pSDLRenderer,
                _pTitleTexture->Ptr(),
                nullptr,
                nullptr);
        }
    }
    else
    {
        if (_pMaze != nullptr)
        {
            _pMaze->Render(_pSDLRenderer);
        }

        if (_pPlayer != nullptr)
        {
            _pPlayer->Render(_pSDLRenderer);
        }

        // This is common, so loop through our array
        for (size_t i = 0; i < SDL_arraysize(_pGhosts); i++)
        {
            if (_pGhosts[i] != nullptr)
            {
                _pGhosts[i]->Render(_pSDLRenderer);
                RenderAITargets(i);
            }
        }
    }
    SDL_RenderPresent(_pSDLRenderer);
}

// Small helper to factor out AI rendering for this module.  This code should not draw in a normal
// game but might be very helpful debugging
void GameHarness::RenderAITargets(size_t ghostIndex)
{
    Uint16 row = _pGhosts[ghostIndex]->TargetRow();
    Uint16 col = _pGhosts[ghostIndex]->TargetCol();

    SDL_Point targetPoint = _pMaze->GetTileCoordinates(row, col);
    targetPoint.x -= Constants::TileWidth / 2;
    targetPoint.y -= Constants::TileHeight / 2;
    SDL_Rect targetRect = { targetPoint.x, targetPoint.y, Constants::TileWidth, Constants::TileHeight };
    SDL_SetRenderDrawColor(
        _pSDLRenderer, 
        _pGhosts[ghostIndex]->TargetColor().r, 
        _pGhosts[ghostIndex]->TargetColor().g, 
        _pGhosts[ghostIndex]->TargetColor().b, 
        255);
    SDL_RenderFillRect(_pSDLRenderer, &targetRect);

    // Draw some specific UI to illustrate the AI targets and range
    if (ghostIndex == 2) // Inky
    {
        //                                     Target                     Blinky
        SDL_RenderDrawLine(_pSDLRenderer, targetPoint.x, targetPoint.y, _pGhosts[0]->X(), _pGhosts[0]->Y());
    }
    else if (ghostIndex == 3) // Clyde
    {
        SDL_Point clydePoint = { static_cast<int>(_pGhosts[ghostIndex]->X()), static_cast<int>(_pGhosts[ghostIndex]->Y()) };
        SDL_Point clydeCircle[SDL_arraysize(Constants::CosineTable)] = { 0,0 };
        // Draw 'circle' using pre-calculated cos/sin table
        for (size_t j = 0; j < SDL_arraysize(Constants::CosineTable); j++)
        {
            clydeCircle[j] = { static_cast<int>(clydePoint.x + (Constants::CosineTable[j] * 8 * Constants::TileWidth)),
                static_cast<int>(clydePoint.y + (Constants::SineTable[j] * 8 * Constants::TileHeight)) };
        }
        SDL_RenderDrawPoints(_pSDLRenderer, clydeCircle, SDL_arraysize(clydeCircle));
    }
    SDL_SetRenderDrawColor(_pSDLRenderer, Constants::RenderDrawColor.r, Constants::RenderDrawColor.g,
        Constants::RenderDrawColor.b, Constants::RenderDrawColor.a);
}

GameHarness::GameState GameHarness::OnLoading()
{
    InitLevel();
    
    // Precalculate our sin/cos table.  This could even be hardcoded, but it won't take long
    for (double i = 0; i < SDL_arraysize(Constants::CosineTable); i++)
    {
        Constants::CosineTable[static_cast<int>(i)] = SDL_cos(i/4);
        Constants::SineTable[static_cast<int>(i)] = SDL_sin(i/4);
    }
    return GameState::Title;
}

// This is the traditional delay before the level starts, normally you hear the little
// tune that signals play is about to begin, then you transition.  We have no sound yet
// so just delay the game a bit
GameHarness::GameState GameHarness::OnWaitingToStartLevel()
{
    static StateTimer timer;
    
    if (!timer.IsStarted())
    {
        timer.Start(Constants::LevelLoadDelay);
        InitLevel();
    }

    if (timer.IsDone())
    {
        timer.Reset();
        return GameState::Running;
    }
    return GameState::WaitingToStartLevel;
}

// Normal game play, check for collisions, update based on input, eventually the ghosts
// and their updates will need to be in here as well.  
GameHarness::GameState GameHarness::OnRunning()
{
    static Uint16 pelletsEaten = 0;
    GameState stateResult = GameState::Running;

    // INPUT
    Direction inputDirection = Direction::None;
    bool fQuit = ProcessInput(&inputDirection);
    if (!fQuit)
    {
        // UPDATE
        _pPlayer->Update(_pMaze, inputDirection); 
        pelletsEaten += HandlePelletCollision();

        // This is common, so loop through our array
        for (size_t i = 0; i < SDL_arraysize(_pGhosts); i++)
        {
            if (_pGhosts[i] != nullptr)
            {
                _pGhosts[i]->Update(_pPlayer, _pMaze);
            }
        }
        //stateResult = HandleGhostCollision();

        if (pelletsEaten == Constants::TotalPellets)
        {
            pelletsEaten = 0;
            return GameState::LevelComplete;
        }
    }
    else
    {
        // Input told us to exit above
        stateResult = GameState::Exiting;
    }
    return stateResult;
}

// All 244 pellets have been eaten, so we briefly flash the screen before moving to the
// next level.  We only have the one level, so it just restarts
GameHarness::GameState GameHarness::OnLevelComplete()
{
    static StateTimer timer;
    static Uint16 counter = 0;
    static bool flip;

    if (!timer.IsStarted())
    {
        counter = 0;
        flip = false;
        timer.Start(Constants::LevelCompleteDelay);
    }
    
    if (counter++ > 60)
    {
        counter = 0;
        flip = !flip;
    }

    // This will add a blue multiplier to the texture, making the shade chage.
    // We flip this back and forth roughly every second until the overall timer is done.
    SDL_SetTextureColorMod(_pTilesTexture->Ptr(), 255, 255, flip ? 100 : 255);
    
    if (timer.IsDone())
    {
        timer.Reset();
        return GameState::WaitingToStartLevel;
    }
    return GameState::LevelComplete;
}

void GameHarness::InitLevel()
{
    // This should be know, but it should also match what we just queried
    SDL_assert(_pTilesTexture->Width() == Constants::TileTextureWidth);
    SDL_assert(_pTilesTexture->Height() == Constants::TileTextureHeight);
    SDL_Rect textureRect{ 0, 0, Constants::TileTextureWidth, Constants::TileTextureHeight };

    SDL_SetTextureColorMod(_pTilesTexture->Ptr(), 255, 255, 255);

    // Initialize our tiled map object
    SafeDelete(_pMaze);
    _pMaze = new Maze(Constants::MapRows, Constants::MapCols, Constants::ScreenWidth, Constants::ScreenHeight);

    _pMaze->Initialize(textureRect, { 0, 0,  Constants::TileWidth,  Constants::TileHeight }, _pTilesTexture->Ptr(),
        Constants::MapIndicies, Constants::MapRows *  Constants::MapCols);

    // Clip around the maze so nothing draws there (this will help with the wrap around for example)
    SDL_Rect mapBounds = _pMaze->GetMapBounds();
    if (SDL_RenderSetClipRect(_pSDLRenderer, &mapBounds) != 0)
    {
        printf("SDL_RenderSetClipRect() failed, error = %s\n", SDL_GetError());
    }
    else
    {
        // Initialize our sprites
        InitializeSprites();
    }
}
