#include <MainController.hpp>
#include <cmath>
#include <engine/graphics/GraphicsController.hpp>
#include <string>
#include <vector>

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    platform->set_enable_cursor(false);

    camera->Position = glm::vec3(0.0f, 1.5f, 5.0f);
    camera->Yaw = -90.0f;
    camera->Pitch = -10.0f;
    camera->rotate_camera(0.0f, 0.0f);

    graphics->initialize_bloom(platform->window()->width(), platform->window()->height());
    graphics->initialize_point_shadows(1024, 1024);
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    float intensity_speed = 0.8f;
    float dt = platform->dt();

    if (platform->key(engine::platform::KeyId::KEY_UP).state() ==
        engine::platform::Key::State::Pressed) {
        point_light_intensity_ += intensity_speed * dt;
    }

    if (platform->key(engine::platform::KeyId::KEY_DOWN).state() ==
        engine::platform::Key::State::Pressed) {
        point_light_intensity_ -= intensity_speed * dt;
    }

    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).state() ==
        engine::platform::Key::State::JustPressed) {
        return false;
    }

    if (platform->key(engine::platform::KeyId::KEY_1).state() ==
        engine::platform::Key::State::JustPressed) {
        directional_light_enabled_ = !directional_light_enabled_;
    }

    if (platform->key(engine::platform::KeyId::KEY_2).state() ==
        engine::platform::Key::State::JustPressed) {
        point_light_enabled_ = !point_light_enabled_;
    }

    if (platform->key(engine::platform::KeyId::KEY_3).state() ==
        engine::platform::Key::State::JustPressed) {
        lamp_sway_enabled_ = !lamp_sway_enabled_;
    }

    if (platform->key(engine::platform::KeyId::KEY_G).state() ==
        engine::platform::Key::State::JustPressed) {
        lamp_event_state_ = LampEventState::Idle;
        point_light_color_ = glm::vec3(0.1f, 1.0f, 0.1f);
        point_light_intensity_ = 1.0f;
    }

    if (platform->key(engine::platform::KeyId::KEY_B).state() ==
        engine::platform::Key::State::JustPressed) {
        lamp_event_state_ = LampEventState::Idle;
        point_light_color_ = glm::vec3(0.1f, 0.3f, 1.0f);
        point_light_intensity_ = 1.0f;
    }

    if (platform->key(engine::platform::KeyId::KEY_Q).state() ==
        engine::platform::Key::State::JustPressed) {
        lamp_event_state_ = LampEventState::Idle;
        point_light_color_ = default_point_light_color_;
        point_light_intensity_ = 1.0f;
    }

    float current_time = platform->frame_time().current;

    if (platform->key(engine::platform::KeyId::KEY_SPACE).state() ==
        engine::platform::Key::State::JustPressed) {
        trigger_lamp_event(current_time);
    }

    point_light_intensity_ = glm::clamp(point_light_intensity_, 0.0f, 2.0f);

    return true;
}

void MainController::update() {
    update_camera();

    float current_time = engine::core::Controller::get<engine::platform::PlatformController>()->frame_time().current;

    update_lamp_event(current_time);
}

void MainController::draw_skybox() {
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("skybox");
    auto skybox_cube = engine::core::Controller::get<engine::resources::ResourcesController>()->skybox("skybox");
    engine::core::Controller::get<engine::graphics::GraphicsController>()->draw_skybox(shader, skybox_cube);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    float current_time = engine::core::Controller::get<engine::platform::PlatformController>()->frame_time().current;

    draw_point_shadow_depth(current_time);

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->begin_bloom_render();
    engine::graphics::OpenGL::clear_buffers();

    draw_scene_models(current_time);
    draw_light_bulb(current_time);
    draw_skybox();
}

void MainController::end_draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    auto blur_shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("blur");
    auto bloom_final_shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("bloom_final");

    graphics->end_bloom_render();
    graphics->draw_bloom_result(blur_shader, bloom_final_shader);
    engine::core::Controller::get<engine::platform::PlatformController>()->swap_buffers();
}

void MainController::draw_light_bulb(float current_time) {
    if (!point_light_enabled_) {
        return;
    }

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    auto shader = resources->shader("bulb");
    auto bulb = resources->model("bulb");
    auto camera = graphics->camera();

    shader->use();

    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    float aspect_ratio =
            static_cast<float>(platform->window()->width()) /
            static_cast<float>(platform->window()->height());

    glm::mat4 projection = glm::perspective(
            glm::radians(camera->Zoom),
            aspect_ratio,
            0.1f,
            100.0f);

    shader->set_mat4("projection", projection);
    shader->set_mat4("view", camera->view_matrix());
    shader->set_vec3("bulbColor", point_light_color_ * point_light_intensity_);

    glm::mat4 model = get_lamp_model_matrix(current_time);
    model = glm::translate(model, glm::vec3(0.0f, 8.8f, 0.0f));
    model = glm::scale(model, glm::vec3(0.6f));

    shader->set_mat4("model", model);

    bulb->draw(shader);
}

void MainController::update_camera() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt = platform->dt();

    if (platform->key(engine::platform::KEY_W).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KEY_S).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KEY_A).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KEY_D).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
    }
    auto mouse = platform->mouse();
    if (m_first_mouse) {
        m_first_mouse = false;
    } else {
        camera->rotate_camera(mouse.dx, mouse.dy);
        camera->zoom(mouse.scroll);
    }
}

void MainController::trigger_lamp_event(float current_time) {
    lamp_event_state_ = LampEventState::WaitBeforeFlicker;
    lamp_event_start_time_ = current_time;

    point_light_enabled_ = true;
    point_light_color_ = default_point_light_color_;
    point_light_intensity_ = 1.0f;
}

void MainController::update_lamp_event(float current_time) {
    switch (lamp_event_state_) {
        case LampEventState::Idle: break;

        case LampEventState::WaitBeforeFlicker:
            point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);
            point_light_intensity_ = 1.0f;

            if (current_time - lamp_event_start_time_ >= 2.0f) {
                lamp_event_state_ = LampEventState::Flicker;
                flicker_start_time_ = current_time;
            }
            break;

        case LampEventState::Flicker: {
            float elapsed = current_time - flicker_start_time_;
            float t = current_time;

            point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);

            float flicker =
                    0.2f * std::sin(t * 25.0f) +
                    0.15f * std::sin(t * 40.0f) +
                    0.10f * std::sin(t * 70.0f);

            if (std::sin(t * 10.0f) > 0.97f) {
                point_light_intensity_ = 0.2f;
            } else {
                point_light_intensity_ = 0.8f + flicker;
            }

            point_light_intensity_ = glm::clamp(point_light_intensity_, 0.1f, 1.2f);

            if (elapsed >= 2.0f) {
                lamp_event_state_ = LampEventState::RedLight;
                point_light_color_ = glm::vec3(1.0f, 0.1f, 0.05f);
                point_light_intensity_ = 1.0f;
            }
            break;
        }

        case LampEventState::RedLight:
            point_light_color_ = glm::vec3(1.0f, 0.1f, 0.05f);
            point_light_intensity_ = 1.0f;
            break;
    }
}

float MainController::get_lamp_sway_angle(float current_time) const {
    if (!lamp_sway_enabled_) {
        return 0.0f;
    }

    float amplitude = 12.0f;
    float speed = 1.8f;

    return amplitude * std::sin(current_time * speed);
}

glm::mat4 MainController::get_lamp_model_matrix(float current_time) const {
    glm::mat4 model(1.0f);

    model = glm::translate(model, glm::vec3(0.9f, 0.0f, -0.2f));

    float sway_angle = get_lamp_sway_angle(current_time);
    model = glm::rotate(model, glm::radians(sway_angle), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, glm::vec3(0.2f));

    return model;
}

glm::vec3 MainController::get_point_light_position(float current_time) const {
    glm::mat4 bulb_model = get_lamp_model_matrix(current_time);
    bulb_model = glm::translate(bulb_model, glm::vec3(0.0f, 8.8f, 0.0f));

    glm::vec4 world_pos = bulb_model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec3(world_pos);
}

void MainController::draw_point_shadow_depth(float current_time) {
    if (!point_light_enabled_) {
        return;
    }

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    glm::vec3 light_pos = get_point_light_position(current_time);

    auto shadow_shader = engine::core::Controller::get<engine::resources::ResourcesController>()
                                 ->shader("point_shadow_depth");

    // pripremi depth cubemap, izracunaj 6 shadow matrica i bind shadow framebuffer
    graphics->begin_point_shadow_render(light_pos, shadow_shader);

    // floor
    draw_model_depth(
            "floor",
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)),
            shadow_shader);

    // chair
    glm::mat4 chair_model = glm::mat4(1.0f);
    chair_model = glm::translate(chair_model, glm::vec3(0.0f, 0.0f, 0.35f));
    chair_model = glm::rotate(chair_model, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    chair_model = glm::scale(chair_model, glm::vec3(0.01f));
    draw_model_depth("chair", chair_model, shadow_shader);

    // Model lampe nije uključen u shadow pass jer se point light nalazi u njenoj geometriji.
    // Uključivanje lampe u shadow mapu dovodilo je do velikih nerealnih senki koje bi zaklanjale veći deo scene.

    // house
    glm::mat4 house_model = glm::mat4(1.0f);
    house_model = glm::translate(house_model, glm::vec3(0.5f, 0.0f, -3.7f));
    house_model = glm::rotate(house_model, glm::radians(-26.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    house_model = glm::scale(house_model, glm::vec3(0.15f));
    draw_model_depth("house", house_model, shadow_shader);

    // barrel1
    glm::mat4 barrel1_model = glm::mat4(1.0f);
    barrel1_model = glm::translate(barrel1_model, glm::vec3(1.2f, 0.0f, 0.2f));
    barrel1_model = glm::scale(barrel1_model, glm::vec3(0.5f));
    draw_model_depth("barrel1", barrel1_model, shadow_shader);

    // barrel2
    glm::mat4 barrel2_model = glm::mat4(1.0f);
    barrel2_model = glm::translate(barrel2_model, glm::vec3(0.6f, 0.0f, -0.5f));
    barrel2_model = glm::scale(barrel2_model, glm::vec3(0.5f));
    draw_model_depth("barrel2", barrel2_model, shadow_shader);

    graphics->end_point_shadow_render();
}

void MainController::draw_model_depth(const std::string &model_name, const glm::mat4 &model_matrix,
                                      const engine::resources::Shader *shader) {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    auto model = resources->model(model_name);

    shader->use();
    shader->set_mat4("model", model_matrix);

    model->draw(shader);
}

void MainController::draw_scene_models(float current_time) {
    struct SceneModel {
        const char *model_name;
        const char *shader_name;
        glm::mat4 transform;
        float material_shininess;
        float material_specular_strength;
    };

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera = graphics->camera();

    float aspect_ratio =
            static_cast<float>(platform->window()->width()) /
            static_cast<float>(platform->window()->height());

    glm::mat4 projection = glm::perspective(
            glm::radians(camera->Zoom),
            aspect_ratio,
            0.1f,
            100.0f);

    glm::mat4 view = camera->view_matrix();

    glm::vec3 light_pos = get_point_light_position(current_time);

    glm::vec3 light_color;
    if (point_light_enabled_) {
        light_color = point_light_color_ * point_light_intensity_;
    } else {
        light_color = glm::vec3(0.0f);
    }

    glm::vec3 directional_light_color;
    if (directional_light_enabled_) {
        directional_light_color = glm::vec3(0.12f, 0.12f, 0.15f);
    } else {
        directional_light_color = glm::vec3(0.0f);
    }

    glm::mat4 floor_model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

    glm::mat4 chair_model = glm::mat4(1.0f);
    chair_model = glm::translate(chair_model, glm::vec3(0.0f, 0.0f, 0.35f));
    chair_model = glm::rotate(chair_model, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    chair_model = glm::scale(chair_model, glm::vec3(0.01f));

    glm::mat4 lamp_model = get_lamp_model_matrix(current_time);

    glm::mat4 house_model = glm::mat4(1.0f);
    house_model = glm::translate(house_model, glm::vec3(0.5f, 0.0f, -3.7f));
    house_model = glm::rotate(house_model, glm::radians(-26.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    house_model = glm::scale(house_model, glm::vec3(0.15f));

    glm::mat4 barrel1_model = glm::mat4(1.0f);
    barrel1_model = glm::translate(barrel1_model, glm::vec3(1.2f, 0.0f, 0.2f));
    barrel1_model = glm::scale(barrel1_model, glm::vec3(0.5f));

    glm::mat4 barrel2_model = glm::mat4(1.0f);
    barrel2_model = glm::translate(barrel2_model, glm::vec3(0.6f, 0.0f, -0.5f));
    barrel2_model = glm::scale(barrel2_model, glm::vec3(0.5f));

    std::vector<SceneModel> scene_models = {
            {"floor", "lighting", floor_model, 10.0f, 0.12f},
            {"chair", "lighting", chair_model, 16.0f, 0.22f},
            {"lamp", "lighting", lamp_model, 32.0f, 0.35f},
            {"house", "lighting", house_model, 8.0f, 0.05f},
            {"barrel1", "lighting1", barrel1_model, 32.0f, 0.60f},
            {"barrel2", "lighting1", barrel2_model, 32.0f, 0.60f}};

    for (const SceneModel &scene_model: scene_models) {
        auto shader = resources->shader(scene_model.shader_name);
        auto model = resources->model(scene_model.model_name);

        shader->use();

        shader->set_bool("shadows", point_light_enabled_);
        shader->set_float("far_plane", graphics->point_shadow_far_plane());

        graphics->bind_point_shadow_depth_map(3);
        shader->set_int("depthMap", 3);

        shader->set_vec3("viewPos", camera->Position);
        shader->set_vec3("lightPos", light_pos);
        shader->set_vec3("lightColor", light_color);

        shader->set_vec3("dirLightDir", glm::vec3(-0.35f, -1.0f, 0.15f));
        shader->set_vec3("dirLightColor", directional_light_color);

        shader->set_float("materialShininess", scene_model.material_shininess);
        shader->set_float("materialSpecularStrength", scene_model.material_specular_strength);

        shader->set_mat4("projection", projection);
        shader->set_mat4("view", view);
        shader->set_mat4("model", scene_model.transform);

        model->draw(shader);
    }
}
