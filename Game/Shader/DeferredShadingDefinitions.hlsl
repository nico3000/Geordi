struct ScreenQuadVertex
{
    float2 position : POSITION0;
};


struct ScreenQuadFragment
{
    float4 position : SV_Position;
    float2 tex : TEXCOORD0;
};


ScreenQuadFragment ScreenQuadVS(ScreenQuadVertex input)
{
    ScreenQuadFragment output = (ScreenQuadFragment)0;
    output.position = float4(input.position, 0.0, 1.0);
    output.tex = 0.5 + float2(+0.5, -0.5) * input.position;
    return output;
}