#pragma once

#ifndef OPENGL_TEST_HELPERFUNCTIONS_H
#define OPENGL_TEST_HELPERFUNCTIONS_H

#include "SDL3/SDL.h"

inline SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

#endif //OPENGL_TEST_HELPERFUNCTIONS_H
