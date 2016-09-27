#pragma once
#include "ghost.h"

namespace XplatGameTutorial
{
    namespace PacManClone
    {
        // "Inky" type ghost.  
        class Inky : public Ghost
        {
        public:
            Inky(TextureWrapper* pTextureWrapper);

            // "Interface" for my ghosts to implement
            bool Initialize();
            bool Reset(Maze *pMaze);
            Direction MakeBranchDecision(Uint16 nRow, Uint16 nCol, Player* pPlayer, Maze *pMaze);

            void SetBlinkyReference(Ghost* pBlinkyGhost) { _pBlinky = pBlinkyGhost; }
            Ghost* GetBlinkyReference() { return _pBlinky; }


        private:
            Ghost *_pBlinky; // Not owned
        };
    }
}