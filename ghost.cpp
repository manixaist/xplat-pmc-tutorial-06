#include "include/ghost.h"
#include "include/constants.h"

using namespace XplatGameTutorial::PacManClone;

Ghost::Ghost(TextureWrapper *pTextureWrapper, Uint16 /*cxFrame*/, Uint16 /*cyFrame*/, Uint16 /*cFramesTotal*/, Uint16 /*cAnimationsTotal*/) :
    Sprite(pTextureWrapper, Constants::GhostSpriteWidth, Constants::GhostSpriteHeight, Constants::GhostTotalFrameCount, Constants::GhostTotalAnimationCount),
    _currentRow(0),
    _currentCol(0),
    _scatterRow(0),
    _scatterCol(0),
    _targetRow(0),
    _targetCol(0),
    _targetColor(Constants::SDLColorGrey),
    _penTimerMax(0),
    _mode(Mode::Chase),
    _fScatter(false),
    _pNextDecision(nullptr),
    _pCurrentDecision(nullptr),
    _pPrevDecision(nullptr)
{
}

// Call the subroutine based on our internal state
void Ghost::Update(Player* pPlayer, Maze* pMaze)
{
    switch (_mode)
    {
    case Mode::ExitingPen:
        OnExitingPen(pPlayer, pMaze);
        break;
    case Mode::WarpingOut:
        OnWarpingOut(pPlayer, pMaze);
        break;
    case Mode::WarpingIn:
        OnWarpingIn(pPlayer, pMaze);
        break;
    case Mode::Chase:
        OnChasing(pPlayer, pMaze);
        break;
    }
}

void Ghost::OnPowerPelletEaten(Maze* pMaze)
{
    // Called by the GameHarness when the player eats a pellet
    _fScatter = true;
    
    if (!_scatterTimer.IsStarted())
    {
        _scatterTimer.Start(10000);
    }
    else
    {
        _scatterTimer.Reset();
        _scatterTimer.Start(10000);
    }
    if (_mode == Mode::Chase)
    {
        // Don't do this for other cases like warping
        // Let the velocity stay managed by those handlers
        ReverseDirection();
    }
    UpdateAnimation(CurrentDirection());
}

bool Ghost::OnPlayerCollision()
{
    return !_fScatter;
}

void Ghost::InitializeCommon()
{
    LoadFrames(8, 256, 64, 2);
    LoadFrames(10, 256, 96, 2);
    LoadFrames(12, 256, 128, 2);
    LoadFrames(14, 256, 150, 2);
    LoadAnimationSequence(Constants::AnimationIndexLeft, AnimationType::Loop, Constants::GhostAnimation_LEFT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexRight, AnimationType::Loop, Constants::GhostAnimation_RIGHT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexUp, AnimationType::Loop, Constants::GhostAnimation_UP, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDown, AnimationType::Loop, Constants::GhostAnimation_DOWN, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexFright, AnimationType::Loop, Constants::GhostAnimation_FRIGHT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexScared, AnimationType::Loop, Constants::GhostAnimation_SCARED, Constants::GhostScaredAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDeathLeft, AnimationType::Loop, Constants::GhostAnimation_DEATHLEFT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDeathRight, AnimationType::Loop, Constants::GhostAnimation_DEATHRIGHT, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDeathUp, AnimationType::Loop, Constants::GhostAnimation_DEATHUP, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    LoadAnimationSequence(Constants::AnimationIndexDeathDown, AnimationType::Loop, Constants::GhostAnimation_DEATHDOWN, Constants::GhostMovingAnimationFrameCount, Constants::GhostAnimationSpeed);
    SetFrameOffset(1 - (Constants::GhostSpriteWidth / 2), 1 - (Constants::GhostSpriteHeight / 2));
}

Direction Ghost::ShortestDirectionToTarget(Uint16 originRow, Uint16 originCol, Uint16 targetRow, Uint16 targetCol, Maze *pMaze)
{
    Direction result = Direction::None;

    // we know this cell should be an intersection
    SDL_assert(pMaze->IsTileIntersection(originRow, originCol) == SDL_TRUE);

    // This means there should be at least 2 options to pick from minus the
    // reverse of our current direction which is invalid
    // ...
    struct MAZECELL
    {
        Uint16 row;
        Uint16 col;
        double distance;
        bool valid;
    };

    // Direction order should match the Direction Enum for easy array access
    MAZECELL options[] = {
        { static_cast<Uint16>(originRow - 1), originCol, 0, false }, // UP
        { static_cast<Uint16>(originRow + 1), originCol, 0, false }, // DOWN
        { originRow, static_cast<Uint16>(originCol - 1), 0, false }, // LEFT
        { originRow, static_cast<Uint16>(originCol + 1), 0, false }  // RIGHT
    };

    for (size_t index = 0; index < SDL_arraysize(options); index++)
    {
        options[index].valid = (pMaze->IsTileSolid(options[index].row, options[index].col) == SDL_FALSE);
        if (Opposite(static_cast<Direction>(index)) == CurrentDirection())
        {
            options[index].valid = false; // even though it's non solid
        }

        if (options[index].valid)
        {
            //Distance Cell and Player(P, C)
            options[index].distance = Distance(options[index].row, options[index].col, targetRow, targetCol);
        }
    }

    size_t index = 0;
    size_t shortest = 0;
    for (;index < SDL_arraysize(options); index++)
    {
        if (options[index].valid)
        {
            shortest = index;
            break;
        }
    }

    for (index = shortest + 1;index < SDL_arraysize(options); index++)
    {
        if (options[index].distance < options[shortest].distance && options[index].valid)
        {
            shortest = index;
        }
    }

    SDL_assert(options[shortest].valid);

    // Remember I said the order should match the enum?
    switch (shortest)
    {
    case 0:
        result = Direction::Up;
        break;
    case 1:
        result = Direction::Down;
        break;
    case 2:
        result = Direction::Left;
        break;
    case 3:
        result = Direction::Right;
        break;
    }

    return result;
}

// The conditions under which this is called is when the current cell is
// *NOT* an intersection, and thus should only have 1 valid exit that is not
// in the reverse direction of the sprite
Direction Ghost::GetNextDirection(Uint16 r, Uint16 c, Maze *pMaze)
{
    Direction options[] = // Logic assumes the order here matches the enum
    {
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right
    };

    // This option is automatically invalid
    size_t oppositeOption = static_cast<size_t>(Opposite(_pCurrentDecision->GetDirection()));
    SDL_assert(oppositeOption != static_cast<size_t>(Direction::None));

    // Now there are 3 options left
    // Only one of them should be free
    for (size_t index = 0; index < SDL_arraysize(options); index++)
    {
        if (index != oppositeOption)
        {
            Uint16 row = r;
            Uint16 col = c;
            TranslateCell(row, col, options[index]);
            if (pMaze->IsTileSolid(row, col) == SDL_FALSE)
            {
                return options[index];
            }
        }
    }

    // Should never get here
    return Direction::None;
}

// Look ahead one tile and make a decision about what to do when we
// eventually get there.  If the tile is an intersection, we will ask
// our specific ghost implementation what to do.
Ghost::Decision* Ghost::GetNextDecision(Player *pPlayer, Maze* pMaze)
{
    // Record current cell
    SDL_Point ghostPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
    pMaze->GetTileRowCol(ghostPoint, _currentRow, _currentCol);

    // Get the next cell based only on Direction of current decision
    Uint16 r = _currentRow;
    Uint16 c = _currentCol;
    TranslateCell(r, c, _pCurrentDecision->GetDirection());

    // This cell should be free
    SDL_assert(pMaze->IsTileSolid(r, c) == SDL_FALSE);

    Direction newDirection = Direction::None;
    // Is the next cell an intersection?
    if (pMaze->IsTileIntersection(r, c))
    {
        // Yes - Now we need to as the derived class
        newDirection = MakeBranchDecision(r, c, pPlayer, pMaze);
    }
    else
    {
        // Should only be one option left
        newDirection = GetNextDirection(r, c, pMaze);
    }

    return new Decision(r, c, newDirection);
}

bool Ghost::IsGhostWarpingOut(Maze* pMaze)
{
    SDL_Point updatedPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
    Uint16 row = 0;
    Uint16 col = 0;
    pMaze->GetTileRowCol(updatedPoint, row, col);
    // Unlike the player, start warping 1 more tile inside, this is because the
    // ghost logic looks ahead one tile in normal mode and this will ensure it
    // is always in bounds of our map.  We have no need of the map indicies while
    // in "warp" mode, so just make sure we're in bounds again before changing
    // state back to Chase.
    return ((row == Constants::WarpRow) && ((col == Constants::WarpColGhostLeft) || (col == Constants::WarpColGhostRight)));
}

void Ghost::OnExitingPen(Player* pPlayer, Maze* pMaze)
{
    Sprite::Update();
    // Check if we're done exiting
    // Then change to chase mode
    SDL_Point centerPoint = pMaze->GetTileCoordinates(14, 13);
    if (pMaze->IsSpritePastCenter(Constants::GhostPenRowExit, Constants::GhostPenCol, this))
    {
        ResetPosition(centerPoint.x, centerPoint.y);
        _currentRow = Constants::GhostPenRowExit;
        _currentCol = Constants::GhostPenCol;
        SafeDelete<Decision>(_pPrevDecision);
        SafeDelete<Decision>(_pNextDecision);
        SafeDelete<Decision>(_pCurrentDecision);

        double speed = Constants::GhostBaseSpeed * 1.75;
        if (pPlayer->X() < X())
        {
            speed = speed * -1;
        }

        SetVelocity(speed, 0.0);
        _pCurrentDecision = new Decision(Constants::GhostPenRowExit, Constants::GhostPenCol, CurrentDirection());
        _mode = Mode::Chase;
    }
}

// Just like the player, keep moving until out of view, but unlike the player, the 
// ghost will incur a speed penalty while warping
void Ghost::OnWarpingOut(Player* /*pPlayer*/, Maze* pMaze)
{
    Sprite::Update();
    SDL_Rect mapRect = pMaze->GetMapBounds();
    if (IsOutOfView(mapRect))
    {
        if (DX() > 0)
        {
            ResetPosition(mapRect.x - Width(), Y());
            _mode = Mode::WarpingIn;
        }
        else if (DX() < 0)
        {
            ResetPosition(mapRect.x + mapRect.w + Width(), Y());
            _mode = Mode::WarpingIn;
        }
    }
}

void Ghost::OnWarpingIn(Player* /*pPlayer*/, Maze* pMaze)
{
    // Maintain current velocity until we're back in frame
    Sprite::Update();
    SDL_Point ghostPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
    Uint16 row, col;
    pMaze->GetTileRowCol(ghostPoint, row, col);
    // We stay in this state until we're 1 tile in from the "warp out" tile, this way
    // We won't immediately reenter the WarpingOut state and we can't turn anyway with
    // the map design, so this is an optimization
    if ((row == Constants::WarpRow) && ((col == 2) || (col == 25)))
    {
        // Remove the speed penalty
        SetVelocity(2.0 * DX(), 2.0 * DY());
        _currentRow = row;
        _currentCol = col;
        // Need a new decision as well
        SafeDelete<Decision>(_pPrevDecision);
        SafeDelete<Decision>(_pNextDecision);
        SafeDelete<Decision>(_pCurrentDecision);
        _pCurrentDecision = new Decision(row, col, CurrentDirection());
        _mode = Mode::Chase;
    }
}

void Ghost::OnChasing(Player* pPlayer, Maze* pMaze)
{
    if (IsGhostPenned())
    {
        // Should we release it?
        if (!_penTimer.IsStarted())
        {
            // Simple timer for now
            _penTimer.Start(_penTimerMax);
        }
        else if (_penTimer.IsDone())
        {
            // Place below pen and move upward to outer row
            SDL_Point exitPoint = pMaze->GetTileCoordinates(17, 13);
            ResetPosition(exitPoint.x, exitPoint.y);
            SetAnimation(Constants::AnimationIndexUp);
            SetVelocity(0.0, Constants::GhostBaseSpeed * -1.75);
            _mode = Mode::ExitingPen;
        }
    }
    else if (_fScatter)
    {
        if (_scatterTimer.IsDone())
        {
            _fScatter = false;
        }
    }
    // Otherwise the chase logic is exactly the same, it just can't reach the player

    if (_mode != Mode::ExitingPen)
    {
        // Move along the current direction, but never further than the centerpoint
        // of the given cell
        SDL_Point centerPoint = pMaze->GetTileCoordinates(_currentRow, _currentCol);
        Sprite::Update();
        if (pMaze->IsSpritePastCenter(_currentRow, _currentCol, this) &&
            _pCurrentDecision->GetDirection() != CurrentDirection())
        {
            ResetPosition(centerPoint.x, centerPoint.y);
            Stop();
        }
        else
        {
            SDL_assert(_pCurrentDecision != nullptr);
            if (_pNextDecision == nullptr)
            {
                _pNextDecision = GetNextDecision(pPlayer, pMaze);
            }

            SDL_Point updatedPoint = { static_cast<int>(X()), static_cast<int>(Y()) };
            Uint16 row = 0;
            Uint16 col = 0;
            pMaze->GetTileRowCol(updatedPoint, row, col);

            if ((row != _currentRow) || (col != _currentCol))
            {
                // Entering a new cell
                _currentRow = row;
                _currentCol = col;
                SDL_assert(_pNextDecision != nullptr);
                SafeDelete<Decision>(_pPrevDecision);
                _pPrevDecision = _pCurrentDecision;
                _pCurrentDecision = _pNextDecision;
                _pNextDecision = nullptr;

                // Did we move into a warp cell?
                if (IsGhostWarpingOut(pMaze))
                {
                    // Add a speed penalty
                    SetVelocity(0.5 * DX(), 0.5 * DY());
                    _mode = Mode::WarpingOut;
                }
            }
            else
            {
                if (IsStopped())
                {
                    // Set Direction
                    UpdateAnimation(_pCurrentDecision->GetDirection());
                }
            }
        }
    }
}

void Ghost::UpdateAnimation(Direction direction)
{
    // Set Direction
    if (_fScatter)
    {
        SetAnimation(Constants::AnimationIndexFright);
    }

    switch (direction)
    {
    case Direction::Up:
        SetVelocity(0, Constants::GhostBaseSpeed * -1.75);
        if (!_fScatter)
        {
            SetAnimation(Constants::AnimationIndexUp);
        }
        break;
    case Direction::Down:
        SetVelocity(0, Constants::GhostBaseSpeed * 1.75);
        if (!_fScatter)
        {
            SetAnimation(Constants::AnimationIndexDown);
        }
        break;
    case Direction::Left:
        SetVelocity(Constants::GhostBaseSpeed * -1.75, 0);
        if (!_fScatter)
        {
            SetAnimation(Constants::AnimationIndexLeft);
        }
        break;
    case Direction::Right:
        SetVelocity(Constants::GhostBaseSpeed * 1.75, 0);
        if (!_fScatter)
        {
            SetAnimation(Constants::AnimationIndexRight);
        }
        break;
    case Direction::None:
        break;
    }
}

void Ghost::ReverseDirection()
{
    // this should be safe in all cases
    SetVelocity(DX() * -1, DY() * -1);
    SafeDelete<Decision>(_pNextDecision);
 
    Decision *tmp = _pCurrentDecision;
    Direction dir = Opposite(_pPrevDecision->GetDirection());
    _pCurrentDecision = new Decision(_currentRow, _currentCol, dir);

    SafeDelete<Decision>(_pPrevDecision);
    SafeDelete<Decision>(tmp);

}