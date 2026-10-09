#pragma once

// Instantiates the render_data buffer templates for a backend's handle type.
#define DEFINE_RENDER_DATA_TYPES(HandleType)                                                                           \
	using PbrMaterialBufferData = PbrMaterialBufferDataTemplate<HandleType>;                                           \
	using LightBufferData = LightBufferDataTemplate<HandleType>;
