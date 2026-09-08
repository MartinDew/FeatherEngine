// A minimal Feather extension in C++.
//
// Everything here resolves to a flat feather_* C symbol in the running engine.
// No engine headers are involved, and no C++ types cross the boundary.

#include <feather_cpp/feather.hpp>
#include <feather_cpp/plugin.hpp>
#include <feather_cpp/scripted_abi.hpp>

#include <cstdio>

namespace
{
    // Called once per init level, in order. Core is the first level at which
    // the reflection database exists; the ECS world is not up until World.
    void on_init_level(feather::InitLevel level) // TODO: rename along with the entry point below
    {
        std::printf("[my_plugin] init level '%s' entered\n", feather::to_string(level));

        if (level == feather::InitLevel::Core)
        {
            // The math types are the engine's own, compiled from the core/math
            // sources the SDK vendors, so they cross by value.
            feather::Transform transform = feather::Transform::create();
            transform.set_position(feather::Vector3(1.0f, 2.0f, 3.0f));
            const feather::Vector3 position = transform.get_position();
            std::printf("[my_plugin] transform at (%.2f, %.2f, %.2f)\n",
                position.x, position.y, position.z);
        }

        if (level == feather::InitLevel::World)
        {
            // A component and a system defined at runtime, through the same
            // flat C ABI a C# plugin uses. Everything after the two definitions
            // is the engine's own World/Entity/ComponentHandle API.
            const feather::Field fields[] = {
                {.name = "speed", .type = feather::FieldType::Float},
                {.name = "ticks", .type = feather::FieldType::Int},
            };
            feather::define_component("MyPluginSpin", fields);

            const std::string components[] = {"MyPluginSpin"};
            feather::define_system("my_plugin_spin", components,
                feather::Phase::OnUpdate,
                [](const feather::Invocation &invocation)
                {
                    feather::ComponentHandle spin = invocation.entity.component("MyPluginSpin");
                    const std::int32_t ticks = spin.get_int("ticks") + 1;
                    spin.set_int("ticks", ticks);
                    spin.set_float("speed", spin.get_float("speed") + float(invocation.delta_time));
                });

            feather::World world = feather::WorldSim::get().get_world();
            feather::Entity entity = world.create_entity("MyPluginDemo");
            entity.add_component("MyPluginSpin");
        }
    }
}

// The name here must match the "entry" field of my_plugin.fext.
FEATHER_PLUGIN_ENTRY(register_my_plugin, on_init_level)
