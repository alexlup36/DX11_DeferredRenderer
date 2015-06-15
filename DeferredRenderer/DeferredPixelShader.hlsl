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
	Light light;
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
float3 CalculateLighting(in float3 normal,
	in float3 position,
	in float3 diffuseAlbedo,
	in float3 specularAlbedo,
	in float specularPower)
{
	// Light vector
	float3 LightVector = 0;
	float Attenuation = 1.0f;

	// Calculate the light vector
	LightVector = light.pos - position;

	// Calculate the attenuation based on the distance to the light source
	float dist = length(LightVector);
	Attenuation = max(0.0f, 1.0f - (dist / light.range));

	// Normalize light vector
	LightVector /= dist;

	// Calculate the ambient component
	float3 ambientComponent = light.ambient * diffuseAlbedo;

	// Calculate the diffuse component
	float normalDotLight = saturate(dot(normal, LightVector));
	float3 diffuseComponent = light.diffuse * diffuseAlbedo * normalDotLight;

	// Calculate the specular component
	float3 SurfaceToCamera = CameraPosition - position;
	float3 HalfVector = normalize(LightVector + SurfaceToCamera);
	float3 specularComponent = specularAlbedo * light.specular * normalDotLight * pow(saturate(dot(normal, HalfVector)), specularPower);

	/*float3 ViewVector = normalize(CameraPosition - position);
	float3 ReflectionVector = normalize(reflect(LightVector, normal));
	float viewDotReflect = dot(ViewVector, ReflectionVector);

	float3 specularComponent = float3(0.0f, 0.0f, 0.0f);
	if (normalDotLight > 0)
	{
		specularComponent = pow(max(viewDotReflect, 0.0f), specularPower) * specularAlbedo * light.specular;
	}*/

	// Calculate the final color
	return saturate(ambientComponent + (diffuseComponent + specularComponent) * Attenuation);
	
	//return specularComponent;
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

	// Sample the GBuffer properties
	GetGBufferAttributes(input.TexCoord, normal, position, diffuseAlbedo, specularAlbedo, specularPower);

	// Calculate lighting
	float3 lighting = CalculateLighting(normal, position, diffuseAlbedo, specularAlbedo, specularPower);

	return float4(lighting, 1.0f);
}

// ----------------------------------------------------------------------------