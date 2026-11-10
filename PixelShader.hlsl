
cbuffer ObjectSettings : register(b0)
{
    int rotateObject;
    float cubePositionZ;
    float cubePositionX;
    float padding;

    float cubeLength;
    float3 extraPadding;

    float4 lanePressed;
    float2 lanePressedLastTwo;
    float2 extraPadding2;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 localPosition : COLOR;
    float3 viewPosition : TEXCOORD0;
};

// Calculate Phong lighting for a 3D surface.
float3 CalculatePhongLighting(float3 position, float3 baseColor)
{
    // Calculate the surface normal using screen-space derivatives.
    float3 normal = normalize(cross(
        ddx(position),
        ddy(position)
    ));

    // Define the light source above the track.
    float3 lightPosition = float3(0.0f, 4.0f, 1.0f);

    // Calculate the direction toward the light.
    float3 lightDirection = normalize(lightPosition - position);

    // Calculate the direction toward the camera.
    float3 viewDirection = normalize(-position);

    // Calculate the diffuse lighting.
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Calculate the specular highlight.
    float3 reflectionDirection = reflect(-lightDirection, normal);

    float specular = pow(
        max(dot(viewDirection, reflectionDirection), 0.0f),
        32.0f
    );

    // Strong ambient lighting keeps the neon colors bright.
    float ambientStrength = 0.65f;
    
    // Diffuse lighting adds depth to the cube faces.
    float diffuseStrength = 0.85f;
    
    // Specular lighting creates cyan-white highlights.
    float specularStrength = 0.8f;
    
    float3 ambient = baseColor * ambientStrength;
    float3 diffuseColor = baseColor * diffuse * diffuseStrength;
    
    float3 specularColor = float3(0.65f, 0.90f, 1.0f) * specular * specularStrength;
    
    // Combine the lighting while preserving the neon appearance.
    return saturate(ambient + diffuseColor + specularColor);
}

float4 main(PixelInput input) : SV_TARGET
{

    // Draw the track and its lane markings.
    if (rotateObject == 0)
    {
        float x = input.localPosition.x;

        // Dark blue track surface.
        float4 trackColor = float4(0.04f, 0.12f, 0.30f, 1.0f);

        // Neon cyan outer borders.
        if (abs(x + 1.8f) < 0.035f ||
            abs(x - 1.8f) < 0.035f)
        {
            return float4(0.10f, 0.85f, 1.0f, 1.0f);
        }

        // Five internal lane dividers.
        if (abs(x + 1.2f) < 0.025f ||
            abs(x + 0.6f) < 0.025f ||
            abs(x) < 0.025f ||
            abs(x - 0.6f) < 0.025f ||
            abs(x - 1.2f) < 0.025f)
        {
            return float4(0.20f, 0.75f, 1.0f, 1.0f);
        }

        // Keep the yellow hit line visible.
        float z = input.localPosition.z;

        if (abs(z - 0.15f) < 0.035f)
        {
            return float4(1.0f, 0.85f, 0.20f, 1.0f);
        }

        // Determine which of the six lanes this pixel belongs to.
        int lane = (int)floor((x + 1.8f) / 0.6f);
        lane = clamp(lane, 0, 5);

        // Read the keyboard state for that lane.
        float pressed = 0.0f;

        if (lane == 0) pressed = lanePressed.x;
        else if (lane == 1) pressed = lanePressed.y;
        else if (lane == 2) pressed = lanePressed.z;
        else if (lane == 3) pressed = lanePressed.w;
        else if (lane == 4) pressed = lanePressedLastTwo.x;
        else pressed = lanePressedLastTwo.y;

        // Match each lane's color to its notes.
        float3 laneColor;

        if (lane == 0)
            laneColor = float3(0.05f, 0.75f, 1.0f);
        else if (lane == 1)
            laneColor = float3(0.10f, 0.30f, 1.0f);
        else if (lane == 2)
            laneColor = float3(0.65f, 0.20f, 1.0f);
        else if (lane == 3)
            laneColor = float3(1.0f, 0.15f, 0.55f);
        else if (lane == 4)
            laneColor = float3(1.0f, 0.45f, 0.05f);
        else
            laneColor = float3(0.15f, 1.0f, 0.30f);

        // Blend each lane with more of its corresponding note color.
        trackColor.rgb = lerp(trackColor.rgb, laneColor, 0.35f);

        // Brighten the entire lane while its key is held.
        if (pressed > 0.5f)
        {
            trackColor.rgb =
                saturate(trackColor.rgb + laneColor * 0.65f);
        }

        return trackColor;
    }

        // Render the keyboard labels in bright cyan-white.
    if (rotateObject == 2)
    {
        return float4(0.75f, 0.95f, 1.0f, 1.0f);
    }


    // Display missed notes in grey.
    if (extraPadding.x == 1.0f)
    {
        return float4(0.45f, 0.45f, 0.45f, 1.0f);
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

   // Determine which lane the note belongs to.
int lane = (int)round((cubePositionX + 1.5f) / 0.6f);

// Select the note's lane color.
float3 laneColor;

if (lane == 0)
    laneColor = float3(0.05f, 0.75f, 1.0f); // Cyan
else if (lane == 1)
    laneColor = float3(0.10f, 0.30f, 1.0f); // Blue
else if (lane == 2)
    laneColor = float3(0.65f, 0.20f, 1.0f); // Purple
else if (lane == 3)
    laneColor = float3(1.0f, 0.15f, 0.55f); // Pink
else if (lane == 4)
    laneColor = float3(1.0f, 0.45f, 0.05f); // Orange
else
    laneColor = float3(0.15f, 1.0f, 0.30f); // Green

// Apply Phong lighting to the note cube.
float3 litColor = CalculatePhongLighting(
    input.viewPosition,
    laneColor
);

return float4(litColor, 1.0f);
}