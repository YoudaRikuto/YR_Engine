struct PSIn
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

cbuffer BloomConstants : register(b1)
{
    float bloomExtractionThreshold_;
    float bloomIntencity_;
}