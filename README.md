# input graphics sound menu

This is a basic menu that could used in almost any 3d program. Additionally it acts as the default menu for all cpptbx programs as a starting point.

In order to use the menu a few things need to happen, first it requires the following sounds to be defined:

```cpp
    std::unordered_map<SoundType, std::string> sound_type_to_file = {
        {SoundType::UI_HOVER, "assets/sounds/hover.wav"},
        {SoundType::UI_CLICK, "assets/sounds/click.wav"},
        {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
    };
    SoundSystem sound_system(100, sound_type_to_file);

```

The menu requires a few systems and a single shader to render itself, so when initializing the shader cache we do this:
```cpp
    std::vector<ShaderType> requested_shaders = {ShaderType::ABSOLUTE_POSITION_WITH_COLORED_VERTEX};
    ShaderCache shader_cache(requested_shaders);
    Batcher batcher(shader_cache);
```

It also requires the configuration system that's in charge of a config file:

```cpp
    Configuration configuration("assets/config/user_cfg.ini");
```

Then you can create it: 

```cpp
    InputGraphicsSoundMenu input_graphics_sound_menu(window, batcher, sound_system, configuration);
```

In order for it to render you have to call the following member function: 
```cpp
    void process_and_queue_render_menu(Window &window, InputState &input_state, IUIRenderSuite &ui_render_suite);
```

The `IUIRenderSuite` is an interface that when implemented renders the UI, here is an example implementation: https://github.com/cpp-toolbox/ui_render_suite_implementation


It also relies on `InputState`, so make sure you set that up.
