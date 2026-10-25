
struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VertexOutput main(float4 pos : POSITION)
{
    VertexOutput output;

    // Rotate the cube around the Y axis.
    float angle = 0.65f;
    float cosine = cos(angle);
    float sine = sin(angle);

    float x = pos.x * cosine - pos.z * sine;
    float z = pos.x * sine + pos.z * cosine;

    // Apply a slight vertical and depth adjustment.
    output.position = float4(x, pos.y, z, 1.0f);

    // Give the cube a blue color.
    output.color = float3(0.15f, 0.55f, 1.0f);

    return output;
}
