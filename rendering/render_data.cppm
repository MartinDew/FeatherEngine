// Data structures for GPU buffers and shader resources

export module feather.rendering:render_data;

import std.compat;
import feather.math;

export namespace feather {

struct FEATHER_RENDERING_API CameraBufferData {
	Matrix viewProj;
	Vector3 cameraPos;
	float _padding;
};

struct FEATHER_RENDERING_API InstanceBufferData {
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
