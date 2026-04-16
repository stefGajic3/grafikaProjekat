//
// Created by stefan on 4/6/26.
//

#ifndef GRAFIKA_PROJEKAT_MAINCONTROLLER_HPP
#define GRAFIKA_PROJEKAT_MAINCONTROLLER_HPP

#include <engine/core/Engine.hpp>

class MainController final : public engine::core::Controller {
private:
    void initialize() override;

    bool loop() override;

    void update() override;

    void begin_draw() override;

    void draw() override;

    void end_draw() override;

    void draw_floor();

    void draw_chair();

    void update_camera();

    bool m_first_mouse{true};
};

#endif
