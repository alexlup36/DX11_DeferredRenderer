// ----------------------------------------------------------------------------
// Structure definition
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 WorldPos : POSITION;
};

struct VS_INPUT
{
	float4 Position	: POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
};

// ----------------------------------------------------------------------------

// Constant buffers
cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

// ----------------------------------------------------------------------------

// Main
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Pos = mul(input.Position, WVP);
	output.WorldPos = mul(input.Position, World);
	output.TexCoord = input.TexCoord;
	output.Normal = mul(float4(input.Normal, 1.0f), World).xyz;

	return output;
}

// ----------------------------------------------------------------------------