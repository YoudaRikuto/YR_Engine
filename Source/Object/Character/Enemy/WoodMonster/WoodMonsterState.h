#pragma once
#include "WoodMonster.h"

namespace WoodMonsterState
{
    class IdleState : public State<WoodMonster>
    {
    public:
        IdleState(WoodMonster* woodMonster) : State(woodMonster, "IdleState") {}
        ~IdleState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };


}

