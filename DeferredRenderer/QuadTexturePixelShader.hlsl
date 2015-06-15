// ----------------------------------------------------------------------------
// Structure definition
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 WorldPos : POSITION;
};

// ----------------------------------------------------------------------------
Texture2D ObjTexture;
SamplerState ObjSamplerState;

// ----------------------------------------------------------------------------

// Main
float4 main(VS_OUTPUT input) : SV_TARGET
{
	return ObjTexture.Sample(ObjSamplerState, input.TexCoord);
}

// ----------------------------------------------------------------------------