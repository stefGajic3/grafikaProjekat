#include <MainController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera   = graphics->camera();
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    platform->set_enable_cursor(false);

    camera->Position = glm::vec3(0.0f, 1.5f, 7.0f);
    camera->Yaw      = -90.0f;
    camera->Pitch    = -10.0f;
    camera->rotate_camera(0.0f, 0.0f);
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).state() == engine::platform::Key::State::JustPressed) {
        return false;
    }
    return true;
}

void MainController::draw_floor() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader   = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("floor");
    auto floor    = engine::core::Controller::get<engine::resources::ResourcesController>()->model("floor");
    shader->use();

    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();

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
}

void MainController::end_draw() {
    engine::core::Controller::get<engine::platform::PlatformController>()->swap_buffers();
}

void MainController::update() {
    update_camera();
}

void MainController::update_camera() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera   = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt      = platform->dt();

    if (platform->key(engine::platform::KEY_W).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt * 1.5);
    }
    if (platform->key(engine::platform::KEY_S).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt * 1.5);
    }
    if (platform->key(engine::platform::KEY_A).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt * 1.5);
    }
    if (platform->key(engine::platform::KEY_D).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt * 1.5);
    }
    auto mouse = platform->mouse();
    if (m_first_mouse) {
        m_first_mouse = false;
    } else {
        camera->rotate_camera(mouse.dx, mouse.dy);
        camera->zoom(mouse.scroll);
    }
}
