
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Skybox.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace engine::graphics {
void GraphicsController::initialize() {
    const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto handle = platform->window()->handle_();
    m_perspective_params.FOV = glm::radians(m_camera.Zoom);
    m_perspective_params.Width = static_cast<float>(platform->window()->width());
    m_perspective_params.Height = static_cast<float>(platform->window()->height());
    m_perspective_params.Near = 0.1f;
    m_perspective_params.Far = 100.f;
    m_ortho_params.Bottom = 0.0f;
    m_ortho_params.Top = static_cast<float>(platform->window()->height());
    m_ortho_params.Left = 0.0f;
    m_ortho_params.Right = static_cast<float>(platform->window()->width());
    m_ortho_params.Near = 0.1f;
    m_ortho_params.Far = 100.0f;

    platform->register_platform_event_observer(std::make_unique<GraphicsPlatformEventObserver>(this));
    CHECKED_GL_CALL(glViewport, 0, 0, platform->window()->width(), platform->window()->height());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
    RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params().Width = static_cast<float>(width);
    m_graphics->perspective_params().Height = static_cast<float>(height);
    m_graphics->orthographic_params().Right = static_cast<float>(width);
    m_graphics->orthographic_params().Top = static_cast<float>(height);
    CHECKED_GL_CALL(glViewport, 0, 0, width, height);
}

std::string_view GraphicsController::name() const {
    return "GraphicsController";
}

void GraphicsController::begin_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicsController::end_gui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
    CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    CHECKED_GL_CALL(glDepthFunc, GL_LESS);// set depth function back to default
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}

// bloom

void GraphicsController::initialize_bloom(int width, int height) {
    bloom_width_ = width;
    bloom_height_ = height;

    // HDR framebuffer
    glGenFramebuffers(1, &bloom_hdr_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_hdr_fbo_);

    // 2 color buffers:
    // [0] cela scena
    // [1] bright delovi scene
    glGenTextures(2, bloom_color_buffers_);

    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, bloom_color_buffers_[i]);

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA16F,
                bloom_width_,
                bloom_height_,
                0,
                GL_RGBA,
                GL_FLOAT,
                nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + i,
                GL_TEXTURE_2D,
                bloom_color_buffers_[i],
                0);
    }

    // Depth buffer za HDR framebuffer
    glGenRenderbuffers(1, &bloom_depth_rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, bloom_depth_rbo_);

    glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT,
            bloom_width_,
            bloom_height_);

    glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            bloom_depth_rbo_);

    unsigned int attachments[2] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1};

    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Bloom HDR framebuffer is not complete");
    }

    // Ping-pong framebuffer-i za Gaussian blur
    glGenFramebuffers(2, bloom_pingpong_fbo_);
    glGenTextures(2, bloom_pingpong_colorbuffers_);

    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_pingpong_fbo_[i]);

        glBindTexture(GL_TEXTURE_2D, bloom_pingpong_colorbuffers_[i]);

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA16F,
                bloom_width_,
                bloom_height_,
                0,
                GL_RGBA,
                GL_FLOAT,
                nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                bloom_pingpong_colorbuffers_[i],
                0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Bloom ping-pong framebuffer is not complete");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bloom_initialized_ = true;
}

void GraphicsController::begin_bloom_render() {
    if (!bloom_initialized_)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, bloom_hdr_fbo_);
    glViewport(0, 0, bloom_width_, bloom_height_);
}

void GraphicsController::end_bloom_render() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsController::render_screen_quad() {
    if (bloom_quad_vao_ == 0) {
        float quadVertices[] = {
                // positions   // texCoords
                -1.0f, 1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,

                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f};

        glGenVertexArrays(1, &bloom_quad_vao_);
        glGenBuffers(1, &bloom_quad_vbo_);

        glBindVertexArray(bloom_quad_vao_);

        glBindBuffer(GL_ARRAY_BUFFER, bloom_quad_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
                0,
                2,
                GL_FLOAT,
                GL_FALSE,
                4 * sizeof(float),
                (void *) 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
                1,
                2,
                GL_FLOAT,
                GL_FALSE,
                4 * sizeof(float),
                (void *) (2 * sizeof(float)));
    }

    glBindVertexArray(bloom_quad_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GraphicsController::draw_bloom_result(const resources::Shader *blur_shader,
                                           const resources::Shader *final_shader) {
    bool horizontal = true;
    bool first_iteration = true;

    blur_shader->use();
    blur_shader->set_int("image", 0);

    for (int i = 0; i < bloom_blur_amount_; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_pingpong_fbo_[horizontal]);

        blur_shader->set_bool("horizontal", horizontal);

        glActiveTexture(GL_TEXTURE0);

        if (first_iteration) {
            glBindTexture(GL_TEXTURE_2D, bloom_color_buffers_[1]);
        } else {
            glBindTexture(GL_TEXTURE_2D, bloom_pingpong_colorbuffers_[!horizontal]);
        }

        render_screen_quad();

        horizontal = !horizontal;

        if (first_iteration) {
            first_iteration = false;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    final_shader->use();

    final_shader->set_int("scene", 0);
    final_shader->set_int("bloomBlur", 1);
    final_shader->set_bool("bloom", true);
    final_shader->set_float("exposure", 0.35f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bloom_color_buffers_[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_pingpong_colorbuffers_[!horizontal]);

    render_screen_quad();
}

// end bloom

// point shadows
unsigned int GraphicsController::point_shadow_depth_cubemap() const {
    return point_shadow_depth_cubemap_;
}

float GraphicsController::point_shadow_far_plane() const {
    return point_shadow_far_plane_;
}

void GraphicsController::initialize_point_shadows(unsigned int shadow_width, unsigned int shadow_height) {
    point_shadow_width_ = shadow_width;
    point_shadow_height_ = shadow_height;

    glGenFramebuffers(1, &point_shadow_fbo_);

    glGenTextures(1, &point_shadow_depth_cubemap_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_cubemap_);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_DEPTH_COMPONENT,
                point_shadow_width_,
                point_shadow_height_,
                0,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo_);
    glFramebufferTexture(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            point_shadow_depth_cubemap_,
            0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Point shadow framebuffer is not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsController::begin_point_shadow_render(const glm::vec3 &light_pos,
                                                   const resources::Shader *shadow_shader) {
    point_shadow_light_pos_ = light_pos;

    glm::mat4 shadow_projection = glm::perspective(
            glm::radians(90.0f),
            static_cast<float>(point_shadow_width_) / static_cast<float>(point_shadow_height_),
            point_shadow_near_plane_,
            point_shadow_far_plane_);

    point_shadow_matrices_[0] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f));

    point_shadow_matrices_[1] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(-1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f));

    point_shadow_matrices_[2] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(0.0f, 1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, 1.0f));

    point_shadow_matrices_[3] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(0.0f, -1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, -1.0f));

    point_shadow_matrices_[4] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(0.0f, 0.0f, 1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f));

    point_shadow_matrices_[5] = shadow_projection * glm::lookAt(
                                                            light_pos,
                                                            light_pos + glm::vec3(0.0f, 0.0f, -1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f));

    shadow_shader->use();

    for (unsigned int i = 0; i < 6; ++i) {
        shadow_shader->set_mat4(
                "shadowMatrices[" + std::to_string(i) + "]",
                point_shadow_matrices_[i]);
    }

    shadow_shader->set_vec3("lightPos", light_pos);
    shadow_shader->set_float("far_plane", point_shadow_far_plane_);

    glViewport(0, 0, point_shadow_width_, point_shadow_height_);
    glBindFramebuffer(GL_FRAMEBUFFER, point_shadow_fbo_);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GraphicsController::end_point_shadow_render() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    glViewport(
            0,
            0,
            platform->window()->width(),
            platform->window()->height());
}

void GraphicsController::bind_point_shadow_depth_map(unsigned int texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, point_shadow_depth_cubemap_);
}

// end point shadows
}// namespace engine::graphics
