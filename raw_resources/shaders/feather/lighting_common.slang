implementing feather;
// Common lighting structures and definitions

// Light types
public static const uint LIGHT_TYPE_DIRECTIONAL = 0;
public static const uint LIGHT_TYPE_POINT = 1;
public static const uint LIGHT_TYPE_SPOT = 2;

// GPU Light structure (must match C++ GPULight in vex_renderer.cpp)
public struct GPULight {
    public uint type;                  // Light type (directional/point/spot)
    public float3 position;            // World position
    public float3 direction;           // Light direction (for directional and spot)
    public float4 color;               // RGB + intensity in alpha
    public float range;                // Range for point/spot lights
    public float spotAngleCos;         // cos(spot_angle) for fast comparison
    public uint shadowMapHandle;        // Shadow map bindless handle (or -1 if no shadows)
    public float shadowBias;           // Shadow bias to prevent acne
    public float4x4 viewProj;          // Light View Proj
};

// Material properties for PBR
public struct MaterialProperties {
    public float3 baseColor;
    public float metallic;
    public float roughness;
    public float3 emissive;
    public float3 normal;      // World-space normal
    public float ao;           // Ambient occlusion (1.0 = no occlusion)
};

// View-dependent properties
public struct ViewProperties {
    public float3 position;    // Camera world position
    public float3 direction;   // View direction (normalized)
    public float NdotV;        // dot(normal, view)
};

struct ShadowVP
{

}