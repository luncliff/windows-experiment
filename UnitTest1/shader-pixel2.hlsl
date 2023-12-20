// https://learn.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering

Texture2D<float> luma : register(t0);
Texture2D<float2> chroma : register(t1);

SamplerState sampler0 : register(s0);

struct PixelShdareInput
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
};

static const float3x3 coefficients =
{
    1.164383f, 1.164383f, 1.164383f, //
    0.000000f, -0.391762f, 2.017232f, //
    1.596027f, -0.812968f, 0.000000f //
};

float3 ToRBG(float3 ycbcr)
{
    ycbcr -= float3(0.062745f, 0.501960f, 0.501960f);
    ycbcr = mul(ycbcr, coefficients);
    return saturate(ycbcr);
}

float4 main(PixelShdareInput input) : SV_TARGET
{
    float y = luma.Sample(sampler0, input.Texcoord);
    float2 cbcr = chroma.Sample(sampler0, input.Texcoord);
    float4 output = float4(ToRBG(float3(y, cbcr)), 1);
    return output;
}
