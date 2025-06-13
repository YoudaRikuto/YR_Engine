#include "SkyMap.hlsli"

PSIn main(in uint vertexId : SV_VERTEXID)
{
    PSIn vsOut;
    
    const float2 position[4] = { { -1, +1 }, { +1, +1 }, { -1, -1 }, { +1, -1 } };
    const float2 texcoord[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
    vsOut.position_ = float4(position[vertexId], 0, 1);
    vsOut.texcoord_ = texcoord[vertexId];
    
    return vsOut;
}