implementing feather;

// Camera uniforms
public struct CameraData {
    public float4x4 viewProj;
    public float3 cameraPos;
    public float _padding;
};

// Per-entity uniforms
public struct InstanceData {
    public float4x4 model;
    public float4x4 normalMatrix;      // For transforming normals
};

public struct MaterialData {
    public float4 baseColorFactor;
    public float4 emissiveFactor;
    public float metallicFactor;
    public float roughnessFactor;
    public uint baseColorHandle;
    public uint metallicRoughnessHandle;
    public uint normalHandle;
    public uint emissiveHandle;
};