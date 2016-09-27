
#include "include/clyde.h"

using namespace XplatGameTutorial::PacManClone;

Clyde::Clyde(TextureWrapper* pTextureWrapper) :
    Ghost(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight,
        Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount)
{
}

bool Clyde::Initialize()
{
    // Each ghost will have its own set of frames, so each will override this method and implement
    // the specific loading data
    InitializeCommon();
    LoadFrames(0, 0, 160, 8);
    _scatterRow = 35;
    _scatterCol = 0;
    _targetColor = Constants::ClydeDrawColor;
    return true;
}

bool Clyde::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexUp);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::GhostPenRow, Constants::GhostPenCol + 1);

    // There is no "penned" mode, just placement will take care of that.  Clyde is the only
    // ghost that is supposed to start outside of the pen, but since he's the only one for now
    // put him inside to test out that code path.
    _currentRow = Constants::GhostPenRow;
    _currentCol = Constants::GhostPenCol + 1;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(0, Constants::GhostBaseSpeed * -1.75);

    SafeDelete<Decision>(_pNextDecision);
    SafeDelete<Decision>(_pCurrentDecision);
    _pCurrentDecision = new Decision(Constants::GhostPenRow, Constants::GhostPenCol + 1, CurrentDirection());
    _penTimer.Reset();
    SetPenTimerMax(8000);
    _mode = Mode::Chase;
    _fScatter = false;
    return true;
}

Direction Clyde::MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze)
{
    // Clyde's target tile is the player if we're within 8 squares, otherwise the target is
    // Clyde's home tile (35, 0) - bottom left corner
    Direction result = CurrentDirection();

    // Need Clyde's row/col
    Uint16 clydeRow = 0;
    Uint16 clydeCol = 0;
    SDL_Point clydePoint = { static_cast<int>(X()), static_cast<int>(Y()) };
    pMaze->GetTileRowCol(clydePoint, clydeRow, clydeCol);

    // Need player's row/col
    SDL_Point playerPoint = { static_cast<int>(pPlayer->X()), static_cast<int>(pPlayer->Y()) };
    pMaze->GetTileRowCol(playerPoint, _targetRow, _targetCol);

    // Logic
    if (Distance(clydeRow, clydeCol, _targetRow, _targetCol) < 8 || _fScatter)
    {
        _targetRow = _scatterRow;
        _targetCol = _scatterCol;
    }

    // Common return path
    return ShortestDirectionToTarget(nRow, nCol, _targetRow, _targetCol, pMaze);
}