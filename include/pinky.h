#pragma once
#include "ghost.h"

namespace XplatGameTutorial
{
    namespace PacManClone
    {
        // "Pinky" type ghost.  
        class Pinky : public Ghost
        {
        public:
            Pinky(TextureWrapper* pTextureWrapper);

            // "Interface" for my ghosts to implement
            bool Initialize();
            bool Reset(Maze *pMaze);
            Direction MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze);
        };
    }
}