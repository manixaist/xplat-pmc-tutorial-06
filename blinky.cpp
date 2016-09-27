
#include "include/blinky.h"

using namespace XplatGameTutorial::PacManClone;

Blinky::Blinky(TextureWrapper* pTextureWrapper) :
    Ghost(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight,
        Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount)
{
}

bool Blinky::Initialize()
{
    // Each ghost will have its own set of frames, so each will override this method and implement
    // the specific loading data
    InitializeCommon();
    LoadFrames(0, 0, 64, 8);
    _scatterRow = Constants::BlinkyScatterRow;
    _scatterCol = Constants::BlinkyScatterCol;
    _targetColor = Constants::BlinkyDrawColor;
    return true;
}

bool Blinky::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexUp);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::GhostPenRow-3, Constants::GhostPenCol);
    
    // There is no "penned" mode, just placement will take care of that.  Blinky is the only
    // ghost that is supposed to start outside of the pen, but since he's the only one for now
    // put him inside to test out that code path.
    _currentRow = Constants::GhostPenRow-3;
    _currentCol = Constants::GhostPenCol;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(Constants::GhostBaseSpeed * -1.75, 0);

    SafeDelete<Decision>(_pNextDecision);
    SafeDelete<Decision>(_pCurrentDecision);
    _pCurrentDecision = new Decision(Constants::GhostPenRow-3, Constants::GhostPenCol, CurrentDirection());
    _penTimer.Reset();
    _mode = Mode::Chase;
    _fScatter = false;
    return true;
}

Direction Blinky::MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze)
{
    // Blinky's target tile is the player's current tile
    // We won't bother with "Elroy" states at the moment
    Direction result = CurrentDirection();

    // The "next" cell is already passed in here, given this location, find the branch
    // That brings us closest to the target cell (the player)
    SDL_Point playerPoint = { static_cast<int>(pPlayer->X()), static_cast<int>(pPlayer->Y()) };
    if (_fScatter)
    {
        _targetRow = _scatterRow;
        _targetCol = _scatterCol;
    }
    else
    {
        pMaze->GetTileRowCol(playerPoint, _targetRow, _targetCol);
    }
    return ShortestDirectionToTarget(nRow, nCol, _targetRow, _targetCol, pMaze);
}