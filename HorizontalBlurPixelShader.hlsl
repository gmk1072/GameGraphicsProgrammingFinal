
#define MAX_BLUR_PIXELS 26

// Adapted from http://www.rastertek.com/dx11tut36.html
// Lighting information to sample from
Texture2D blurTexture		: register(t0);
SamplerState blurSampler	: register(s0);

//extra data
cbuffer data : register(b0) {
	float blurDistance;	//how far from pixel to blur
	float texelSize; //dist to next pixel -->calculated in code b/c only need to calculate once or when resized, not every pixel/draw/etc.
}

static const float weights[MAX_BLUR_PIXELS] = { 0.040312,0.040111,0.039514,0.038539,0.037215,0.035579,0.033676,0.031559,0.02928,0.026896,0.024461,0.022024,0.019634,0.017328,0.015142,0.0131,0.01122,0.009515,0.007988,0.00664,0.005465,0.004453,0.003592,0.002869,0.002268,0.001776};

//Input from vertex shader
struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};


float4 main(TargetCoords input) : SV_TARGET
{
	//add the weighted colors together
	float4 color = float4(0, 0, 0, 0);
	for (int k = -blurDistance; k <= blurDistance; k++) {
		color += blurTexture.Sample(blurSampler, input.uv + float2(texelSize * k, 0.0f)) * weights[clamp(abs(k), 0, MAX_BLUR_PIXELS - 1)];	//how to use diff weights -- need to calc weights depending on blurDistance
	}

	//set alpha to 1
	color.a = 1.0f;
	//second shader should blur vertically
	return color;
}