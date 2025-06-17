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

It also relies on `InputState`, so make sure you set that up.
