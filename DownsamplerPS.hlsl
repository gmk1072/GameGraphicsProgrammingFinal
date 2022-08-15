Texture2D tex	: register(t0);
SamplerState texSampler	: register(s0);

cbuffer Data : register(b0)
{
	float texelHeight;
	float texelWidth;
	float sizeMod;
};


struct TargetCoords
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};


float4 main(TargetCoords input) : SV_TARGET
{
	//SizedOutput output;

	float4 col = float4(0, 0, 0, 0);
	for (int i = 0; i < sizeMod; i++) {
		for (int j = 0; j < sizeMod; j++) {
			col += tex.Sample(texSampler, (input.uv * sizeMod) + float2(texelWidth*-i, texelHeight*-j));
		}
	}
	col = col / pow(sizeMod, 2);
	col.a = 1.0f;

	return col;
}