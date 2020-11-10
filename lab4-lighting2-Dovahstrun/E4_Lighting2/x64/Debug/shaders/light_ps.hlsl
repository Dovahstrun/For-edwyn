// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambient;
	float4 diffuse;
	float3 position;
	float padding;
};

cbuffer AttBuffer : register(b1)
{
    float constFactor;
    float linFactor;
    float quadFactor;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 ldiffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(ldiffuse * intensity);
	return colour;
}

float4 calculateAttentuation(float distance)
{
    float attenuation = 1 / (constFactor + (linFactor * distance) + (quadFactor * pow(distance, 2)));
	
    return attenuation;
}

float4 main(InputType input) : SV_TARGET
{
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    float distance = length(position - input.worldPosition);
	float4 textureColour = texture0.Sample(sampler0, input.tex);
	float3 lightVector = normalize(position - input.worldPosition);
    float4 lightColour = ambient + (calculateLighting(lightVector, input.normal, diffuse) * calculateAttentuation(distance));
	
	return lightColour * textureColour;
}



