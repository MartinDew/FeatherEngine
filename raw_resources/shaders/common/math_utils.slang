// Math utility functions for PBR rendering

import vex_common;

// Convert degrees to radians
float degToRad(float degrees) {
    return degrees * (PI / 180.0);
}

// Convert radians to degrees
float radToDeg(float radians) {
    return radians * (180.0 / PI);
}

// Safe normalize - returns zero vector if length is too small
float3 safeNormalize(float3 v) {
    float len = length(v);
    return (len > EPSILON) ? (v / len) : float3(0.0, 0.0, 0.0);
}

// Compute luminance from RGB color
float luminance(float3 rgb) {
    return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}

// Power function that's safe for negative bases
float safePow(float base, float exponent) {
    return pow(max(abs(base), EPSILON), exponent);
}

// Linear to sRGB conversion
float linearToSRGB(float linear) {
    if (linear <= 0.0031308) {
        return 12.92 * linear;
    }
    return 1.055 * pow(linear, 1.0 / 2.4) - 0.055;
}

float3 linearToSRGB(float3 linear) {
    return float3(
        linearToSRGB(linear.x),
        linearToSRGB(linear.y),
        linearToSRGB(linear.z)
    );
}

// sRGB to linear conversion
float sRGBToLinear(float srgb) {
    if (srgb <= 0.04045) {
        return srgb / 12.92;
    }
    return pow((srgb + 0.055) / 1.055, 2.4);
}

float3 sRGBToLinear(float3 srgb) {
    return float3(
        sRGBToLinear(srgb.x),
        sRGBToLinear(srgb.y),
        sRGBToLinear(srgb.z)
    );
}
