// Shadow map generation shader (depth-only pass)
// Renders scene from light's perspective to create shadow maps

import Vex;
import feather;

// Vertex shader output
struct VSOutput {
    float4 position : SV_POSITION;
};

// Vertex shader
[shader("vertex")]
VSOutput VSMain(
    uint vertexID : SV_VertexID,
    uniform float4x4 mvp,      // Model-View-Projection from light's perspective
    uniform uint vertexHandle  // Bindless handle for this mesh's vertex buffer
) {
    VSOutput output;

    StructuredBuffer<VertexData> vertices = GetBindlessResource<StructuredBuffer<VertexData>>(vertexHandle);
    VertexData v = vertices[vertexID];

    // Transform vertex position to light clip space
    output.position = mul(mvp, float4(v.position, 1.0));

    return output;
}

// Pixel shader: depth-only pass (no output needed, depth written automatically)
[shader("pixel")]
void PSMain() {
    // No pixel shader output needed for depth-only pass
    // Depth is automatically written to depth buffer
}
