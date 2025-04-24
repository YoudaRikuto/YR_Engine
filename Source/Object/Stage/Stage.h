#pragma once
#include "Object/Object.h"

class Stage : public Object
{
public:
    Stage();
    ~Stage() {}

    void Render(ID3D11PixelShader* psShader);
    void DrawDebug();
};

