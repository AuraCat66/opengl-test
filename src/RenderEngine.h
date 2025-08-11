#pragma once

#ifndef OPENGL_TEST_RENDERENGINE_H
#define OPENGL_TEST_RENDERENGINE_H

#include <SDL3/SDL.h>

struct AppContext;
inline float vertices[] = {
    0.5f, 0.5f, 0.0f, // top right
    0.5f, -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f, 0.5f, 0.0f // top left
};
inline unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3 // second triangle
};

inline const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
inline const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

class RenderEngine {
public:
    SDL_Window *window{};

    bool wireframe = false;
    unsigned int VAO{};
    unsigned int shaderProgram{};

    void viewport_resize() const;

    static void setAttributes();

    SDL_Window *createWindow(
        const char *title,
        int w, int h,
        SDL_WindowFlags flags
    );

    SDL_AppResult init();

    SDL_AppResult render(const AppContext *app) const;
};


#endif //OPENGL_TEST_RENDERENGINE_H
