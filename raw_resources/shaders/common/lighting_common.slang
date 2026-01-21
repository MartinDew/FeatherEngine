// Common lighting structures and definitions

// Light types
static const uint LIGHT_TYPE_DIRECTIONAL = 0;
static const uint LIGHT_TYPE_POINT = 1;
static const uint LIGHT_TYPE_SPOT = 2;

// GPU Light structure (must match C++ GPULight in vex_renderer.cpp)
struct GPULight {
    uint type;                  // Light type (directional/point/spot)
    float3 position;            // World position
    float3 direction;           // Light direction (for directional and spot)
    float4 color;               // RGB + intensity in alpha
    float range;                // Range for point/spot lights
    float spotAngleCos;         // cos(spot_angle) for fast comparison
    uint shadowMapIndex;        // Shadow map bindless handle (or -1 if no shadows)
    float shadowBias;           // Shadow bias to prevent acne
};

// Material properties for PBR
struct MaterialProperties {
    float3 baseColor;
    float metallic;
    float roughness;
    float3 emissive;
    float3 normal;      // World-space normal
    float ao;           // Ambient occlusion (1.0 = no occlusion)
};

// View-dependent properties
struct ViewProperties {
    float3 position;    // Camera world position
    float3 direction;   // View direction (normalized)
    float NdotV;        // dot(normal, view)
};
