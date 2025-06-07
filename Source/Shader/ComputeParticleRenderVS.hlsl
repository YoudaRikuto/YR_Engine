#include "ComputeParticle.hlsli"

GSIn main(uint vertexId : SV_VertexID)
{
    // ’¸“_”Ô†‘—‚é‚¾‚¯
    GSIn vsOut;
    vsOut.vertexId_ = vertexId;
    return vsOut;
}
