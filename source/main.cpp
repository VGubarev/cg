#include "Object.h"
#include "Camera.h"
#include "pipeline.h"
#include "ShadowMapEffect.h"
#include "ShadowMapFBO.h"

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

int main(void)
{
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window;
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1920, 1080, "Tutorial 07 - Model Loading", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1920 / 2, 1080 / 2);

    // Dark black background
    glClearColor(0.376f, 0.235f, 0.149f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glCullFace(GL_CCW);

    auto camera = Camera::get_camera(window);

    /*
     * Object stuff
     */
    Texture home_texture(GL_TEXTURE_2D, "models/basic_home/tex_woodlands_main.jpg");
    Object home("models/basic_home/cabin01.obj", home_texture, 0);
    home.init();

    /*
     * Light stuff
     */
    DirectionalLight light_direction;
    light_direction.color = glm::vec3(1.0f, 1.0f, 1.0f);
    light_direction.ambient_intensity = 0.0f;
    light_direction.diffuse_intensity = 0.0f;
    light_direction.direction = glm::vec3(0.5f, 0.0f, 0.5f);

    std::vector<PointLight> pl;
    PointLight p;
    pl.emplace_back(p);
    pl[0].diffuse_intensity = 1.0f;
    pl[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    pl[0].attentuation.linear = 0.1f;
    pl[0].position = glm::vec3(-20.0f, 5.0f, -20.0f);

    /*
     * Shadow stuff
     */
    ShadowMapFBO shadow_map_fbo;
    shadow_map_fbo.init(1920, 1080);
    ShadowMapEffect shadow_map;
    shadow_map.init();

    do {
        Pipeline p;
        p.world_position(0.0f, 0.0f, 0.0f);
        p.set_camera(pl[0].position, glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        p.set_perspective(45.0f, 1920, 1080, 0.1f, 500.0f);

        /* 1 step - make shadow */
        shadow_map_fbo.write_bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        shadow_map.enable();

        shadow_map.set_wvp(p.get_wvp());
        home.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* 2 step - make shadow */

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	home.enable();
        shadow_map.set_texture_unit(1);
        shadow_map_fbo.read_bind(GL_TEXTURE0 + 1);

        const auto & position = camera->get_position();
        const auto & target = camera->get_target();
        const auto & up = camera->get_up();

        p.set_camera(position, target, up);
        home.set_wvp(p.get_wvp());
        home.set_world_matrix(p.get_world());
   	home.set_camera_position(position);
	home.set_directional_light(light_direction);
        home.set_specular_intensity(1.0f);
        home.set_specular_power(4.0f);
        home.set_point_lights(pl);
        home.render();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
