// PBR lighting evaluation for different light types
implementing pbr;

import common.vex_common;
import common.math_utils;
import common.lighting_common;

__include shadow_mapping;
__include pbr_common;


// Evaluate directional light contribution
float3 EvaluateDirectionalLight(
    GPULight light,
    float3 worldPos,
    float3 N,
    float3 V,
    float3 baseColor,
    float metallic,
    float roughness,
    float shadow
) {
    // Light direction for directional lights is constant
    float3 L = normalize(-light.direction);

    // Calculate BRDF
    float3 brdf = CookTorranceBRDF(N, V, L, baseColor, metallic, roughness);

    // Light color and intensity
    float3 radiance = light.color.rgb * light.color.a;

    // Final contribution
    return brdf * radiance * shadow;
}

// Evaluate point light contribution
float3 EvaluatePointLight(
    GPULight light,
    float3 worldPos,
    float3 N,
    float3 V,
    float3 baseColor,
    float metallic,
    float roughness,
    float shadow
) {
    // Calculate light direction and distance
    float3 L = light.position - worldPos;
    float distance = length(L);
    L = L / distance;  // Normalize

    // Check if within range
    if (distance > light.range) {
        return float3(0.0);
    }

    // Calculate BRDF
    float3 brdf = CookTorranceBRDF(N, V, L, baseColor, metallic, roughness);

    // Light color and intensity with attenuation
    float attenuation = 1.0 / (distance * distance);  // Inverse square falloff
    float rangeAttenuation = saturate_custom(1.0 - (distance / light.range));
    rangeAttenuation *= rangeAttenuation;  // Smooth falloff at range limit

    float3 radiance = light.color.rgb * light.color.a * attenuation * rangeAttenuation;

    // Final contribution
    return brdf * radiance * shadow;
}

// Evaluate spot light contribution
float3 EvaluateSpotLight(
    GPULight light,
    float3 worldPos,
    float3 N,
    float3 V,
    float3 baseColor,
    float metallic,
    float roughness,
    float shadow
) {
    // Calculate light direction and distance
    float3 L = light.position - worldPos;
    float distance = length(L);
    L = L / distance;  // Normalize

    // Check if within range
    if (distance > light.range) {
        return float3(0.0);
    }

    // Spot light cone attenuation
    float theta = dot(L, normalize(-light.direction));
    float spotAttenuation = saturate_custom((theta - light.spotAngleCos) / (1.0 - light.spotAngleCos));
    spotAttenuation *= spotAttenuation;  // Smooth falloff

    if (spotAttenuation <= 0.0) {
        return float3(0.0);  // Outside spot cone
    }

    // Calculate BRDF
    float3 brdf = CookTorranceBRDF(N, V, L, baseColor, metallic, roughness);

    // Light color and intensity with attenuation
    float attenuation = 1.0 / (distance * distance);  // Inverse square falloff
    float rangeAttenuation = saturate_custom(1.0 - (distance / light.range));
    rangeAttenuation *= rangeAttenuation;

    float3 radiance = light.color.rgb * light.color.a * attenuation * rangeAttenuation * spotAttenuation;

    // Final contribution
    return brdf * radiance * shadow;
}

// Evaluate all lights for a fragment
float3 EvaluateAllLights(
    StructuredBuffer<GPULight> lights,
    uint lightCount,
    float3 worldPos,
    float3 N,
    float3 V,
    float3 baseColor,
    float metallic,
    float roughness
) {
    float3 totalLight = float3(0.0);

    for (uint i = 0; i < lightCount; ++i) {
        GPULight light = lights[i];

        // Calculate shadow (if light has shadow map)
        float shadow = 1.0;  // Default: no shadow
        // TODO: Implement shadow map sampling when shadow maps are passed
        // if (light.shadowMapIndex != -1) {
        //     shadow = SampleShadowMap(...);
        // }

        // Evaluate light based on type
        if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            totalLight += EvaluateDirectionalLight(light, worldPos, N, V, baseColor, metallic, roughness, shadow);
        }
        else if (light.type == LIGHT_TYPE_POINT) {
            totalLight += EvaluatePointLight(light, worldPos, N, V, baseColor, metallic, roughness, shadow);
        }
        else if (light.type == LIGHT_TYPE_SPOT) {
            totalLight += EvaluateSpotLight(light, worldPos, N, V, baseColor, metallic, roughness, shadow);
        }
    }

    return totalLight;
}
