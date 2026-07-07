#include <MainController.hpp>
#include <MyApp.hpp>

void MyApp::app_setup() {
    auto controller = register_controller<MainController>();

    controller->after(
            engine::core::Controller::get<engine::core::EngineControllersEnd>());
}
