module;

// Data structures for GPU buffers and shader resources
#include <cstdint>

export module feather.core:rendering.render_data;

import :math.math_defs;

export namespace feather {

struct CameraBufferData {
	Matrix viewProj;
	Vector3 cameraPos;
	float _padding;
};

struct InstanceBufferData {
	Matrix model;
	Matrix normalMatrix;
};

template <class THandle>
struct PbrMaterialBufferDataTemplate {
	Color baseColorFactor;
	Color emissiveFactor;
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	THandle baseColorHandle;
	THandle metallicRoughnessHandle;
	THandle normalHandle;
	THandle emissiveHandle;
	// Eventually, there will be the next pass handle here
};

template <class THandle>
struct LightBufferDataTemplate {
	uint32_t type;
	Vector3 position;
	Vector3 direction;
	Color color; // RGB + intensity in alpha
	float range;
	float spotAngleCos;
	THandle shadowMapHandle;
	float shadowBias;
	Matrix viewProj;
};

} //namespace feather
