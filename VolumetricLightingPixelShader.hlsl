cbuffer data : register(b0) {
	float2 ScreenLightPos;//screen space position of the light
	float Exposure;//overall intensity of the post process
	float Decay;//0-1 dissipates each sample's contribution as the ray progresses away from the light source
	float Density;//control over the separation between samples for cases in which we wish to reduce the overall number of sample iterations
	float Weight;//intensity of each sample
	int NumSamples;
}

struct TargetCoords
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD;
};

Texture2D<uint4> volumetricTexture : register(t0);
SamplerState volumetricSampler	: register(s0);

float4 main(TargetCoords input) : SV_TARGET
{
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);

	//half = half precision float, apparently better for optomization, just less precise. but i don't think i need that precision anyway
    half2 deltaTexCoord = input.position.xy - ScreenLightPos.xy;

	deltaTexCoord *= 1.0f / NumSamples * Density;

    half3 color = half3(1.0f, 1.0f, 1.0f) - half3(volumetricTexture.Load(int3(input.position.xy, 0)).ggg); // = tex2D(volumetricSampler, input.uv);

	half illuminationDecay = 1.0f;

	for (int i = 0; i < NumSamples; i++)
	{
		input.position.xy -= deltaTexCoord;

        half3 sample = half3(1.0f, 1.0f, 1.0f) - half3(volumetricTexture.Load(int3(input.position.xy, 0)).ggg);

		sample *= illuminationDecay * Weight;

		color += sample;

		illuminationDecay *= Decay;
	}

	return float4(color * Exposure, .1);
}

//float4 main(float2 texCoord : TEXCOORD0) : COLOR0
//{
//	// Calculate vector from pixel to light source in screen space.  
//	half2 deltaTexCoord = (texCoord - ScreenLightPos.xy);
//	// Divide by number of samples and scale by control factor.  
//	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
//	// Store initial sample.  
//	half3 color = tex2D(frameSampler, texCoord);
//	// Set up illumination decay factor.  
//	half illuminationDecay = 1.0f;
//	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
//	for (int i = 0; i < NUM_SAMPLES; i++)
//	{
//		// Step sample location along ray.  
//		texCoord -= deltaTexCoord;
//		// Retrieve sample at new location.  
//		half3 sample = tex2D(frameSampler, texCoord);
//		// Apply sample attenuation scale/decay factors.  
//		sample *= illuminationDecay * Weight;
//		// Accumulate combined color.  
//		color += sample;
//		// Update exponential decay factor.  
//		illuminationDecay *= Decay;
//	}
//	// Output final color with a further scale control factor.  
//	return float4(color * Exposure, 1);
//}