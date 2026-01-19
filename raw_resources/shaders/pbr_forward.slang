// PBR Forward Rendering Shader
// Implements physically-based rendering with multiple light types and shadow mapping
module pbr;

import Vex;
import common.vex_common;
import common.math_utils;
import common.lighting_common;
__include pbr.pbr_common;
__include pbr.pbr_lighting;
// __include pbr.shadow_mapping;

// Camera uniforms
struct CameraUniforms {
    float4x4 viewProj;
    float3 cameraPos;
    float _padding;
};

// Per-entity uniforms
struct EntityUniforms {
    float4x4 model;
    float4x4 normalMatrix;      // For transforming normals
    float4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float3 emissiveFactor;
    uint baseColorHandle;
    uint metallicRoughnessHandle;
    uint normalHandle;
    uint emissiveHandle;
};

struct Uniforms {
uniform uint camera_handle;
uniform uint entity_handle;
uniform uint lights_handle;
}

[vk::push_constant] uniform Uniforms uniforms;

// Vertex input (matches Vertex struct in C++)
struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
};

// Vertex shader output
struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    float3 worldNormal : NORMAL;
};

static let entity = *GetBindlessResource<ConstantBuffer<EntityUniforms>>(uniforms.entity_handle);
static let camera = *GetBindlessResource<ConstantBuffer<CameraUniforms>>(uniforms.camera_handle);

// Vertex shader
[shader("vertex")]
VSOutput VSMain(
    VSInput input
) {
    VSOutput output;

    // Transform position to world space
    float4 worldPos = mul(float4(input.position, 1.0), entity->model);
    output.worldPos = worldPos.xyz;

    // Transform position to clip space
    output.position = mul(worldPos, camera->viewProj);

    // Transform normal to world space
    // Use normalMatrix for non-uniform scaling support
    output.worldNormal = normalize(mul(float4(input.normal, 0.0), entity->normalMatrix).xyz);

    return output;
}

// Pixel shader
[shader("pixel")]
float4 PSMain(
    VSOutput input,
) : SV_Target {
    // Normalize interpolated normal
    float3 N = normalize(input.worldNormal);

    // View direction
    float3 V = normalize(camera->cameraPos - input.worldPos);

    // Sample material textures (if available)
    // For now, use procedural UVs based on world position (since we don't have UVs in vertex data)
    // TODO: Implement proper UV mapping via texture info or other mechanism
    float2 uv = input.worldPos.xz * 0.1;  // Simple planar mapping for testing

    // Base color
    float3 baseColor = entity->baseColorFactor.rgb;
    if (entity->baseColorHandle != 0) {
        Texture2D<float4> baseColorTex = GetBindlessTexture2D(entity->baseColorHandle);
        float4 texColor = baseColorTex.SampleLevel(LinearSampler, uv, 0);
        baseColor *= texColor.rgb;
    }

    // Metallic and roughness
    float metallic = entity->metallicFactor;
    float roughness = entity->roughnessFactor;
    if (entity->metallicRoughnessHandle != 0) {
        Texture2D<float4> mrTex = GetBindlessTexture2D(entity->metallicRoughnessHandle);
        float4 mr = mrTex.SampleLevel(LinearSampler, uv, 0);
        roughness *= mr.g;  // Green channel = roughness
        metallic *= mr.b;   // Blue channel = metallic
    }

    // Normal mapping (TODO: requires tangent space)
    // For now, use vertex normal
    float3 normal = N;
    // if (entity->normalHandle != 0) {
    //     // Sample normal map and transform to world space (requires TBN matrix)
    // }

    // Emissive
    float3 emissive = entity->emissiveFactor;
    if (entity->emissiveHandle != 0) {
        Texture2D<float4> emissiveTex = GetBindlessTexture2D(entity->emissiveHandle);
        float4 emissiveColor = emissiveTex.SampleLevel(LinearSampler, uv, 0);
        emissive *= emissiveColor.rgb;
    }

    // Evaluate all lights
    /*float3 litColor = EvaluateAllLights(
        lights,
        lightCount,
        input.worldPos,
        normal,
        V,
        baseColor,
        metallic,
        roughness
    );
*/
	float3 litColor = float3(0.0,0.0,0.0);
    // Add ambient lighting (simple constant for now)
    // TODO: Use environment lighting from RenderCapture
    float3 ambient = baseColor * 0.03;
    litColor += ambient;

    // Add emissive
    litColor += emissive;

    // Output HDR color (tonemapping will be applied later if needed)
    return float4(1.0 ,1.0 ,1.0 , 1.0);
}
