module depth_prepass;

import Vex;
import feather;

struct MeshVertex {
    float3 position;
    float3 normal;
    float2 uv;
};

struct VSOutput {
    float4 clipPos : SV_Position;
};

struct Uniforms
{
	uint vertex_buffer_handle;
	uint camera_handle;
};

[[vk::push_constant]]
Uniforms uniforms;

static let camera_desc = GetBindlessResource<ConstantBuffer<CameraData>>(uniforms.camera_handle);

[shader("vertex")]
VSOutput Vertex(
	uint vertex_id : SV_VertexID
) {
	VSOutput output;

    let vertices = GetBindlessResource<StructuredBuffer<MeshVertex>>(uniforms.vertex_buffer_handle);
    let vertex = vertices[vertex_id];

    let cam = *camera_desc;
	output.clipPos = mul(cam.viewProj, float4(vertex.position, 1.0));
    return output;
}

[shader("pixel")]
void Pixel() {
	discard;
}