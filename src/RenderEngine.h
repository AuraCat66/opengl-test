#pragma once

#ifndef OPENGL_TEST_RENDERENGINE_H
#define OPENGL_TEST_RENDERENGINE_H

#include "SDL3/SDL.h"

#include "Shader.h"

struct AppContext;

class RenderEngine {
public:
    Shader shader;
    SDL_Window *window{};

    bool wireframe = false;
    unsigned int VAO{};
    unsigned int texture{};

    void viewport_resize() const;

    static SDL_AppResult setAttributes();

    SDL_Window *createWindow(
        const char *title,
        int w, int h,
        SDL_WindowFlags flags
    );

    SDL_AppResult init();

    SDL_AppResult render(const AppContext *app) const;
};


#endif //OPENGL_TEST_RENDERENGINE_H
