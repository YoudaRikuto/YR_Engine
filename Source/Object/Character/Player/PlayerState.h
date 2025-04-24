#pragma once
#include "Ai/State.h"
#include "Player.h"

namespace PlayerState
{
    class IdleState : public State<Player>
    {
    public:
        IdleState(Player* player) : State(player, "IdleState") {}
        ~IdleState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class RunState : public State<Player>
    {
    public:
        RunState(Player* player) : State(player, "RunState") {}
        ~RunState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class RollState : public State<Player>
    {
    public:
        RollState(Player* player) : State(player, "RollState") {}
        ~RollState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;
    };

    class JumpState : public State<Player>
    {
    public:
        JumpState(Player* player) : State(player, "JumpState") {}
        ~JumpState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;
    };

    class DoubleJumpState : public State<Player>
    {
    public:
        DoubleJumpState(Player* player) : State(player, "DoubleJumpState") {}
        ~DoubleJumpState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;
    };
}