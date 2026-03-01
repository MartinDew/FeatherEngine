// PBR Forward Rendering Shader
// Implements physically-based rendering with multiple light types and shadow mapping
module pbr;

import feather;
import Vex;
__include pbr.pbr_common;
__include pbr.pbr_lighting;
// __include pbr.shadow_mapping;

struct Uniforms {
uint camera_handle;
uint instance_handle;
uint material_handle;
uint lights_handle;
uint num_lights;
}

[vk::push_constant] uniform Uniforms uniforms;

// Vertex input (matches Vertex struct in C++)
struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;  // Optional UVs for texturing (not used in this example)
};

// Vertex shader output
struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    float3 worldNormal : NORMAL;
};

static let entity_desc = GetBindlessResource<ConstantBuffer<InstanceData>>(uniforms.instance_handle);
static let camera_desc = GetBindlessResource<ConstantBuffer<CameraData>>(uniforms.camera_handle);

// Vertex shader
[shader("vertex")]
VSOutput VSMain(
    VSInput input
) {
    VSOutput output;

    let entity = *entity_desc;
    let camera = *camera_desc;
    // Transform position to world space
    float4 worldPos = mul(entity->model, float4(input.position, 1.0));
    output.worldPos = worldPos.xyz;

    // Transform position to clip space
    output.position = mul(camera->viewProj, worldPos);

    // Transform normal to world space
    // Use normalMatrix for non-uniform scaling support
    output.worldNormal = normalize(mul(entity->normalMatrix, float4(input.normal, 0.0)).xyz);

    return output;
}

// Pixel shader
[shader("pixel")]
float4 PSMain(
    VSOutput input
) : SV_Target {
    let entity = *entity_desc;
    let camera = *camera_desc;

    let material = *GetBindlessResource<ConstantBuffer<MaterialData>>(uniforms.material_handle);

    // Normalize interpolated normal
    float3 N = normalize(input.worldNormal);

    // View direction
    float3 V = normalize(camera->cameraPos - input.worldPos);

    // Sample material textures (if available)
    // For now, use procedural UVs based on world position (since we don't have UVs in vertex data)
    // TODO: Implement proper UV mapping via texture info or other mechanism
    float2 uv = input.worldPos.xz * 0.1;  // Simple planar mapping for testing

    // Base color
    float3 baseColor = material->baseColorFactor.rgb;
    if (material->baseColorHandle != 0) {
        Texture2D<float4> baseColorTex = GetBindlessTexture2D(material->baseColorHandle);
        float4 texColor = baseColorTex.SampleLevel(LinearSampler, uv, 0);
        baseColor *= texColor.rgb;
    }

    // Metallic and roughness
    float metallic = material->metallicFactor;
    float roughness = material->roughnessFactor;
    if (material->metallicRoughnessHandle != 0) {
        Texture2D<float4> mrTex = GetBindlessTexture2D(material->metallicRoughnessHandle);
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
    float4 emissive = material->emissiveFactor;
    if (material->emissiveHandle != 0) {
        Texture2D<float4> emissiveTex = GetBindlessTexture2D(material->emissiveHandle);
        float4 emissiveColor = emissiveTex.SampleLevel(LinearSampler, uv, 0);
        emissive *= emissiveColor;
    }


    static let lightsDesc = GetBindlessResource<StructuredBuffer<GPULight>>(uniforms.lights_handle);
    StructuredBuffer<GPULight> lights = *lightsDesc;

    // Evaluate all lights
    float3 litColor = EvaluateAllLights(
        lights,
        uniforms.num_lights,
        input.worldPos,
        normal,
        V,
        baseColor,
        metallic,
        roughness
    );

    // Add ambient lighting (simple constant for now)
    // TODO: Use environment lighting from RenderCapture
    float3 ambient = baseColor * 0.3;
    litColor += ambient;

    float4 result_color = float4(litColor.rgb, 0.0);
    // Add emissive
    result_color += emissive;

    // Output HDR color (tonemapping will be applied later if needed)
    return result_color;
}
