#include <MainController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera   = graphics->camera();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    platform->set_enable_cursor(false);

    camera->Position = glm::vec3(0.0f, 1.5f, 5.0f);
    camera->Yaw      = -90.0f;
    camera->Pitch    = -10.0f;
    camera->rotate_camera(0.0f, 0.0f);
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).state() ==
        engine::platform::Key::State::JustPressed) {
        return false;
    }

    float current_time = static_cast<float>(glfwGetTime());

    if (platform->key(engine::platform::KeyId::KEY_SPACE).state() ==
        engine::platform::Key::State::JustPressed) {
        trigger_lamp_event(current_time);
    }

    update_lamp_event(current_time);

    return true;
}

void MainController::draw_floor() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader   = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("lighting");
    auto floor    = engine::core::Controller::get<engine::resources::ResourcesController>()->model("floor");
    shader->use();

    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    shader->set_vec3("viewPos", camera->Position);

    // izracunao sam da se koord light bulba nalaze na (1.5, 1.76, 1.0)
    // a to su koordinate za point light
    shader->set_vec3("lightPos", glm::vec3(1.5f, 1.76f, 1.0f));
    shader->set_vec3("lightColor", point_light_color_ * point_light_intensity_);

    shader->set_vec3("dirLightDir", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->set_vec3("dirLightColor", glm::vec3(0.15f, 0.15f, 0.2f));

    shader->set_float("materialShininess", 10.0f);
    shader->set_float("materialSpecularStrength", 0.12f);

    // dodao sam svoju projection matrix jer mi zoom ne radi sa graphics->projection_matrix()
    glm::mat4 projection = glm::perspective(
        glm::radians(camera->Zoom),
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );

    shader->set_mat4("projection", projection);
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)));
    floor->draw(shader);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    draw_floor();
    draw_chair();
    draw_lamp();
    draw_light_bulb();
}

void MainController::end_draw() {
    engine::core::Controller::get<engine::platform::PlatformController>()->swap_buffers();
}

void MainController::draw_chair() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader   = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("lighting");
    auto chair    = engine::core::Controller::get<engine::resources::ResourcesController>()->model("chair");
    shader->use();

    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();

    shader->set_vec3("viewPos", camera->Position);

    // izracunao sam da se koord light bulba nalaze na (1.5, 1.76, 1.0)
    shader->set_vec3("lightPos", glm::vec3(1.5f, 1.76f, 1.0f));
    shader->set_vec3("lightColor", point_light_color_ * point_light_intensity_);

    shader->set_vec3("dirLightDir", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->set_vec3("dirLightColor", glm::vec3(0.15f, 0.15f, 0.2f));

    shader->set_float("materialShininess", 20.0f);
    shader->set_float("materialSpecularStrength", 0.22f);

    // dodao sam svoju projection matrix jer mi zoom ne radi sa graphics->projection_matrix()
    glm::mat4 projection = glm::perspective(
        glm::radians(camera->Zoom),
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );

    shader->set_mat4("projection", projection);
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model           = glm::scale(model, glm::vec3(1.0f));
    shader->set_mat4("model", model);

    chair->draw(shader);
}

void MainController::draw_lamp() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader   = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("lighting");
    auto lamp     = engine::core::Controller::get<engine::resources::ResourcesController>()->model("lamp");
    shader->use();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();

    shader->set_vec3("viewPos", camera->Position);

    // izracunao sam da se koord light bulba nalaze na (1.5, 1.76, 1.0)
    shader->set_vec3("lightPos", glm::vec3(1.5f, 1.76f, 1.0f));
    shader->set_vec3("lightColor", point_light_color_ * point_light_intensity_);

    shader->set_vec3("dirLightDir", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->set_vec3("dirLightColor", glm::vec3(0.15f, 0.15f, 0.2f));

    shader->set_float("materialShininess", 32.0f);
    shader->set_float("materialSpecularStrength", 0.35f);

    // dodao sam svoju projection matrix jer mi zoom ne radi sa graphics->projection_matrix()
    glm::mat4 projection = glm::perspective(
        glm::radians(camera->Zoom),
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );

    shader->set_mat4("projection", projection);
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, glm::vec3(1.5f, 0.0f, 1.0f));;
    model           = glm::scale(model, glm::vec3(0.2f));
    shader->set_mat4("model", model);

    lamp->draw(shader);
}

void MainController::draw_light_bulb() {
    auto graphics  = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    auto shader = resources->shader("bulb");
    auto bulb   = resources->model("bulb");
    auto camera = graphics->camera();

    shader->use();

    glm::mat4 projection = glm::perspective(
        glm::radians(camera->Zoom),
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );

    shader->set_mat4("projection", projection);
    shader->set_mat4("view", camera->view_matrix());
    shader->set_vec3("bulbColor", point_light_color_ * point_light_intensity_);

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.2f));
    model = glm::translate(model, glm::vec3(0.0f, 8.8f, 0.0f));
    model = glm::scale(model, glm::vec3(0.6f));

    shader->set_mat4("model", model);

    bulb->draw(shader);
}

void MainController::update() {
    update_camera();
}

void MainController::update_camera() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera   = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt      = platform->dt();

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
    lamp_event_state_      = LampEventState::WaitBeforeFlicker;
    lamp_event_start_time_ = current_time;

    point_light_color_     = glm::vec3(1.0f, 0.75f, 0.4f);
    point_light_intensity_ = 1.0f;
}

void MainController::update_lamp_event(float current_time) {
    switch (lamp_event_state_) {
    case LampEventState::Idle: point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);
        point_light_intensity_ = 1.0f;
        break;

    case LampEventState::WaitBeforeFlicker: point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);
        point_light_intensity_ = 1.0f;

        if (current_time - lamp_event_start_time_ >= 2.0f) {
            lamp_event_state_   = LampEventState::Flicker;
            flicker_start_time_ = current_time;
        }
        break;

    case LampEventState::Flicker: {
        float elapsed = current_time - flicker_start_time_;
        float t       = current_time;

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
            lamp_event_state_      = LampEventState::RedLight;
            point_light_color_     = glm::vec3(1.0f, 0.1f, 0.05f);
            point_light_intensity_ = 1.0f;
        }
        break;
    }

    case LampEventState::RedLight: point_light_color_ = glm::vec3(1.0f, 0.1f, 0.05f);
        point_light_intensity_ = 1.0f;
        break;
    }
}
