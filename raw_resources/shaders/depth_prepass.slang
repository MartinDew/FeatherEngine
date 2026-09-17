module depth_prepass;

import Vex;
import feather;

struct VSOutput {
    float4 clipPos : SV_Position;
};

struct Uniforms
{
	uint camera_handle;
	uint vertex_handle;
};

[[vk::push_constant]]
Uniforms uniforms;

static let camera_desc = GetBindlessResource<ConstantBuffer<CameraData>>(uniforms.camera_handle);

[shader("vertex")]
VSOutput Vertex(
	uint vertexID : SV_VertexID
) {
	VSOutput output;

	StructuredBuffer<VertexData> vertices = GetBindlessResource<StructuredBuffer<VertexData>>(uniforms.vertex_handle);
	VertexData v = vertices[vertexID];

    let cam = *camera_desc;
	output.clipPos = mul(cam.viewProj, float4(v.position, 1.0));
    return output;
}

[shader("pixel")]
void Pixel() {
	discard;
}
