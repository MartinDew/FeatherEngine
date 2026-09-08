#pragma once

// Defining ECS component and system types from a plugin.
//
// Everything else about the ECS is reached through the generated wrappers:
// feather::World, feather::Entity and feather::ComponentHandle are the engine's
// own classes, with the engine's own method names. Only these two definitions
// need hand-writing, because both cross as things mrbind cannot spell -- a
// component's field list is std::vector<ClassInfo::Property> engine-side, and a
// system is a function pointer.
//
// They wrap modules/c_bindings/scripted_abi.h, the same flat entry points the
// C# bootstrap calls. What this adds is std types, real enums, and a failure
// that raises itself instead of a buffer the caller has to check.

#include <feather_cpp/core.hpp>
#include <feather_cpp/feather.hpp>

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

extern "C"
{
    // Mirrors modules/c_bindings/scripted_abi.h.
    typedef void (*FeatherScriptSystemFn)(
        void *user_data, std::uint64_t entity, void *const *components,
        std::int32_t component_count, double delta_time);

    std::uint64_t feather_script_define_component(
        const char *name, std::int32_t field_count, const char *const *field_names,
        const std::uint8_t *field_types, char *error, std::int32_t error_size);

    std::uint64_t feather_script_define_system(
        const char *name, std::int32_t component_count, const char *const *component_names,
        std::uint8_t phase, FeatherScriptSystemFn callback, void *user_data,
        char *error, std::int32_t error_size);

    std::int32_t feather_script_field_count(const char *component_name);
    std::int32_t feather_script_field_info(
        const char *component_name, std::int32_t index, const char **out_name, std::uint8_t *out_type);
}

namespace feather
{
    // Only trivially-copyable types: a scripted component lives in flecs'
    // zero-initialized storage, which runs no destructor.
    enum class FieldType : std::uint8_t
    {
        Bool = 0, Int = 1, Float = 2, Vec2 = 3, Vec3 = 4, Color = 5,
    };

    enum class Phase : std::uint8_t
    {
        OnLoad = 0, PostLoad = 1, PreUpdate = 2, OnUpdate = 3,
        OnValidate = 4, PostUpdate = 5, PreStore = 6, OnStore = 7,
    };

    struct Field
    {
        std::string name;
        FieldType type = FieldType::Float;
    };

    // What a system is handed for one matching entity. The entity is a handle
    // like any other, so a system reads and writes through the same
    // Entity/ComponentHandle API the rest of the plugin uses.
    struct Invocation
    {
        Entity entity;
        double delta_time = 0.0;
    };

    using SystemCallback = std::function<void(const Invocation &)>;

    namespace detail
    {
        // The engine keeps the callback's address for the life of the process and never frees it, so it is deliberately
        // leaked rather than owned by something whose destruction order we cannot see.
        struct SystemState
        {
            SystemCallback callback;
        };

        inline void system_trampoline(
            void *user_data, std::uint64_t entity, void *const *, std::int32_t, double delta_time)
        {
            auto *state = static_cast<SystemState *>(user_data);

            // Rebuilt per call rather than cached: the world is the engine's,
            // and this holds a view of it only for the length of the callback.
            World world = WorldSim::get().get_world();
            const Invocation invocation{
                .entity = Entity::create(world, entity),
                .delta_time = delta_time,
            };

            // Nothing may escape into the engine's C frame above us. With
            // exceptions off there is nothing that could.
#if FEATHER_CPP_EXCEPTIONS
            try
            {
                state->callback(invocation);
            }
            catch (...)
            {
            }
#else
            state->callback(invocation);
#endif
        }
    }

    // Registers a new component type, which is then an ordinary component: add
    // it by name, read and write it through a ComponentHandle. Throws
    // feather::Error with the engine's own message if the name is taken or a
    // field type cannot be stored.
    inline std::uint64_t define_component(std::string_view name, std::span<const Field> fields)
    {
        std::vector<std::string> owned_names;
        std::vector<const char *> name_ptrs;
        std::vector<std::uint8_t> types;
        owned_names.reserve(fields.size());
        for (const Field &f : fields)
            owned_names.push_back(f.name);
        for (const std::string &n : owned_names)
            name_ptrs.push_back(n.c_str());
        for (const Field &f : fields)
            types.push_back(static_cast<std::uint8_t>(f.type));

        char error[512] = {};
        const std::string owned(name);
        const std::uint64_t id = ::feather_script_define_component(
            owned.c_str(), std::int32_t(fields.size()), name_ptrs.data(), types.data(), error, sizeof(error));
        if (id == 0)
            ::feather::detail::fail(error[0] ? error : "cannot define component '" + owned + "'");
        return id;
    }

    // Registers a system over the named components, scripted or built in.
    inline std::uint64_t define_system(
        std::string_view name, std::span<const std::string> components, Phase phase, SystemCallback callback)
    {
        auto *state = new detail::SystemState{.callback = std::move(callback)};

        std::vector<std::string> owned_names(components.begin(), components.end());
        std::vector<const char *> name_ptrs;
        name_ptrs.reserve(owned_names.size());
        for (const std::string &n : owned_names)
            name_ptrs.push_back(n.c_str());

        char error[512] = {};
        const std::string owned(name);
        const std::uint64_t id = ::feather_script_define_system(
            owned.c_str(), std::int32_t(name_ptrs.size()), name_ptrs.data(),
            static_cast<std::uint8_t>(phase), &detail::system_trampoline, state, error, sizeof(error));
        if (id == 0)
        {
            delete state;
            ::feather::detail::fail(error[0] ? error : "cannot define system '" + owned + "'");
        }
        return id;
    }

    // The fields a component was registered with, in order.
    [[nodiscard]] inline std::vector<Field> fields_of(const char *component)
    {
        const std::int32_t count = ::feather_script_field_count(component);
        if (count < 0)
            ::feather::detail::fail(std::string("no component named '") + component + "'");

        std::vector<Field> ret;
        ret.reserve(std::size_t(count));
        for (std::int32_t i = 0; i < count; i++)
        {
            const char *field_name = nullptr;
            std::uint8_t type = 0;
            if (!::feather_script_field_info(component, i, &field_name, &type))
                ::feather::detail::fail(std::string("cannot describe field ") + std::to_string(i) + " of '" + component + "'");
            ret.push_back(Field{.name = field_name ? field_name : "", .type = static_cast<FieldType>(type)});
        }
        return ret;
    }
}
