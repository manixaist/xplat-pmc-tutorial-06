#pragma once
#include "ghost.h"

namespace XplatGameTutorial
{
    namespace PacManClone
    {
        // "Clyde" type ghost.  
        class Clyde : public Ghost
        {
        public:
            Clyde(TextureWrapper* pTextureWrapper);

            // "Interface" for my ghosts to implement
            bool Initialize();
            bool Reset(Maze *pMaze);
            Direction MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze);
        };
    }
}