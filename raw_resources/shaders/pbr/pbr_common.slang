// PBR BRDF functions - Cook-Torrance microfacet model

import common.vex_common;
import common.math_utils;

// GGX/Trowbridge-Reitz normal distribution function
float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, EPSILON);
}

// Schlick-GGX geometry function (single direction)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, EPSILON);
}

// Smith's method for geometry obstruction (combines view and light directions)
float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(saturate_custom(1.0 - cosTheta), 5.0);
}

// Fresnel-Schlick with roughness (for IBL)
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
    float3 one = float3(1.0);
    return F0 + (max(one * (1.0 - roughness), F0) - F0) * pow(saturate_custom(1.0 - cosTheta), 5.0);
}

// Calculate F0 (surface reflection at zero incidence) from base color and metallic
float3 CalculateF0(float3 baseColor, float metallic) {
    // Dielectric F0 is approximately 0.04
    float3 F0 = float3(0.04);
    // Metals use base color as F0
    return lerp(F0, baseColor, metallic);
}

// Cook-Torrance BRDF
float3 CookTorranceBRDF(
    float3 N,           // Normal
    float3 V,           // View direction
    float3 L,           // Light direction
    float3 baseColor,
    float metallic,
    float roughness
) {
    float3 H = normalize(V + L);

    // Calculate F0
    float3 F0 = CalculateF0(baseColor, metallic);

    // Cook-Torrance BRDF components
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Calculate specular component
    float3 numerator = NDF * G * F;
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float denominator = 4.0 * NdotV * NdotL;
    float3 specular = numerator / max(denominator, EPSILON);

    // Energy conservation: kS + kD = 1
    float3 kS = F;  // Fresnel is the specular component
    float3 kD = float3(1.0) - kS;
    kD *= (1.0 - metallic);  // Metals don't have diffuse

    // Lambert diffuse
    float3 diffuse = kD * baseColor * INV_PI;

    // Combine diffuse and specular
    return (diffuse + specular) * NdotL;
}
