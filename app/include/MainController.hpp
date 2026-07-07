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

    void draw_light_bulb(float current_time);

    void draw_skybox();

    void update_camera();

    void trigger_lamp_event(float current_time);

    void update_lamp_event(float current_time);

    float get_lamp_sway_angle(float current_time) const;

    void draw_point_shadow_depth(float current_time);

    void draw_model_depth(const std::string &model_name, const glm::mat4 &model_matrix,
                          const engine::resources::Shader *shader);

    glm::mat4 get_lamp_model_matrix(float current_time) const;

    glm::vec3 get_point_light_position(float current_time) const;

    void draw_scene_models(float current_time);

    bool m_first_mouse{true};

    bool directional_light_enabled_ = true;

    bool point_light_enabled_ = true;

    bool lamp_sway_enabled_ = false;

    enum class LampEventState {
        Idle,
        WaitBeforeFlicker,
        Flicker,
        RedLight
    };

    LampEventState lamp_event_state_ = LampEventState::Idle;

    float lamp_event_start_time_ = 0.0f;
    float flicker_start_time_    = 0.0f;

    glm::vec3 point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);
    float point_light_intensity_ = 1.0f;

    glm::vec3 default_point_light_color_ = glm::vec3(1.0f, 0.75f, 0.4f);

    struct SceneModel {
        const char *model_name;
        const char *shader_name;
        glm::mat4 transform;
        float material_shininess;
        float material_specular_strength;
        bool animated;
    };

    std::vector<SceneModel> scene_models_;

    void initialize_scene_models();
};

#endif
