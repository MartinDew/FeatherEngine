export module feather.vex_renderer;

export import :renderer;
export import :register_vex_renderer_types;

export namespace feather {
void register_vex_renderer();

void unregister_vex_renderer();
} //namespace feather