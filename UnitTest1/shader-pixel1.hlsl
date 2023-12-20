
Texture2D<float4> texture0 : register(t0);

SamplerState sampler0 : register(s0);

struct PixelShdareInput
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

float4 main(PixelShdareInput input) : SV_Target
{
    float4 output = texture0.Sample(sampler0, input.Texcoord);
  // output.x = 1;
    return output;
}
