#pragma once
#include <vector>
#include <string>
#include "State.h"
#include "ImGui/ImGuiCtrl.h"

template<class T>
class StateMachine
{
public:
    StateMachine() {}
    ~StateMachine();

    void Update(const float& elapsedTime);
    void DrawDebug();

    void SetState(const int& newState);
    void ChangeState(const int& newState);
    void RegisterState(T* state);

    T* GetCurrentState() { return currentState_; }

private:
    T*              currentState_;
    std::vector<T*> statePool_;
};

template<class T>
inline StateMachine<T>::~StateMachine()
{
    // 登録したステートを削除する
    for (T* state : statePool_)
    {
        delete state;
    }
    statePool_.clear();
}

template<class T>
inline void StateMachine<T>::Update(const float& elapsedTime)
{
    currentState_->Update(elapsedTime);
}

template<class T>
inline void StateMachine<T>::DrawDebug()
{
    if (ImGui::TreeNodeEx("StateMachine", ImGuiTreeNodeFlags_Framed))
    {
        const std::string stateText = "CurrentState : ";
        std::string stateName = currentState_->GetName();
        stateName = stateText + stateName;

        ImGui::Text(stateName.c_str());
        
        ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(500, 400), ImGuiWindowFlags_NoTitleBar);

        for (T* state : statePool_)
        {
            state->DrawDebug();
        }

        ImGui::EndChild();

        ImGui::TreePop();
    }
}

template<class T>
inline void StateMachine<T>::SetState(const int& newState)
{
    currentState_ = statePool_.at(newState);
    currentState_->Initialize();
}

template<class T>
inline void StateMachine<T>::ChangeState(const int& newState)
{
    currentState_->Finalize();
    SetState(newState);
}

template<class T>
inline void StateMachine<T>::RegisterState(T* state)
{
    statePool_.emplace_back(state);
}