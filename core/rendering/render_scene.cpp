#include "render_scene.h"

namespace feather {

RenderScene::RenderScene(size_t frame_index) : _frame_index(frame_index) {}

INPLACE_REGISTER_BEGIN(RenderScene);
ClassDB::register_class<RenderScene>();
INPLACE_REGISTER_END(RenderScene);

} //namespace feather