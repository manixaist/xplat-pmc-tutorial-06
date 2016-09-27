
#include "include/pinky.h"

using namespace XplatGameTutorial::PacManClone;

Pinky::Pinky(TextureWrapper* pTextureWrapper) :
    Ghost(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight,
        Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount)
{
}

bool Pinky::Initialize()
{
    // Each ghost will have its own set of frames, so each will override this method and implement
    // the specific loading data
    InitializeCommon();
    LoadFrames(0, 0, 96, 8);
    _scatterRow = Constants::PinkyScatterRow;
    _scatterCol = Constants::PinkyScatterCol;
    _targetColor = Constants::PinkyDrawColor;
    return true;
}

bool Pinky::Reset(Maze *pMaze)
{
    SetAnimation(Constants::AnimationIndexUp);
    SDL_Point playerStartCoord = pMaze->GetTileCoordinates(Constants::GhostPenRow, Constants::GhostPenCol + 2);

    // There is no "penned" mode, just placement will take care of that.  Pinky is the only
    // ghost that is supposed to start outside of the pen, but since he's the only one for now
    // put him inside to test out that code path.
    _currentRow = Constants::GhostPenRow;
    _currentCol = Constants::GhostPenCol + 2;
    ResetPosition(playerStartCoord.x, playerStartCoord.y);
    SetVelocity(0, Constants::GhostBaseSpeed * -1.75);

    SafeDelete<Decision>(_pNextDecision);
    SafeDelete<Decision>(_pCurrentDecision);
    _pCurrentDecision = new Decision(Constants::GhostPenRow, Constants::GhostPenCol + 2, CurrentDirection());
    _penTimer.Reset();
    SetPenTimerMax(2000);
    _mode = Mode::Chase;
    _fScatter = false;

    return true;
}

Direction Pinky::MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze)
{
    // Pinky's target tile is 4 tiles ahead of the player's current tile
    Direction result = CurrentDirection();

    SDL_Point playerPoint = { static_cast<int>(pPlayer->X()), static_cast<int>(pPlayer->Y()) };
    if (_fScatter)
    {
        _targetRow = _scatterRow;
        _targetCol = _scatterCol;
    }
    else
    {
        pPlayer->GetTilePlayerFacingWithOriginalBug(pMaze, 4, _targetRow, _targetCol);
    }
    return ShortestDirectionToTarget(nRow, nCol, _targetRow, _targetCol, pMaze);
}