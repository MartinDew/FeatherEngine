module depth_prepass;

import Vex;
import feather;

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput {
    float4 clipPos : SV_Position;
};

struct Uniforms
{
	uint camera_handle;
};

[[vk::push_constant]]
Uniforms uniforms;

static let camera_desc = GetBindlessResource<ConstantBuffer<CameraData>>(uniforms.camera_handle);

[shader("vertex")]
VSOutput Vertex(
	VSInput input
) {
	VSOutput output;

    let cam = *camera_desc;
	output.clipPos = mul(cam.viewProj, float4(input.position, 1.0));
    return output;
}

[shader("pixel")]
void Pixel() {
	discard;
}