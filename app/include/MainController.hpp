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
};

#endif
