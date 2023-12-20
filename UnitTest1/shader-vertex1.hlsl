
struct VertexShaderInput
{
    float4 Position : POSITION;
    float2 Texcoord : TEXCOORD0;
};
struct PixelShdareInput
{
    float4 Position : POSITION;
    float2 Texcoord : TEXCOORD0;
};

PixelShdareInput main(VertexShaderInput input)
{
    PixelShdareInput output = (PixelShdareInput) 0;
    output.Position = input.Position;
    output.Texcoord = input.Texcoord;
    return output;
}
