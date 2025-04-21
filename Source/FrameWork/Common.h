#pragma once
#include <DirectXMath.h>
#include <stdlib.h>

template<class T>
inline void SafeDeletePtr(T*& p)
{
    if (p == nullptr) return;

    delete (p);
    p = nullptr;
}