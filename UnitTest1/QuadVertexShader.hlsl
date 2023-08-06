
cbuffer ModelConstantBuffer : register(b0) {
  float4x4 model;
  float2 texCoordScale;
  float2 texCoordOffset;
}

// A constant buffer that stores each set of view and projection matrices in column-major format.
cbuffer ViewProjectionConstantBuffer : register(b1) {
  float4x4 viewProjection[2];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput {
  min16float3 pos : POSITION0;
  min16float2 texCoord : TEXCOORD0;
  uint instId : SV_InstanceID;
};

// Per-vertex data passed to the geometry shader.
// Note that the render target array index will be set by the geometry shader
// using the value of viewId.
struct VertexShaderOutput {
  min16float4 pos : SV_POSITION;
  min16float2 texCoord : TEXCOORD0;
  uint viewId : TEXCOORD16; // SV_InstanceID % 2
};

VertexShaderOutput main(VertexShaderInput input) {
  VertexShaderOutput output;
  float4 pos = float4(input.pos, 1.0f);
  output.pos = (min16float4)pos;
  float2 texCoord = float2(input.texCoord);
  output.texCoord = (min16float2)texCoord;
  output.viewId = input.instId % 2;
  return output;
}
