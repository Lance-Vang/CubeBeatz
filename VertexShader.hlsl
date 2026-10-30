//Constant buffer
cbuffer ObjectSettings : register(b0)
{
    int rotateObject;
    float cubePositionZ;
    float cubePositionX;
    float padding;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 localPosition : COLOR;
};

VertexOutput main(float4 pos : POSITION)
{
    VertexOutput output;

    float x = pos.x;
    float y = pos.y;
    float z = pos.z;
    
    // Move the cube horizontally into the selected lane
    if (rotateObject == 1)
    {
        x += cubePositionX;
    }

    // Rotate the cube around the Y axis.
    // The track skips this rotation.
    if (rotateObject == 1)
    {
        float angleY = 0.65f;

        float cosineY = cos(angleY);
        float sineY = sin(angleY);

        float rotatedX = x * cosineY - z * sineY;
        float rotatedZ = x * sineY + z * cosineY;

        x = rotatedX;
        z = rotatedZ;

        // Move the cube along the track.
        z += cubePositionZ;
    }

    // Apply the shared camera angle.
    float angleX = -0.35f;

    float cosineX = cos(angleX);
    float sineX = sin(angleX);

    float rotatedY = y * cosineX - z * sineX;
    float rotatedZ = y * sineX + z * cosineX;

    y = rotatedY;
    z = rotatedZ;

    // Move the objects in front of the camera.
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

    output.localPosition = pos.xyz;

    return output;
}


