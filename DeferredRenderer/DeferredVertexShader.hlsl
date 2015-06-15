// ----------------------------------------------------------------------------
// Structure definition
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord	: TEXCOORD;
};

struct VS_INPUT
{
	float4 Position	: POSITION;
	float2 TexCoord	: TEXCOORD;
};

// ----------------------------------------------------------------------------

// Constant buffers
cbuffer cbPerObject
{
	float4x4 WVP;
};

// ----------------------------------------------------------------------------

// Main
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, WVP);
	output.TexCoord = input.TexCoord;

	return output;
}

// ----------------------------------------------------------------------------