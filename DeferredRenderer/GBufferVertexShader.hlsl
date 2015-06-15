// ----------------------------------------------------------------------------
// Structure definition
struct VS_OUTPUT
{
	float4 PosCS		: SV_POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PosWS		: POSITIONWS;
};

struct VS_INPUT
{
	float4 Position	: POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
};

// ----------------------------------------------------------------------------

// Constant buffers
cbuffer MatrixBuffer
{
	float4x4 WorldViewProjection;
	float4x4 World;
};

// ----------------------------------------------------------------------------

// Main
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.PosCS		= mul(input.Position, WorldViewProjection);
	output.TexCoord		= input.TexCoord;
	output.PosWS		= mul(input.Position, World).xyz;
	output.NormalWS		= normalize(mul(input.Normal, (float3x3)World));

	return output;
}

// ----------------------------------------------------------------------------