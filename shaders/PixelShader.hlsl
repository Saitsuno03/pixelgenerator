Texture2D textureMap : register(t0);
SamplerState samplerState : register(s0);

float4 colorPalette[4];

float4 PS(float2 tex : TEXCOORD) : SV_Target {
    float4 color = textureMap.Sample(samplerState, tex);

    // Use colorPalette to change pixel color based on some logic (you can refine this)
    if (color.r > 0.5f) {
        color = colorPalette[0]; // Blue
    } else {
        color = colorPalette[1]; // Black
    }

    return color;
}
