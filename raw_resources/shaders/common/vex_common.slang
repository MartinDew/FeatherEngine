// Vex common utilities for bindless rendering and shared types
import Vex;

// Bindless resource access (using Vex's bindless system)
Texture2D<float4> GetBindlessTexture2D(uint handle) {
    return GetBindlessResource<Texture2D<float4>>(handle);
}

// Samplers (defined by Vex)
SamplerState LinearSampler;
SamplerState PointSampler;

// Common constants
static const float PI = 3.14159265359;
static const float INV_PI = 0.31830988618;
static const float EPSILON = 1e-5;

// Utility functions
float saturate_custom(float x) {
    return clamp(x, 0.0, 1.0);
}

float3 saturate_custom(float3 x) {
    return clamp(x, float3(0.0), float3(1.0));
}
