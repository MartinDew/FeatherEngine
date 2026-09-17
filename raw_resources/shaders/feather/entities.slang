implementing feather;

// Per-vertex data, fetched by SV_VertexID from a bindless structured buffer
// (Vex no longer binds vertex buffers through the input assembler).
public struct VertexData {
    public float3 position;
    public float3 normal;
    public float2 uv;
};

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