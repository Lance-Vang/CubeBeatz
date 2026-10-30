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
        float z = input.localPosition.z;

        // Start with a dark blue track surface.
        float4 trackColor = float4(0.04f, 0.12f, 0.30f, 1.0f);

        // Find the distance to the three lane divider positions.
        float distanceToCenter = abs(x);
        float distanceToLeft = abs(x + 1.0f);
        float distanceToRight = abs(x - 1.0f);

        // Draw three bright blue lane divider lines.
        if (distanceToCenter < 0.025f ||
            distanceToLeft < 0.025f ||
            distanceToRight < 0.025f)
        {
            return float4(0.20f, 0.75f, 1.0f, 1.0f);
        }

        // Draw a horizontal hit line near the front of the track.
        if (abs(z - 0.15f) < 0.035f)
        {
            return float4(1.0f, 0.85f, 0.20f, 1.0f);
        }

        return trackColor;
    }

    // Differentiate the cube faces.
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


