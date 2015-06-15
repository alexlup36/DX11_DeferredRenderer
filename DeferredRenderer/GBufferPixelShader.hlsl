// ----------------------------------------------------------------------------
// Structure definition
struct PS_INPUT
{
	float4 PosSS		: SV_POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PosWS		: POSITIONWS;
};

struct PS_OUTPUT
{
	float4 Normal			: SV_Target0; // Render target 0
	float4 DiffuseAlbedo	: SV_Target1; // Render target 1
	float4 SpecularAlbedo	: SV_Target2; // Render target 2
	float4 Position			: SV_Target3; // Render target 3
};

// ----------------------------------------------------------------------------

// Constant buffers
cbuffer cbPerObject
{
	float3 SpecularAlbedo;
	float SpecularPower;
};

// ----------------------------------------------------------------------------

Texture2D DiffuseTexture;
SamplerState ObjSamplerState;

// ----------------------------------------------------------------------------

// Main
PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output;

	float3 diffuseAlbedo	= DiffuseTexture.Sample(ObjSamplerState, input.TexCoord).rgb;
	float3 normalWS			= normalize(input.NormalWS);

	output.Normal			= float4(normalWS, 1.0f);
	output.DiffuseAlbedo	= float4(diffuseAlbedo, 1.0f);
	output.SpecularAlbedo	= float4(SpecularAlbedo, SpecularPower);
	output.Position			= float4(input.PosWS, 1.0f);

	return output;
}

// ----------------------------------------------------------------------------