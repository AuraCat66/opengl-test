#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>

#include <glbinding/glbinding.h>
#include <glbinding/gl33core/gl.h>

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/debug.h>

#include "AppContext.h"
#include "RenderEngine.h"
#include "helperFunctions.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

constexpr uint32_t windowStartWidth = 800;
constexpr uint32_t windowStartHeight = 600;

bool wireframe = false;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }

    RenderEngine renderer;

    if (const auto appResult = RenderEngine::setAttributes(); appResult == SDL_APP_FAILURE) {
        return SDL_APP_FAILURE;
    }

    auto *window = renderer.createWindow(
        "OpenGL Test",
        windowStartWidth, windowStartHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (not window) {
        return SDL_Fail();
    }

    if (const auto appResult = renderer.init(); appResult == SDL_APP_FAILURE) {
        return SDL_APP_FAILURE;
    }

    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    *appstate = new AppContext{
        .renderer = renderer,
        .controlFlow = SDL_APP_CONTINUE,
    };

    SDL_Log("Application initialized successfully!");
    return SDL_APP_CONTINUE;
}

void processInput(AppContext *app, const SDL_Event *event) {
    if (event->key.key == SDLK_ESCAPE) {
        app->controlFlow = SDL_APP_SUCCESS;
    }
    if (event->key.key == SDLK_A && event->key.down) {
        if (wireframe == false) {
            wireframe = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            wireframe = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto *app = (AppContext *) appstate;

    switch (event->type) {
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
            processInput(app, event);
        case SDL_EVENT_WINDOW_RESIZED:
            int width, height;
            SDL_GetWindowSizeInPixels(app->renderer.window, &width, &height);
            app->renderer.viewport_resize();
            SDL_Log("Window resized: %ix%i", width, height);
            break;
        case SDL_EVENT_QUIT:
            app->controlFlow = SDL_APP_SUCCESS;
            break;
        default: break;
    }

    return app->controlFlow;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    const auto *app = (AppContext *) appstate;

    return app->renderer.render(app);
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (const auto *app = (AppContext *) appstate) {
        SDL_DestroyWindow(app->renderer.window);
        delete app;
    }

    SDL_Log("Application quit successfully!");
    SDL_Quit();
}
