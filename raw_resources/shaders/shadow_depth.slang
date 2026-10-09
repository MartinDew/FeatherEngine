// Shadow map generation shader (depth-only pass)
// Renders scene from light's perspective to create shadow maps

import Vex;

// Uniforms for shadow pass
struct ShadowUniforms {
    float4x4 lightViewProj;     // Light's view-projection matrix
    float4x4 model;             // Model matrix for this entity
};

// Matches Vertex struct in C++
struct MeshVertex {
    float3 position;
    float3 normal;
    float2 uv;
};

// Vertex shader output
struct VSOutput {
    float4 position : SV_POSITION;
};

// Vertex shader
[shader("vertex")]
VSOutput VSMain(
    uint vertex_id : SV_VertexID,
    uniform float4x4 mvp,  // Model-View-Projection from light's perspective
    uniform uint vertex_buffer_handle
) {
    VSOutput output;

    let vertices = GetBindlessResource<StructuredBuffer<MeshVertex>>(vertex_buffer_handle);
    let vertex = vertices[vertex_id];

    // Transform vertex position to light clip space
    output.position = mul(mvp, float4(vertex.position, 1.0));

    return output;
}

// Pixel shader: depth-only pass (no output needed, depth written automatically)
[shader("pixel")]
void PSMain() {
    // No pixel shader output needed for depth-only pass
    // Depth is automatically written to depth buffer
}
