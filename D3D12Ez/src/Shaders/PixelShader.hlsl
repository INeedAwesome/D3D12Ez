#include "RootSignature.hlsl"

float3 color : register(b0);
Texture2D<float4> textures[] : register(t0);
sampler textureSampler : register(s0);

[RootSignature(ROOTSIG)]
void main(
	// Input
	in float2 uv : TexCoord,

	// Output
	out float4 pixel : SV_Target
)
{
	float4 texel = textures[0].Sample(textureSampler, uv);
	pixel = float4(texel.rgb, 1.0f);
	
}