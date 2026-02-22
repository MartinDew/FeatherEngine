// Shadow mapping utilities with PCF filtering
implementing pbr;

import Vex;
import feather;


// Sample shadow map with PCF (Percentage-Closer Filtering)
// Returns 0.0 = fully shadowed, 1.0 = fully lit
float SampleShadowMapPCF(
    Texture2D<float> shadowMap,
    SamplerState shadowSampler,
    float3 shadowCoord,
    float bias
) {
    // Shadow coordinate is in [0, 1] range for XY, and depth for Z
    // NOTE: Using reverse-Z, so closer depths are greater values

    // Apply bias
    float depthBiased = shadowCoord.z - bias;

    // PCF 3x3 sampling
    float shadow = 0.0;
    float2 texelSize;
    shadowMap.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0 / texelSize;

    // Sample 3x3 grid around the shadow coordinate
    [unroll]
    for (int x = -1; x <= 1; ++x) {
        [unroll]
        for (int y = -1; y <= 1; ++y) {
            float2 offset = float2(x, y) * texelSize;
            float2 sampleUV = shadowCoord.xy + offset;

            // Sample shadow map depth
            float shadowDepth = shadowMap.SampleLevel(shadowSampler, sampleUV, 0).r;

            // Reverse-Z comparison: if depthBiased <= shadowDepth, in shadow
            // (Greater depth = closer to camera in reverse-Z)
            shadow += (depthBiased <= shadowDepth) ? 1.0 : 0.0;
        }
    }

    // Average the 9 samples
    shadow /= 9.0;

    return shadow;
}

float ComputeShadowMapPCF(GPULight light, float3 worldPos, float3 N) {
	if (light.shadowMapHandle != GInvalidBindlessHandle)
	{
		let shadowMap = GetBindlessResource<Texture2D<float>>(light.shadowMapHandle);
		let coords = mul(light.viewProj, float4(worldPos + N * 0.1, 1));
		let projCoords = coords.xyz / coords.w;
		var projUV = projCoords.xyz * 0.5 + 0.5;
		if (!(projUV.x < 0.0 || projUV.x > 1.0 || projUV.y < 0.0 || projUV.y > 1.0))
		{
			projUV.y = 1.0 - projUV.y;

			float total = 0;

			constexpr int PCFKernelSize = 4;
			constexpr int PCFHalfKernelSize = (PCFKernelSize / 2);

			[unroll] for (var x = -PCFHalfKernelSize; x < -PCFHalfKernelSize + PCFKernelSize; x++)
			{
				[unroll] for (var y = -PCFHalfKernelSize; y < -PCFHalfKernelSize + PCFKernelSize; y++)
				{
					total += shadowMap.SampleCmpLevelZero(ShadowMapSampler, projUV.xy, projCoords.z - light.shadowBias, int2(x, y));
				}
			}

			return total / pow(PCFKernelSize, 2.0f);
		}
	}
	return 1.0;
}

// Simple shadow map sample without PCF (for comparison/debugging)
float SampleShadowMapSimple(
    Texture2D<float> shadowMap,
    float3 shadowCoord,
    float bias
) {
    float depthBiased = shadowCoord.z - bias;
    float shadowDepth = shadowMap.SampleLevel(PointSampler, shadowCoord.xy, 0).r;
    return (depthBiased <= shadowDepth) ? 1.0 : 0.0;
}

// Transform world position to shadow map space
// lightViewProj: light's view-projection matrix
float3 WorldToShadowCoord(float3 worldPos, float4x4 lightViewProj) {
    float4 shadowPos = mul(lightViewProj, float4(worldPos, 1.0));

    // Perspective divide
    shadowPos.xyz /= shadowPos.w;

    // Transform from [-1, 1] NDC to [0, 1] texture space
    // NOTE: Assumes standard NDC with Y-up
    shadowPos.xy = shadowPos.xy * 0.5 + 0.5;
    shadowPos.y = 1.0 - shadowPos.y;  // Flip Y for texture coordinates

    return shadowPos.xyz;
}
