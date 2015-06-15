// ----------------------------------------------------------------------------
// Structure definition
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 WorldPos : POSITION;
};

// ----------------------------------------------------------------------------

struct Light
{
	float3 dir;
	float3 pos;
	float range;
	float3 att;
	float4 ambient;
	float4 diffuse;
};

// ----------------------------------------------------------------------------
// Constant buffers
cbuffer cbPerFrame
{
	Light light;
};

// ----------------------------------------------------------------------------
Texture2D ObjTexture;
SamplerState ObjSamplerState;

// ----------------------------------------------------------------------------

// Main
float4 main(PS_INPUT input) : SV_TARGET
{
	// Normalize vertex normal
	input.Normal = normalize(input.Normal);
	// Sample the pixel color from the texture
	float4 fragmentColor = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
	// Final color
	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//// Directional light
	//// Add the ambient component
	//finalColor += fragmentColor * light.ambient;
	//// Add the diffuse component
	//finalColor += saturate(dot(light.dir, input.Normal) * light.diffuse * fragmentColor);

	//// Set the alpha equal to the alpha of the texture
	//finalColor.w = fragmentColor.a;

	// Point light
	float3 lightToPixelVec = light.pos - input.WorldPos.xyz;
	float distance = length(lightToPixelVec);
	// Ambient light
	float4 ambientComponent = fragmentColor * light.ambient;
	// If the pixel is outside the light range use only the ambient light
	if (distance > light.range)
	{
		ambientComponent.w = fragmentColor.w;
		return ambientComponent;
	}
	// Normalize lightToPixelVec
	lightToPixelVec = normalize(lightToPixelVec);
	// Diffuse light
	float diffuseComponent = dot(lightToPixelVec, input.Normal);
	// Check which side of the pixel the light has hit
	if (diffuseComponent > 0.0f)
	{
		// Add the diffuse component
		finalColor += diffuseComponent * fragmentColor * light.diffuse;

		// Add the light attenuation
		finalColor /= light.att[0] + (light.att[1] * distance) + (light.att[2] * distance * distance);
	}

	finalColor = saturate(finalColor + ambientComponent);
	finalColor.w = fragmentColor.a;

	return finalColor;
}

// ----------------------------------------------------------------------------