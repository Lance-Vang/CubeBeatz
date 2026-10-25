
struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 localPosition : COLOR;
};

VertexOutput main(float4 pos : POSITION)
{
    VertexOutput output;

    // Rotate around the Y axis.
    float angleY = 0.65f;
    float cosineY = cos(angleY);
    float sineY = sin(angleY);

    float x = pos.x * cosineY - pos.z * sineY;
    float z = pos.x * sineY + pos.z * cosineY;

    // Rotate around the X axis.
    float angleX = -0.35f;
    float cosineX = cos(angleX);
    float sineX = sin(angleX);

    float y = pos.y * cosineX - z * sineX;
    z = pos.y * sineX + z * cosineX;

    // Move the cube in front of the camera.
    z += 3.0f;

    // Perspective projection.
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float focalLength = 1.5f;

    float projectedZ =
        (z * farPlane / (farPlane - nearPlane))
        - (nearPlane * farPlane / (farPlane - nearPlane));

    output.position = float4(
        x * focalLength,
        y * focalLength,
        projectedZ,
        z
    );

    // Pass the original vertex position to the pixel shader.
    output.localPosition = pos.xyz;

    return output;
}


