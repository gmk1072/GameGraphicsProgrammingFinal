Texture2D tex0	: register(t0);
Texture2D tex1	: register(t1);
Texture2D tex2	: register(t2);
Texture2D tex3	: register(t3);
Texture2D tex4	: register(t4);
Texture2D tex5	: register(t5);
Texture2D tex6	: register(t6);
Texture2D tex7	: register(t7);
SamplerState texSampler	: register(s0);

struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

//add all downsampled textures together to form a full screen image
float4 main(TargetCoords input) : SV_TARGET
{
	float4 col0 = tex0.Sample(texSampler, input.uv);
	float4 col1 = tex1.Sample(texSampler, input.uv);
	float4 col2 = tex2.Sample(texSampler, input.uv);
	float4 col3 = tex3.Sample(texSampler, input.uv);
	float4 col4 = tex4.Sample(texSampler, input.uv);
	float4 col5 = tex5.Sample(texSampler, input.uv);
	float4 col6 = tex6.Sample(texSampler, input.uv);
	float4 col7 = tex7.Sample(texSampler, input.uv);
	return col0 + col1 + col2 + col3 + col4 + col5 + col6 + col7;
}