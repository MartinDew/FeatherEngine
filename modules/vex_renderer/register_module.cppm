export module feather.vex_renderer;

export import :renderer;
export import :register_vex_renderer_types;

export namespace feather {
FEATHER_VEX_RENDERER_API void register_vex_renderer();

FEATHER_VEX_RENDERER_API void unregister_vex_renderer();
} //namespace feather