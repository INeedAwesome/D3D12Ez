#include "RootSignature.hlsl"

[RootSignature(ROOTSIG)]
void main(
	// Input 
	float2 pos : Position, 
	in float2 uv : TexCoord,
	
	// Output
	out float2 o_uv : TexCoord,
	out float4 o_pos : SV_Position
) 
{
	o_pos = float4(pos.xy, 0.0f, 1.0f);
	o_uv = uv;
}
