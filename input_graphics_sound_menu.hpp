#ifndef INPUT_GRAPHICS_SOUND_MENU_HPP
#define INPUT_GRAPHICS_SOUND_MENU_HPP

#include <iostream>
#include <vector>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "sbpt_generated_includes.hpp"

enum class UIState {
    // TODO remove the dummy state and fill in with all the states you need
    MAIN_MENU,

    SETTINGS_MENU,
    PROGRAM_SETTINGS,
    INPUT_SETTINGS,
    SOUND_SETTINGS,
    GRAPHICS_SETTINGS,
    ADVANCED_SETTINGS,

    ABOUT,
};

/**
 * @class InputGraphicsSoundMenu
 * @brief Handles all user interface (UI) states related to input, graphics, and sound configuration.
 *
 * This class manages multiple UI panels including main menu, settings menus, and submenus for
 * sound, graphics, input, and player settings. It integrates input, configuration, sound, and rendering systems
 * to create an interactive settings menu for the engine or game.
 *
 * @warning The class currently depends on `InputState` only for key validity checks.
 *          This should be removed once key validation is decoupled.
 */
class InputGraphicsSoundMenu {
  private:
    vertex_geometry::Rectangle settings_menu_rect = vertex_geometry::Rectangle(glm::vec3(0, 0, 0), 1.2, 1.2);
    std::vector<vertex_geometry::Rectangle> settings_menu = weighted_subdivision(settings_menu_rect, {1, 3}, true);

    SoundSystem &sound_system;
    Batcher &batcher;
    Configuration &configuration;
    Window &window;
    InputState &input_state; // only taking this because of the member function to check if key is valid, remove
                             // hopefully in future

    Logger logger = Logger("input_graphics_sound_menu");

    std::function<void()> on_hover = [&]() { sound_system.queue_sound(SoundType::UI_HOVER); };
    std::function<void(const std::string)> dropdown_on_hover = [&](const std::string) {
        sound_system.queue_sound(SoundType::UI_HOVER);
    };

  public:
    bool enabled = true;
    UIState curr_state = UIState::MAIN_MENU;

    UI main_menu_ui, about_ui, settings_menu_ui, player_settings_ui, input_settings_ui, sound_settings_ui,
        graphics_settings_ui, advanced_settings_ui;

    std::map<UIState, UI &> game_state_to_ui = {
        {UIState::MAIN_MENU, main_menu_ui},
        {UIState::ABOUT, about_ui},
        {UIState::SETTINGS_MENU, settings_menu_ui},
        {UIState::PROGRAM_SETTINGS, player_settings_ui},
        {UIState::INPUT_SETTINGS, input_settings_ui},
        {UIState::SOUND_SETTINGS, sound_settings_ui},
        {UIState::GRAPHICS_SETTINGS, graphics_settings_ui},
        {UIState::ADVANCED_SETTINGS, advanced_settings_ui},
    };

    /**
     * @brief Constructs an InputGraphicsSoundMenu and initializes all UIs and configuration handlers.
     *
     * @param window Reference to the main Window.
     * @param input_state Reference to the InputState used for handling input bindings.
     * @param batcher Reference to the Batcher used for UI rendering.
     * @param sound_system Reference to the SoundSystem for playing UI sounds.
     * @param configuration Reference to the Configuration object managing persistent settings.
     *
     * @note This constructor also registers configuration handlers for graphics-related settings
     *       (resolution, fullscreen, wireframe) and applies configuration logic upon initialization.
     * @throws std::invalid_argument If a configuration handler attempts to parse an invalid setting string.
     */
    InputGraphicsSoundMenu(Window &window, InputState &input_state, Batcher &batcher, SoundSystem &sound_system,
                           Configuration &configuration)
        : window(window), input_state(input_state), batcher(batcher), sound_system(sound_system),
          configuration(configuration), main_menu_ui(create_main_menu_ui()), about_ui(create_about_ui()),
          settings_menu_ui(create_settings_menu_ui()), player_settings_ui(create_player_settings_ui()),
          input_settings_ui(create_input_settings_ui()), sound_settings_ui(create_sound_settings_ui()),
          graphics_settings_ui(create_graphics_settings_ui()), advanced_settings_ui(create_advanced_settings_ui()) {

        configuration.register_config_handler("graphics", "resolution",
                                              [&](const std::string resolution) { window.set_resolution(resolution); });

        configuration.register_config_handler("graphics", "fullscreen",
                                              [&](const std::string value) { window.set_fullscreen_by_on_off(value); });

        configuration.register_config_handler("graphics", "wireframe", [&](const std::string value) {
            if (value == "on") {
                window.enable_wireframe_mode();
            } else if (value == "off") {
                window.disable_wireframe_mode();
            }
        });

        configuration.apply_config_logic();

        logger.info("successfully initialized");
    };

  private:
    /**
     * @brief Retrieves the UIStates that should be rendered alongside a given UI.
     *
     * For example the SOUND_SETTINGS UIState requires the more generic SETTINGS_MENU UIState because it is used as the
     * background
     *
     * @param ui_state The primary UI state for which dependencies are queried.
     * @return A vector of dependent UIState values that must also be rendered.
     */
    std::vector<UIState> get_ui_dependencies(const UIState &ui_state) {
        switch (ui_state) {
        case UIState::MAIN_MENU:
            return {};
        case UIState::SETTINGS_MENU:
            return {};
        case UIState::PROGRAM_SETTINGS:
            return {UIState::SETTINGS_MENU};
        case UIState::INPUT_SETTINGS:
            return {UIState::SETTINGS_MENU};
        case UIState::SOUND_SETTINGS:
            return {UIState::SETTINGS_MENU};
        case UIState::GRAPHICS_SETTINGS:
            return {UIState::SETTINGS_MENU};
        case UIState::ADVANCED_SETTINGS:
            return {UIState::SETTINGS_MENU};
        case UIState::ABOUT:
            return {};
            break;
        }
        return {};
    }

    /**
     * @brief Converts mouse coordinates to normalized device coordinates (NDC).
     *
     * @todo make an NDC subproject one day and dump this there
     *
     * @param window Pointer to the GLFW window.
     * @param xpos Mouse X position in screen space.
     * @param ypos Mouse Y position in screen space.
     * @return A glm::vec2 representing the NDC mouse coordinates.
     */
    glm::vec2 get_ndc_mouse_pos(GLFWwindow *window, double xpos, double ypos) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        return {(2.0f * xpos) / width - 1.0f, 1.0f - (2.0f * ypos) / height};
    }

    /**
     * @brief Applies aspect ratio correction to normalized device coordinates.
     *
     * @param ndc_mouse_pos Mouse position in normalized device coordinates.
     * @param x_scale Horizontal aspect scale (typically width/height).
     * @return A glm::vec2 representing aspect-corrected NDC coordinates.
     */
    glm::vec2 aspect_corrected_ndc_mouse_pos(const glm::vec2 &ndc_mouse_pos, float x_scale) {
        return {ndc_mouse_pos.x * x_scale, ndc_mouse_pos.y};
    }

  public:
    /**
     * @brief Processes and queues the rendering of all active menu UIs.
     *
     * @param window Reference to the main window.
     * @param input_state Reference to the input state.
     * @param ui_render_suite Reference to the UI render suite implementation responsible for drawing the UI.
     *
     * @note This function will automatically render all UIs dependent on the current UI state.
     * @warning Ensure that a valid `IUIRenderSuite` implementation (such as `ui_render_suite_implementation`) is
     * provided before calling this function, a sample implementation using the toolbox_engine is here:
     * https://github.com/cpp-toolbox/ui_render_suite_implementation
     */
    void process_and_queue_render_menu(Window &window, InputState &input_state, IUIRenderSuite &ui_render_suite) {
        auto ndc_mouse_pos =
            get_ndc_mouse_pos(window.glfw_window, input_state.mouse_position_x, input_state.mouse_position_y);
        auto acnmp = aspect_corrected_ndc_mouse_pos(ndc_mouse_pos, window.width_px / (float)window.height_px);

        std::vector<UIState> uis_to_render = {curr_state};
        for (const auto &ui_state : get_ui_dependencies(curr_state)) {
            uis_to_render.push_back(ui_state);
        }

        for (const auto &ui_state : uis_to_render) {
            if (game_state_to_ui.find(ui_state) != game_state_to_ui.end()) {
                UI &selected_ui = game_state_to_ui.at(ui_state);

                process_and_queue_render_ui(
                    acnmp, selected_ui, ui_render_suite, input_state.get_keys_just_pressed_this_tick(),
                    input_state.is_just_pressed(EKey::BACKSPACE), input_state.is_just_pressed(EKey::ENTER),
                    input_state.is_just_pressed(EKey::LEFT_MOUSE_BUTTON));
            }
        }
    }

  private:
    /**
     * @brief Creates and returns the Main Menu UI.
     *
     * @return A fully constructed UI object representing the main menu.
     *
     * @details The main menu includes buttons for RESUME, SETTINGS, ABOUT, and QUIT.
     * @note Each button plays a UI click or hover sound using SoundSystem callbacks.
     */
    UI create_main_menu_ui() {

        std::function<void()> on_program_start = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            enabled = false;
        };
        std::function<void()> on_click_settings = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::PROGRAM_SETTINGS;
        };
        std::function<void()> on_click_about = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::ABOUT;
        };
        std::function<void()> on_game_quit = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            glfwSetWindowShouldClose(window.glfw_window, GLFW_TRUE);
        };
        std::function<void()> on_back_clicked = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::MAIN_MENU;
        };

        // UIRenderSuiteImpl ui_render_suite(batcher);

        // main menu ui
        UI main_menu_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        main_menu_ui.add_textbox("Welcome to the program.", 0, 0.75, 1, 0.25, colors::grey);

        vertex_geometry::Grid grid(4, 1, 0.5, 0.5);
        auto frag_time_rect = grid.get_at(0, 0);
        main_menu_ui.add_clickable_textbox(on_program_start, on_hover, "RESUME", frag_time_rect, colors::darkgreen,
                                           colors::green);

        auto settings_rect = grid.get_at(0, 1);
        main_menu_ui.add_clickable_textbox(on_click_settings, on_hover, "SETTINGS", settings_rect, colors::darkblue,
                                           colors::blue);

        auto credits_rect = grid.get_at(0, 2);
        main_menu_ui.add_clickable_textbox(on_click_about, on_hover, "ABOUT", credits_rect, colors::darkblue,
                                           colors::blue);

        auto exit_rect = grid.get_at(0, 3);
        main_menu_ui.add_clickable_textbox(on_game_quit, on_hover, "QUIT", exit_rect, colors::darkred, colors::red);

        return main_menu_ui;
    }

    /**
     * @brief Creates and returns the About UI.
     *
     * @return A fully constructed UI object for the About section.
     *
     * @details Displays information about the toolbox engine and provides a back button
     *          to return to the main menu.
     */
    UI create_about_ui() {
        std::function<void()> on_back_clicked = [&]() { curr_state = {UIState::MAIN_MENU}; };

        UI about_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        about_ui.add_textbox(
            text_utils::add_newlines_to_long_string(
                "this program was created with the toolbox engine, this engine is an open source collection of tools "
                "which come together to form an engine to make games using c++, it's designed for programmers and just "
                "gives you tools to do stuff faster in that realm instead of an all encompassing solution. Learn more "
                "about it at cpptbx.cuppajoeman.com and join the discord."),
            0, 0, 1, 1, colors::grey18);

        about_ui.add_clickable_textbox(on_back_clicked, on_hover, "back to main menu", -0.65, -0.65, 0.5, 0.5,
                                       colors::seagreen, colors::grey);

        return about_ui;
    }

    /**
     * @brief Creates and returns the Settings Menu UI.
     *
     * @return A fully constructed UI object for the settings menu.
     *
     * @details The menu provides access to player, input, sound, graphics, and advanced settings.
     *          It also includes buttons for saving, applying, and going back.
     *
     * @todo Consider extracting repeated button creation patterns into helper functions.
     */
    UI create_settings_menu_ui() {
        UI settings_menu_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        vertex_geometry::Grid top_row_grid(1, 5, settings_menu.at(0));

        std::function<void()> on_back_clicked = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::MAIN_MENU;
        };
        std::function<void()> on_apply_clicked = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.apply_config_logic();
        };
        std::function<void()> on_save_clicked = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.save_to_file();
        };
        std::function<void()> settings_on_click = [&]() { sound_system.queue_sound(SoundType::UI_CLICK); };

        std::function<void()> player_on_click = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::PROGRAM_SETTINGS;
        };
        auto player_rect = top_row_grid.get_at(0, 0);
        settings_menu_ui.add_clickable_textbox(player_on_click, on_hover, "player", player_rect, colors::darkblue,
                                               colors::blue);

        std::function<void()> input_on_click = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::INPUT_SETTINGS;
        };
        auto input_rect = top_row_grid.get_at(1, 0);
        settings_menu_ui.add_clickable_textbox(input_on_click, on_hover, "input", input_rect, colors::darkblue,
                                               colors::blue);

        std::function<void()> sound_on_click = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::SOUND_SETTINGS;
        };
        auto sound_rect = top_row_grid.get_at(2, 0);
        settings_menu_ui.add_clickable_textbox(sound_on_click, on_hover, "sound", sound_rect, colors::darkblue,
                                               colors::blue);

        std::function<void()> graphics_on_click = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::GRAPHICS_SETTINGS;
        };
        auto graphics_rect = top_row_grid.get_at(3, 0);
        settings_menu_ui.add_clickable_textbox(graphics_on_click, on_hover, "graphics", graphics_rect, colors::darkblue,
                                               colors::blue);

        std::function<void()> network_on_click = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::ADVANCED_SETTINGS;
        };
        auto network_rect = top_row_grid.get_at(4, 0);
        settings_menu_ui.add_clickable_textbox(network_on_click, on_hover, "network", network_rect, colors::darkblue,
                                               colors::blue);

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        settings_menu_ui.add_colored_rectangle(main_settings_rect, colors::grey);

        vertex_geometry::Rectangle go_back_rect = vertex_geometry::create_rectangle_from_corners(
            glm::vec3(-1, -0.75, 0), glm::vec3(-0.75, -0.75, 0), glm::vec3(-1, -1, 0), glm::vec3(-0.75, -1, 0));
        settings_menu_ui.add_clickable_textbox(on_back_clicked, on_hover, "BACK", go_back_rect, colors::darkred,
                                               colors::red);

        vertex_geometry::Rectangle apply_rect = vertex_geometry::create_rectangle_from_corners(
            glm::vec3(1, -0.75, 0), glm::vec3(0.75, -0.75, 0), glm::vec3(1, -1, 0), glm::vec3(0.75, -1, 0));
        settings_menu_ui.add_clickable_textbox(on_apply_clicked, on_hover, "APPLY", apply_rect, colors::darkgreen,
                                               colors::green);

        vertex_geometry::Rectangle save_rect = vertex_geometry::slide_rectangle(apply_rect, -1, 0);

        settings_menu_ui.add_clickable_textbox(on_save_clicked, on_hover, "SAVE", save_rect, colors::darkgreen,
                                               colors::green);

        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        return settings_menu_ui;
    }

    /**
     * @brief Creates and returns the Player Settings UI.
     *
     * @return A fully constructed UI object for player-related settings.
     *
     * @details Currently allows editing of the username and crosshair configuration.
     * @todo Extend this menu to include additional player customization options.
     */
    UI create_player_settings_ui() {

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        UI player_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        std::function<void(std::string)> username_on_confirm = [](std::string s) {};
        player_settings_ui.add_textbox("username", main_settings_grid.get_at(0, 0), colors::maroon);
        player_settings_ui.add_input_box(on_confirm, "username", main_settings_grid.get_at(2, 0), colors::orange,
                                         colors::orangered);
        player_settings_ui.add_textbox("crosshair", main_settings_grid.get_at(0, 1), colors::maroon);

        vertex_geometry::Grid input_settings_grid(10, 3, main_settings_rect);

        return player_settings_ui;
    }

    /**
     * @brief Creates and returns the Input Settings UI.
     *
     * @return A fully constructed UI object for input configuration.
     *
     * @details Provides editable input bindings for key actions (forward, back, left, right, etc.)
     *          and configurable mouse sensitivity.
     *
     * @warning Uses `InputState::is_valid_key_string` for validation — this dependency should be removed in future.
     */
    UI create_input_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid input_settings_grid(11, 3, main_settings_rect);
        UI input_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        input_settings_ui.add_textbox("mouse sensitivity", input_settings_grid.get_at(0, 0), colors::maroon);

        std::function<void(std::string)> sens_on_click = [this](std::string option) {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.set_value("input", "mouse_sensitivity", option);
        };

        input_settings_ui.add_input_box(sens_on_click, "1", input_settings_grid.get_at(2, 0), colors::grey,
                                        colors::lightgrey);

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        input_settings_ui.add_textbox("fire", input_settings_grid.get_at(0, 1), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "lmb", input_settings_grid.get_at(2, 1), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("jump", input_settings_grid.get_at(0, 2), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "space", input_settings_grid.get_at(2, 2), colors::grey,
                                        colors::lightgrey);

        std::function<std::function<void(std::string)>(std::string)> create_key_on_confirm_function =
            [&](std::string key_str) {
                return [this, key_str](std::string input_value) {
                    if (input_state.is_valid_key_string(input_value)) {
                        configuration.set_value("input", key_str, input_value);
                        sound_system.queue_sound(SoundType::UI_CLICK);
                    } else {
                        logger.warn("{} is not a valid key string, not setting it in the config, use a proper value.",
                                    input_value);
                    }
                };
            };

        input_settings_ui.add_textbox("move forward", input_settings_grid.get_at(0, 3), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("forward"),
                                        configuration.get_value("input", "forward").value_or("w"),
                                        input_settings_grid.get_at(2, 3), colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("move backward", input_settings_grid.get_at(0, 4), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("back"), "s", input_settings_grid.get_at(2, 4),
                                        colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("move left", input_settings_grid.get_at(0, 5), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("left"), "a", input_settings_grid.get_at(2, 5),
                                        colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("move right", input_settings_grid.get_at(0, 6), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("right"), "d", input_settings_grid.get_at(2, 6),
                                        colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("move up", input_settings_grid.get_at(0, 7), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("left"), " ", input_settings_grid.get_at(2, 7),
                                        colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("move down", input_settings_grid.get_at(0, 8), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("down"), "left_shift",
                                        input_settings_grid.get_at(2, 8), colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("slow move", input_settings_grid.get_at(0, 9), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("slow_move"), "left_control",
                                        input_settings_grid.get_at(2, 9), colors::grey, colors::lightgrey);

        input_settings_ui.add_textbox("fast move", input_settings_grid.get_at(0, 10), colors::maroon);
        input_settings_ui.add_input_box(create_key_on_confirm_function("fast_move"), "tab",
                                        input_settings_grid.get_at(2, 10), colors::grey, colors::lightgrey);

        return input_settings_ui;
    }

    /**
     * @brief Creates and returns the Sound Settings UI.
     *
     * @return A fully constructed UI object for sound settings.
     *
     * @details Currently only provides a volume control placeholder.
     * @todo Implement actual volume adjustment integration with SoundSystem.
     */
    UI create_sound_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid sound_settings_grid(1, 3, main_settings_rect);
        UI sound_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        sound_settings_ui.add_textbox("volume", sound_settings_grid.get_at(0, 0), colors::maroon);
        return sound_settings_ui;
    }

    int get_index_or_default(const std::string &value, const std::vector<std::string> &vec) {
        auto it = std::find(vec.begin(), vec.end(), value);
        return (it != vec.end()) ? std::distance(vec.begin(), it) : 0;
    }

    /**
     * @brief Creates and returns the Graphics Settings UI.
     *
     * @return A fully constructed UI object for graphics settings.
     *
     * @details Includes controls for resolution, fullscreen, wireframe mode, FOV, FPS cap,
     *          and options to toggle FPS and position display.
     *
     * @warning On macOS, available resolution detection may fail; a fallback resolution ("1920x1080") is used.
     * @todo Add dropdown validation for resolution parsing and better error feedback.
     */
    UI create_graphics_settings_ui() {

        std::vector<std::string> resolutions = get_available_resolutions("16:9");
        // NOTE: on mac this returns empty so for compliation purposes I'm just going to hack a fake value in
        if (resolutions.empty())
            resolutions = {"1920x1080"};

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        std::function<void()> on_click_settings = [&]() { curr_state = UIState::PROGRAM_SETTINGS; };

        std::vector<std::string> on_off_options = {"on", "off"};

        vertex_geometry::Grid graphics_settings_grid(10, 3, main_settings_rect);
        UI graphics_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);

        std::vector<std::string> options = resolutions;
        std::vector<std::function<void()>> option_on_clicks(options.size(), []() {});

        std::function<void(std::string)> empty_on_click = [](std::string option) { std::cout << option << std::endl; };

        std::function<void(std::string)> resolution_dropdown_on_click = [this](std::string option) {
            sound_system.queue_sound(SoundType::UI_CLICK);
            size_t x_pos = option.find('x');
            unsigned int width, height;
            if (x_pos != std::string::npos) {
                width = std::stoi(option.substr(0, x_pos));
                height = std::stoi(option.substr(x_pos + 1));
                // the above verifies that indeed the things are numbers which means its valid I think... probably not
                // needed since the options are already
                configuration.set_value("graphics", "resolution", option);
            } else {
                throw std::invalid_argument("Input string is not in the correct format (e.g. 1280x960)");
            }
        };

        int dropdown_option_idx;

        dropdown_option_idx =
            get_index_or_default(configuration.get_value("graphics", "resolution").value_or("1280x720"), options);
        graphics_settings_ui.add_textbox("resolution", graphics_settings_grid.get_at(0, 0), colors::maroon);

        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 0), colors::orange, colors::orangered,
                                          options, resolution_dropdown_on_click, dropdown_on_hover);

        std::function<void(std::string)> fullscreen_on_click = [this](std::string option) {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.set_value("graphics", "fullscreen", option);
        };

        dropdown_option_idx =
            get_index_or_default(configuration.get_value("graphics", "fullscreen").value_or("off"), on_off_options);
        graphics_settings_ui.add_textbox("fullscreen", graphics_settings_grid.get_at(0, 1), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 1), colors::orange, colors::orangered,
                                          on_off_options, fullscreen_on_click, dropdown_on_hover);

        std::function<void(std::string)> wireframe_on_click = [this](std::string option) {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.set_value("graphics", "wireframe", option);
        };

        dropdown_option_idx =
            get_index_or_default(configuration.get_value("graphics", "wireframe").value_or("off"), on_off_options);
        graphics_settings_ui.add_textbox("wireframe", graphics_settings_grid.get_at(0, 2), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 2), colors::orange, colors::orangered,
                                          on_off_options, wireframe_on_click, dropdown_on_hover);

        std::function<void(std::string)> fov_on_confirm = [&](std::string option) {
            configuration.set_value("graphics", "field_of_view", option);
        };

        graphics_settings_ui.add_textbox("field of view", graphics_settings_grid.get_at(0, 3), colors::maroon);
        graphics_settings_ui.add_input_box(
            fov_on_confirm, configuration.get_value("graphics", "field_of_view").value_or("degrees (30-160 limit)"),
            graphics_settings_grid.get_at(2, 3), colors::grey, colors::lightgrey);

        std::function<void(std::string)> max_fps_on_confirm = [&](std::string option) {
            configuration.set_value("graphics", "max_fps", option);
        };

        graphics_settings_ui.add_textbox("max fps", graphics_settings_grid.get_at(0, 4), colors::maroon);
        graphics_settings_ui.add_input_box(max_fps_on_confirm,
                                           configuration.get_value("graphics", "max_fps").value_or("120"),
                                           graphics_settings_grid.get_at(2, 4), colors::grey, colors::lightgrey);

        std::function<void(std::string)> show_fps_on_click = [&](std::string option) {
            configuration.set_value("graphics", "show_fps", option);
        };

        graphics_settings_ui.add_textbox("show fps", graphics_settings_grid.get_at(0, 5), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 5), colors::orange, colors::orangered,
                                          on_off_options, show_fps_on_click, dropdown_on_hover);

        std::function<void(std::string)> show_pos_on_click = [&](std::string option) {
            configuration.set_value("graphics", "show_pos", option);
        };

        graphics_settings_ui.add_textbox("show pos", graphics_settings_grid.get_at(0, 6), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 6), colors::orange, colors::orangered,
                                          on_off_options, show_pos_on_click, dropdown_on_hover);

        return graphics_settings_ui;
    }

    /**
     * @brief Creates and returns the Advanced Settings UI.
     *
     * @return A fully constructed UI object for advanced diagnostics settings.
     *
     * @details Provides toggles for visualizing tick time, ping, and movement dial indicators.
     */
    UI create_advanced_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid advanced_settings_grid(3, 3, main_settings_rect);
        UI advanced_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        advanced_settings_ui.add_textbox("display tick time expendature", advanced_settings_grid.get_at(0, 0),
                                         colors::maroon);
        advanced_settings_ui.add_textbox("display current ping", advanced_settings_grid.get_at(0, 1), colors::maroon);
        advanced_settings_ui.add_textbox("display movement dial", advanced_settings_grid.get_at(0, 2), colors::maroon);

        return advanced_settings_ui;
    }
};

#endif // INPUT_GRAPHICS_SOUND_MENU_HPP
