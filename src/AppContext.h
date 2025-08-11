#pragma once

#ifndef OPENGL_TEST_APPCONTEXT_H
#define OPENGL_TEST_APPCONTEXT_H
#include "RenderEngine.h"

struct AppContext {
public:
    RenderEngine renderer;
    SDL_AppResult controlFlow = SDL_APP_CONTINUE;
};

#endif //OPENGL_TEST_APPCONTEXT_H
