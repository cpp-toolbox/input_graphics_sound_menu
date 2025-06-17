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
    PLAYER_SETTINGS,
    INPUT_SETTINGS,
    SOUND_SETTINGS,
    GRAPHICS_SETTINGS,
    ADVANCED_SETTINGS,

    ABOUT,
    IN_GAME,
};

class InputGraphicsSoundMenu {
  private:
    vertex_geometry::Rectangle settings_menu_rect = vertex_geometry::Rectangle(glm::vec3(0, 0, 0), 1.2, 1.2);
    std::vector<vertex_geometry::Rectangle> settings_menu = weighted_subdivision(settings_menu_rect, {1, 3}, true);

    SoundSystem &sound_system;
    Batcher &batcher;
    Configuration &configuration;
    Window &window;

    std::function<void()> on_hover = [&]() { sound_system.queue_sound(SoundType::UI_HOVER); };
    std::function<void(const std::string)> dropdown_on_hover = [&](const std::string) {
        sound_system.queue_sound(SoundType::UI_HOVER);
    };

  public:
    UIState curr_state = UIState::MAIN_MENU;

    UI main_menu_ui, in_game_ui, about_ui, settings_menu_ui, player_settings_ui, input_settings_ui, sound_settings_ui,
        graphics_settings_ui, advanced_settings_ui;

    std::map<UIState, UI &> game_state_to_ui = {
        {UIState::MAIN_MENU, main_menu_ui},
        {UIState::IN_GAME, in_game_ui},
        {UIState::ABOUT, about_ui},
        {UIState::SETTINGS_MENU, settings_menu_ui},
        {UIState::PLAYER_SETTINGS, player_settings_ui},
        {UIState::INPUT_SETTINGS, input_settings_ui},
        {UIState::SOUND_SETTINGS, sound_settings_ui},
        {UIState::GRAPHICS_SETTINGS, graphics_settings_ui},
        {UIState::ADVANCED_SETTINGS, advanced_settings_ui},
    };

    InputGraphicsSoundMenu(Window &window, Batcher &batcher, SoundSystem &sound_system, Configuration &configuration)
        : window(window), batcher(batcher), sound_system(sound_system), configuration(configuration),
          main_menu_ui(create_main_menu_ui()), in_game_ui(create_in_game_ui()), about_ui(create_about_ui()),
          settings_menu_ui(create_settings_menu_ui()), player_settings_ui(create_player_settings_ui()),
          input_settings_ui(create_input_settings_ui()), sound_settings_ui(create_sound_settings_ui()),
          graphics_settings_ui(create_graphics_settings_ui()), advanced_settings_ui(create_advanced_settings_ui()) {

        configuration.register_config_handler("graphics", "resolution",
                                              [&](const std::string resolution) { window.set_resolution(resolution); });

        configuration.register_config_handler("graphics", "fullscreen",
                                              [&](const std::string value) { window.set_fullscreen_by_on_off(value); });

        configuration.apply_config_logic();
    };

    std::vector<UIState> get_ui_dependencies(const UIState &ui_state) {
        switch (ui_state) {
        case UIState::MAIN_MENU:
            return {};
        case UIState::SETTINGS_MENU:
            return {};
        case UIState::PLAYER_SETTINGS:
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
        case UIState::IN_GAME:
            return {};
            ;
            break;
        }
        return {};
    }

    glm::vec2 get_ndc_mouse_pos(GLFWwindow *window, double xpos, double ypos) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        return {(2.0f * xpos) / width - 1.0f, 1.0f - (2.0f * ypos) / height};
    }

    glm::vec2 aspect_corrected_ndc_mouse_pos(const glm::vec2 &ndc_mouse_pos, float x_scale) {
        return {ndc_mouse_pos.x * x_scale, ndc_mouse_pos.y};
    }

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

    UI create_main_menu_ui() {

        std::function<void()> on_game_start = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::IN_GAME;
        };
        std::function<void()> on_click_settings = [&]() {
            sound_system.queue_sound(SoundType::UI_CLICK);
            curr_state = UIState::PLAYER_SETTINGS;
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
        main_menu_ui.add_clickable_textbox(on_game_start, on_hover, "RUN", frag_time_rect, colors::darkgreen,
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

    UI create_in_game_ui() {

        std::function<void()> on_back_clicked = [&]() { curr_state = {UIState::MAIN_MENU}; };

        UI in_game_ui(0, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };
        in_game_ui.add_input_box(on_confirm, "password", 0, 0.25, 1, 0.25, colors::grey, colors::lightgrey);
        in_game_ui.add_clickable_textbox(on_back_clicked, on_hover, "back to main menu", -0.65, -0.65, 0.5, 0.5,
                                         colors::seagreen, colors::grey);

        return in_game_ui;
    }

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
            curr_state = UIState::PLAYER_SETTINGS;
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

    UI create_input_settings_ui() {

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        vertex_geometry::Grid input_settings_grid(10, 3, main_settings_rect);
        UI input_settings_ui(-0.1, batcher.absolute_position_with_colored_vertex_shader_batcher.object_id_generator);
        input_settings_ui.add_textbox("mouse sensitivity", input_settings_grid.get_at(0, 0), colors::maroon);

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        input_settings_ui.add_input_box(on_confirm, "1", input_settings_grid.get_at(2, 0), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("fire", input_settings_grid.get_at(0, 1), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "lmb", input_settings_grid.get_at(2, 1), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("jump", input_settings_grid.get_at(0, 2), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "space", input_settings_grid.get_at(2, 2), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("move forward", input_settings_grid.get_at(0, 3), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "w", input_settings_grid.get_at(2, 3), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("move backward", input_settings_grid.get_at(0, 4), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "s", input_settings_grid.get_at(2, 4), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("move left", input_settings_grid.get_at(0, 5), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "a", input_settings_grid.get_at(2, 5), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("move right", input_settings_grid.get_at(0, 6), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "d", input_settings_grid.get_at(2, 6), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("select weapon 1", input_settings_grid.get_at(0, 7), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "1", input_settings_grid.get_at(2, 7), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("select weapon 2", input_settings_grid.get_at(0, 8), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "2", input_settings_grid.get_at(2, 8), colors::grey,
                                        colors::lightgrey);

        input_settings_ui.add_textbox("select weapon 3", input_settings_grid.get_at(0, 9), colors::maroon);
        input_settings_ui.add_input_box(on_confirm, "3", input_settings_grid.get_at(2, 9), colors::grey,
                                        colors::lightgrey);

        return input_settings_ui;
    }

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

    UI create_graphics_settings_ui() {

        std::vector<std::string> resolutions = get_available_resolutions("16:9");

        std::function<void(std::string)> on_confirm = [&](std::string contents) { std::cout << contents << std::endl; };

        vertex_geometry::Rectangle main_settings_rect = settings_menu.at(1);
        vertex_geometry::Grid main_settings_grid(7, 3, main_settings_rect);

        std::function<void()> on_click_settings = [&]() { curr_state = UIState::PLAYER_SETTINGS; };

        std::vector<std::string> yes_no_options = {"on", "off"};

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

        dropdown_option_idx = get_index_or_default(configuration.get_value("graphics", "resolution").value(), options);
        graphics_settings_ui.add_textbox("resolution", graphics_settings_grid.get_at(0, 0), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 0), colors::orange, colors::orangered,
                                          options, resolution_dropdown_on_click, dropdown_on_hover);

        std::function<void(std::string)> fullscreen_on_click = [this](std::string option) {
            sound_system.queue_sound(SoundType::UI_CLICK);
            configuration.set_value("graphics", "fullscreen", option);
        };

        dropdown_option_idx =
            get_index_or_default(configuration.get_value("graphics", "fullscreen").value(), yes_no_options);
        graphics_settings_ui.add_textbox("fullscreen", graphics_settings_grid.get_at(0, 1), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 1), colors::orange, colors::orangered,
                                          yes_no_options, fullscreen_on_click, dropdown_on_hover);

        std::vector<std::string> lighting_options = {"none", "early 2000s"};
        std::vector<std::function<void()>> lighting_option_on_clicks = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("lighting", graphics_settings_grid.get_at(0, 2), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 2), colors::orange, colors::orangered,
                                          lighting_options, empty_on_click, dropdown_on_hover);

        graphics_settings_ui.add_textbox("fov", graphics_settings_grid.get_at(0, 3), colors::maroon);
        graphics_settings_ui.add_input_box(on_confirm, "enter fov", graphics_settings_grid.get_at(2, 3), colors::grey,
                                           colors::lightgrey);

        std::vector<std::function<void()>> viewmodel_options_on_click = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("enable view model", graphics_settings_grid.get_at(0, 4), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 4), colors::orange, colors::orangered,
                                          yes_no_options, empty_on_click, dropdown_on_hover);

        std::vector<std::function<void()>> fps_options_on_click = {[]() {}, []() {}};
        graphics_settings_ui.add_textbox("show fps", graphics_settings_grid.get_at(0, 5), colors::maroon);
        graphics_settings_ui.add_dropdown(on_click_settings, on_hover, dropdown_option_idx,
                                          graphics_settings_grid.get_at(2, 5), colors::orange, colors::orangered,
                                          yes_no_options, empty_on_click, dropdown_on_hover);

        return graphics_settings_ui;
    }

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
