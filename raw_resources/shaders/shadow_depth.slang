// Shadow map generation shader (depth-only pass)
// Renders scene from light's perspective to create shadow maps

// Uniforms for shadow pass
struct ShadowUniforms {
    float4x4 lightViewProj;     // Light's view-projection matrix
    float4x4 model;             // Model matrix for this entity
};

// Vertex input
struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
};

// Vertex shader output
struct VSOutput {
    float4 position : SV_POSITION;
};

// Vertex shader
[shader("vertex")]
VSOutput VSMain(
    VSInput input,
    uniform float4x4 mvp  // Model-View-Projection from light's perspective
) {
    VSOutput output;

    // Transform vertex position to light clip space
    output.position = mul(float4(input.position, 1.0), mvp);

    return output;
}

// Pixel shader: depth-only pass (no output needed, depth written automatically)
[shader("pixel")]
void PSMain() {
    // No pixel shader output needed for depth-only pass
    // Depth is automatically written to depth buffer
}
