// Shadow mapping utilities with PCF filtering
implementing pbr;

import Vex;
import feather;

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
