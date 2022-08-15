// Lighting information to sample from
Texture2D colorTexture          : register(t0);
Texture2D lightTexture          : register(t1);
Texture2D emissionTexture       : register(t2);
SamplerState deferredSampler    : register(s0);

cbuffer externalData : register(b0)
{
    //Bloom pixel threshold -- I see no reason to make it in a buffer of its own
    float ColorThreshold;

	//Glow for emission
    float GlowPercentage;
};

// Input info from vertex shader
struct TargetCoords
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// Output info for color with light and pixels to blur
struct DefferedOut
{
    float4 color : SV_Target0;
    float4 bloom : SV_Target1;
    float4 glow : SV_Target2;
};

DefferedOut main(TargetCoords input)
{
    DefferedOut output;

    // Sample color, world pos, normals and emissions
    float4 col = colorTexture.Sample(deferredSampler, input.uv);
    float4 light = lightTexture.Sample(deferredSampler, input.uv);
    float4 emission = emissionTexture.Sample(deferredSampler, input.uv);

    bool isEmission = any(emission.rgb);
    if (isEmission)
        output.color = emission;
    else
        output.color = col * light;

    output.bloom = output.color * ColorThreshold;
    output.glow = emission * isEmission * GlowPercentage;
    return output;
}