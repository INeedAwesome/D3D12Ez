#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

Correction correction : register(b1);

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
	// Rules of transformation: Model -> View -> Projection 
	// Model: Apply a transformation on the object.
	// View: The transformation of a camera for example.
	// Projection: Aspect ratio, and fov, stuff like that.
	
    float2 px;
	
	// Model 
    px.x = (pos.x * correction.CosAngle) - (pos.y * correction.SinAngle);
    px.y = (pos.x * correction.SinAngle) + (pos.y * correction.CosAngle);
	
    px *= correction.Zoom;				// View
    px.x *= correction.AspectRatio;		// Projection
	
    o_pos = float4(px, 0.0f, 1.0f);
	o_uv = uv;
}
