
struct PixelInput
{
    float4 position : SV_POSITION;
    float3 localPosition : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
    // Differentiate the faces by their dominant local axis.
    float3 p = abs(input.localPosition);

    if (p.y > p.x && p.y > p.z)
    {
        // Top and bottom faces: lighter blue.
        return float4(0.35f, 0.70f, 1.0f, 1.0f);
    }
    else if (p.x > p.z)
    {
        // Left and right faces: darker blue.
        return float4(0.08f, 0.30f, 0.75f, 1.0f);
    }

    // Front and back faces: standard blue.
    return float4(0.15f, 0.55f, 1.0f, 1.0f);
}

