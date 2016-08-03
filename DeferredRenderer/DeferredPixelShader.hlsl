#define LIGHTCOUNT 144

// ----------------------------------------------------------------------------
// Structure definition
struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord	: TEXCOORD;
};

// ----------------------------------------------------------------------------
// Textures for deferred rendering

Texture2D NormalTexture				: register(t0);
Texture2D DiffuseAlbedoTexture		: register(t1);
Texture2D SpecularAlbedoTexture		: register(t2);
Texture2D PositionTexture			: register(t3);

// ----------------------------------------------------------------------------

struct Light
{
	float3		dir;
	float3		pos;
	float		range;
	float3		att;
	float3		ambient;
	float3		diffuse;
	float3		specular;
	float2		spotlightAngles;
};

// ----------------------------------------------------------------------------
// Sampler state
SamplerState samplerObj;

// ----------------------------------------------------------------------------
// Constant buffers
cbuffer cbPerFrame
{
	Light light[LIGHTCOUNT];
	float3 CameraPosition;
};

// ----------------------------------------------------------------------------
// Auxiliary method for sampling the G-Buffer attributes
void GetGBufferAttributes(in float2 texCoord,
	out float3 normal,
	out float3 position,
	out float3 diffuseAlbedo,
	out float3 specularAlbedo,
	out float specularPower)
{
	// Sample normal texture
	normal = NormalTexture.Sample(samplerObj, texCoord).xyz;

	// Sample position texture
	position = PositionTexture.Sample(samplerObj, texCoord).xyz;

	// Sample diffuse albedo
	diffuseAlbedo = DiffuseAlbedoTexture.Sample(samplerObj, texCoord).xyz;

	// Sample specular albedo
	float4 specularData = SpecularAlbedoTexture.Sample(samplerObj, texCoord);
	specularAlbedo = specularData.xyz;
	specularPower = specularData.w;
}

// ----------------------------------------------------------------------------
// Auxiliary method for calculating the lighing based on the sampled values
float3 CalculateLighting(int lightIndex, 
	in float3 normal,
	in float3 position,
	in float3 diffuseAlbedo,
	in float3 specularAlbedo,
	in float specularPower)
{
	Light currentLight = light[lightIndex];

	// Light vector
	float3 LightVector = 0;

	// Calculate the light vector
	LightVector = currentLight.pos - position;

	// Calculate the attenuation based on the distance to the light source
	float dist = length(LightVector);
	// Normalize light vector
	LightVector /= dist;

	float d = max(0.0f, dist - currentLight.range);
	float denom = d / currentLight.range + 1.0f;
	float Attenuation = 1.0f / (denom * denom);
	float cutoff = 0.005f;
	Attenuation = (Attenuation - cutoff) / (1.0f - cutoff);
	Attenuation = max(0.0f, Attenuation);

	// Calculate the diffuse component
	float normalDotLight = saturate(dot(normal, LightVector));
	float3 diffuseComponent = currentLight.diffuse * diffuseAlbedo * normalDotLight;

	// Calculate the specular component
	float3 SurfaceToCamera = CameraPosition - position;
	float3 HalfVector = normalize(LightVector + SurfaceToCamera);
	float3 specularComponent = specularAlbedo * currentLight.specular * normalDotLight * pow(saturate(dot(normal, HalfVector)), specularPower);

	// Calculate the final color
	return saturate((diffuseComponent + specularComponent) * Attenuation);
}

// ----------------------------------------------------------------------------
// Main
float4 main(PS_INPUT input) : SV_TARGET
{
	float3 normal;
	float3 position;
	float3 diffuseAlbedo;
	float3 specularAlbedo;
	float specularPower;

	float3 lighting = float3(0.0f, 0.0f, 0.0f);

	// Sample the GBuffer properties
	GetGBufferAttributes(input.TexCoord, normal, position, diffuseAlbedo, specularAlbedo, specularPower);

	for (int iPointLightIndex = 0; iPointLightIndex < LIGHTCOUNT; iPointLightIndex++)
	{
		// Calculate lighting
		lighting += CalculateLighting(iPointLightIndex, normal, position, diffuseAlbedo, specularAlbedo, specularPower);
	}

	// Add ambient component
	lighting += light[0].ambient * diffuseAlbedo * 0.3f;

	// Final color
	return saturate(float4(lighting, 1.0f));
}

// ----------------------------------------------------------------------------