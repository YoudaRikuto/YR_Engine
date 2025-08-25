#include "GltfModel.hlsli"

PSIn main(VSIn vsIn)
{
    float sigma = vsIn.tangent_.w;
    
    if (skin_ > -1)
    {
        float totalWeight = dot(vsIn.weights_[0], 1) + dot(vsIn.weights_[1], 1);

        row_major float4x4 skinMatrix =
        vsIn.weights_[0].x / totalWeight * jointMatrices_[vsIn.joints_[0].x] +
        vsIn.weights_[0].y / totalWeight * jointMatrices_[vsIn.joints_[0].y] +
        vsIn.weights_[0].z / totalWeight * jointMatrices_[vsIn.joints_[0].z] +
        vsIn.weights_[0].w / totalWeight * jointMatrices_[vsIn.joints_[0].w] +
        vsIn.weights_[1].x / totalWeight * jointMatrices_[vsIn.joints_[1].x] +
        vsIn.weights_[1].y / totalWeight * jointMatrices_[vsIn.joints_[1].y] +
        vsIn.weights_[1].z / totalWeight * jointMatrices_[vsIn.joints_[1].z] +
        vsIn.weights_[1].w / totalWeight * jointMatrices_[vsIn.joints_[1].w];
        
        vsIn.position_ = mul(float4(vsIn.position_.xyz, 1), skinMatrix);
        vsIn.normal_ = normalize(mul(float4(vsIn.normal_.xyz, 0), skinMatrix));
        vsIn.tangent_ = normalize(mul(float4(vsIn.tangent_.xyz, 0), skinMatrix));
    }
    
    PSIn vsOut;
    
    vsIn.position_.w = 1;
    vsOut.position_ = mul(vsIn.position_, mul(world_, viewProjection_));
    vsOut.worldPosition_ = mul(vsIn.position_, world_);
    
    vsIn.normal_.w = 0;
    vsOut.worldNormal_ = normalize(mul(vsIn.normal_, world_));
    
    vsIn.tangent_.w = 0;
    vsOut.worldTangent_ = normalize(mul(vsIn.tangent_, world_));
    vsOut.worldTangent_.w = sigma;
    
    //vsOut.texcoord_ = vsIn.texcoord_;
    vsOut.texcoord_ = vsIn.texcoord_ + scrollDirection_ * scrollTimer_;

    return vsOut;
}