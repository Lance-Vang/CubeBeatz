
cbuffer ObjectSettings : register(b0)
{
    int rotateObject;
    float cubePositionZ;
    float cubePositionX;
    float padding;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 localPosition : COLOR;
};

float4 main(PixelInput input) : SV_TARGET
{
    // Draw the track and its lane markings.
    if (rotateObject == 0)
    {
        float x = input.localPosition.x;

        // Dark blue track surface.
        float4 trackColor = float4(0.04f, 0.12f, 0.30f, 1.0f);

        // Five internal lane dividers.
        if (abs(x + 1.2f) < 0.025f ||
            abs(x + 0.6f) < 0.025f ||
            abs(x) < 0.025f ||
            abs(x - 0.6f) < 0.025f ||
            abs(x - 1.2f) < 0.025f)
        {
            return float4(0.20f, 0.75f, 1.0f, 1.0f);
        }

        // Yellow hit line.
        float z = input.localPosition.z;

        if (abs(z - 0.15f) < 0.035f)
        {
            return float4(1.0f, 0.85f, 0.20f, 1.0f);
        }

        return trackColor;
    }

    // Outline pass: choose a color based on timing.
    if (padding == 1.0f)
    {
        // Perfect timing: golden outline.
        if (cubePositionZ >= 0.03f && cubePositionZ <= 0.27f)
        {
            return float4(1.0f, 0.65f, 0.08f, 1.0f);
        }

        // Good timing: green outline.
        if (cubePositionZ >= -0.15f && cubePositionZ <= 0.45f)
        {
            return float4(0.10f, 1.0f, 0.30f, 1.0f);
        }
    }

    // Original blue cube faces.
    float3 p = abs(input.localPosition);

    if (p.y > p.x && p.y > p.z)
    {
        return float4(0.35f, 0.70f, 1.0f, 1.0f);
    }
    else if (p.x > p.z)
    {
        return float4(0.08f, 0.30f, 0.75f, 1.0f);
    }

    return float4(0.15f, 0.55f, 1.0f, 1.0f);
}
