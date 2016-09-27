
#include "include/inky.h"

using namespace XplatGameTutorial::PacManClone;

Inky::Inky(TextureWrapper* pTextureWrapper) :
    Ghost(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight,
        Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount),
    _pBlinky(nullptr)
{
}

bool Inky::Initialize()
{
    // Each ghost will have its own set of frames, so each will override this method and implement
    // the specific loading data
    InitializeCommon();
    LoadFrames(0, 0, 128, 8);
    _scatterRow = Constants::InkyScatterRow;
    _scatterCol = Constants::InkyScatterCol;
    _targetColor = Constants::InkyDrawColor;
    return true;
}

bool Inky::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexUp);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::GhostPenRow, Constants::GhostPenCol - 2);

    // There is no "penned" mode, just placement will take care of that.  Inky is the only
    // ghost that is supposed to start outside of the pen, but since he's the only one for now
    // put him inside to test out that code path.
    _currentRow = Constants::GhostPenRow;
    _currentCol = Constants::GhostPenCol - 2;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(0, Constants::GhostBaseSpeed * -1.75);

    SafeDelete<Decision>(_pNextDecision);
    SafeDelete<Decision>(_pCurrentDecision);
    _pCurrentDecision = new Decision(Constants::GhostPenRow, Constants::GhostPenCol - 2, CurrentDirection());
    _penTimer.Reset();
    SetPenTimerMax(5000);
    _mode = Mode::Chase;
    _fScatter = false;

    return true;
}

Direction Inky::MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze)
{
    // Inky's target tile is an extension of the line between 2 tiles ahead of the player and
    // the other ghost "Blinky" by 2 (or 2 * DX, 2 * DY) from that point
    SDL_assert(GetBlinkyReference() != nullptr);

    Direction result = CurrentDirection();

    SDL_Point playerPoint = { static_cast<int>(pPlayer->X()), static_cast<int>(pPlayer->Y()) };
    if (_fScatter)
    {
        _targetRow = _scatterRow;
        _targetCol = _scatterCol;
    }
    else
    {
        pPlayer->GetTilePlayerFacingWithOriginalBug(pMaze, 2, _targetRow, _targetCol);

        SDL_Point blinkyPoint{ static_cast<int>(GetBlinkyReference()->X()), static_cast<int>(GetBlinkyReference()->Y()) };
        Uint16 blinkyRow = 0;
        Uint16 blinkyCol = 0;
        pMaze->GetTileRowCol(blinkyPoint, blinkyRow, blinkyCol);

        _targetRow = (2 * _targetRow) - blinkyRow;
        _targetCol = (2 * _targetCol) - blinkyCol;
    }
    return ShortestDirectionToTarget(nRow, nCol, _targetRow, _targetCol, pMaze);
}