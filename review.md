# Code Review — `main-engine-flow`

10 findings, ranked most-severe first. **Revalidated 2026-06-16** against current tree.

---

## 1. ~~Textures never load — dot mismatch in `recognize_extension`~~ ✅ FIXED

**Fixed by commit `51d4e6b`.**  
`texture_format_loader.cpp:10` now compares against `"png"`, `"jpg"`, etc. without dots, matching `strip_extension()`'s output.

---

## 2. ~~Lost-update race on `_dirty` can silently drop a frame~~ ✅ FIXED

`_dirty` is now cleared inside the `_wait_mutex` unique_lock scope (before the lock is released), so any subsequent `notify_one()` from the main thread is either delivered to the still-waiting CV or causes the predicate to fire immediately on the next `wait()` call.

---

## 3. ~~Data race on plain `bool _needs_resize`~~ ✅ FIXED

`_needs_resize` is now `std::atomic<bool>` in `rendering_server.h`. Accesses in `_render_function` use explicit `load`/`store` with relaxed ordering (correctness is provided by the surrounding CV wake-up synchronization).

---

## 4. ~~`get_static_method()` null-dereferences on unknown class name~~ ✅ FIXED

`get_static_method` now returns `{}` immediately if `_get_class_info_internal` returns `nullptr`.

---

## 5. ~~`has_parent()` null-dereferences on unknown class name~~ ✅ FIXED

`has_parent` now guards the loop with `while (ci && ci->parent != ""_ss)`, handling both unknown `object_name` and a registered class whose declared parent was never registered. Removed the dead `_current_name` variable.

---

## 6. `scene_query<TComps>()` static captures `_world` at first call — stale after re-creation (PLAUSIBLE — OPEN)

**`core/main/world_sim.h:72`**

`static auto q = _world.query_builder<TComps...>()...build()` is constructed once, embedding a Flecs world handle. Any test harness or hot-reload scenario that destroys and re-creates `WorldSim` leaves all callers holding a stale handle — use-after-free when the query is iterated.

---

## 7. Notification lambda captures `this` stored indefinitely in Window (PLAUSIBLE — OPEN)

**`core/rendering/rendering_server.cpp:66`**

`[this]{ _needs_resize = true; }` is stored in `Window::_notification_listeners` for the Window's lifetime. If `Window` ever outlives `RenderingServer`, the next resize fires a use-after-free. Once finding #3 is fixed (`_needs_resize` made atomic), prefer capturing `&_needs_resize` instead of `this`.

---

## 8. `_compute_light_view_proj` called twice per light per frame (PLAUSIBLE — OPEN)

**`modules/vex_renderer/vex_renderer.cpp:454` and `:631`**

`_compute_light_view_proj` is called in `_render_shadow_pass` (line 454) and again in `_upload_lights_buffer` (line 631) for every light, every frame. Each call internally runs `_compute_scene_center` and `_compute_scene_radius`, which each iterate all entities — 4×N×M traversals per frame where 2×N×M are redundant. Cache the computed matrices alongside `_light_to_shadow_map_index`.

---

## 9. `ResourceLoader::load()` duplicates extension and localized-path derivation (cleanup — OPEN)

**`core/resources/resource_loader.cpp:49–50` and `:61–67`**

`strip_extension` and `localize_path` appear in both the cache-hit branch and the cache-miss branch. Extracting them before the cache lookup eliminates the duplication and ensures future changes only need one edit.

---

## 10. Dead platform headers in `shared_library.cpp` (cleanup — OPEN)

**`core/framework/shared_library.cpp:6–9`**

`<windows.h>` / `<dlfcn.h>` are conditionally included but never called — all symbol loading goes through `SDL_LoadObject`/`SDL_LoadFunction`. On Windows, `<windows.h>` without `WIN32_LEAN_AND_MEAN`/`NOMINMAX` injects `min`/`max` macros that can collide with STL in module targets. Remove the `#if` block entirely.