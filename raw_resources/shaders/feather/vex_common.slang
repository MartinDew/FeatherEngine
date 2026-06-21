implementing feather;
// Vex common utilities for bindless rendering and shared types

import Vex;

// Bindless resource access (using Vex's bindless system)
public Texture2D<float4> GetBindlessTexture2D(uint handle) { return GetBindlessResource<Texture2D<float4>>(handle); }

// Samplers - Vex places static samplers in space1 (DX12) / set 1 (Vulkan)
[vk::binding(0, 1)] public SamplerState LinearSampler : register(s0, space1);
[vk::binding(1, 1)] public SamplerState PointSampler : register(s1, space1);
[vk::binding(2, 1)] public SamplerComparisonState ShadowMapSampler : register(s2, space1);

// Common constants
public static const float PI = 3.14159265359;
public static const float INV_PI = 0.31830988618;
public static const float EPSILON = 1e-5;

// Utility functions
public float saturate_custom(float x) { return clamp(x, 0.0, 1.0); }

public float3 saturate_custom(float3 x) { return clamp(x, float3(0.0), float3(1.0)); }

public static const uint GInvalidBindlessHandle = ~0U;
