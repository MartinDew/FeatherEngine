# Code Review — `main-engine-flow`

10 findings, ranked most-severe first. Status: CONFIRMED (verified by reading code) or PLAUSIBLE (realistic but depends on call-site).

---

## 1. Textures never load — dot mismatch in `recognize_extension` (CONFIRMED)

**`core/resources/texture_format_loader.cpp:10`**

`strip_extension()` (`resource_loader.cpp:38`) removes the leading `.` before passing to loaders, yielding `"png"`. `TextureFormatLoader::recognize_extension` compares against `".png"` (with dot) so it always returns false. Every other loader (`MaterialFormatLoader`, `MeshFormatLoader`, `ExtensionFormatLoader`) compares without a dot — this is a regression unique to textures.

Any project with `.png`/`.jpg` assets: `index_project()` silently skips all texture files and `ResourceLoader::load("res://foo.png")` returns `nullptr`.

---

## 2. Lost-update race on `_dirty` can silently drop a frame (CONFIRMED)

**`core/rendering/rendering_server.cpp:26`**

Between the render thread passing the CV predicate (dirty=true, frame N) and re-acquiring `_write_lock` to clear it, the main thread can commit frame N+1 (`dirty=true`, `notify_one()`). That notify is consumed immediately (no thread is waiting). The render thread then copies frame N's buffer and stores `dirty=false`. The CV re-enters `wait()` with dirty=false and no pending notification — frame N+1 is silently skipped until frame N+2 is committed.

**Fix:** clear `_dirty` before exiting the `_wait_mutex` scope (while the CV lock is still held), ensuring the clearing is atomic with respect to new notifications.

---

## 3. Data race on plain `bool _needs_resize` (CONFIRMED)

**`core/rendering/rendering_server.h:38`**

`_needs_resize` is a plain `bool` written on the main thread (notification lambda, `rendering_server.cpp:66`) and read + written on the render thread (`_render_function`, line 36). No mutex or atomic protects either access. This is a C++ data race (UB). With optimizations enabled the compiler may cache the read in a register, causing the render thread to never observe a resize, or producing a mis-sized swap chain.

**Fix:** `std::atomic<bool> _needs_resize{false}`.

---

## 4. `get_static_method()` null-dereferences on unknown class name (PLAUSIBLE)

**`core/main/class_db.cpp:112`**

`get_static_method` calls `_get_class_info_internal(class_name)` which returns `nullptr` for any name not in `_class_infos`, then immediately accesses `ci->methods.begin()` — null pointer dereference / UB. Reachable from any extension, tool, or module that queries a method on a class with a typo or on a class not yet registered. The `on_subclass_registered` dynamic system makes registration ordering non-deterministic.

---

## 5. `has_parent()` null-dereferences on unknown class name (PLAUSIBLE)

**`core/main/class_db.cpp:87`**

`has_parent` calls `_get_class_info_internal(object_name)` and immediately reads `ci->parent` without a null guard. The traversal loop also calls `_get_class_info_internal(ci->parent)` and dereferences the result on the next iteration — so a registered class whose declared parent was never registered also crashes mid-traversal.

---

## 6. `scene_query<TComps>()` static captures `_world` at first call — stale after re-creation (PLAUSIBLE)

**`core/main/world_sim.h:72`**

`static auto q = _world.query_builder<TComps...>().build()` is constructed once on first call, embedding a Flecs world handle. Any test harness or hot-reload scenario that destroys and re-creates `WorldSim` leaves all callers of `scene_query` holding a stale world handle, causing use-after-free when the query is iterated.

---

## 7. Notification lambda captures `this` (RenderingServer) stored indefinitely in Window (PLAUSIBLE)

**`core/rendering/rendering_server.cpp:66`**

`Window::_notification_listeners` stores the `[this]{ _needs_resize = true; }` lambda for the Window's lifetime. If `Window` is ever extracted into a separately-owned object that outlives `RenderingServer`, the stored pointer becomes dangling and the next window resize fires a use-after-free.

**Fix:** once `_needs_resize` is made atomic (finding #3), capture only `&_needs_resize` instead of `this`.

---

## 8. `_compute_light_view_proj` called twice per light per frame (PLAUSIBLE — efficiency)

**`modules/vex_renderer/vex_renderer.cpp:454`**

`_compute_light_view_proj` is called once in `_render_shadow_pass` and again in `_upload_lights_buffer` for every light, every frame. Both calls internally invoke `_compute_scene_center` and `_compute_scene_radius`, which each iterate all entities. With N shadow lights and M entities: 4×N×M entity traversals per frame, where 2×N×M are redundant. Caching the computed matrices alongside `_light_to_shadow_map_index` would halve the traversal cost.

---

## 9. `ResourceLoader::load()` re-derives extension and localized path in two branches (cleanup)

**`core/resources/resource_loader.cpp:44`**

The cache-miss path (lines ~61–68) re-computes `extension` and `localized_path` from scratch, duplicating the same two calls made in the cache-hit guard just above (lines ~48–56). Any future change to `strip_extension` or `localize_path` must be applied in both branches. Extracting the two shared computations to before the cache lookup eliminates the duplication.

---

## 10. Dead platform headers in `shared_library.cpp` (cleanup)

**`core/framework/shared_library.cpp:6`**

`<windows.h>` and `<dlfcn.h>` are included but never called — all symbol loading is delegated to `SDL_LoadObject`/`SDL_LoadFunction` which already abstracts them. On Windows, `<windows.h>` without `WIN32_LEAN_AND_MEAN`/`NOMINMAX` injects `min`/`max` macros that can collide with STL headers in module targets that don't inherit the Editor/Standalone `PUBLIC` compile definitions. Remove the dead includes.
