#include <iostream>
#include <GLES2/gl2.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>

#include "glbinding/glbinding.h"
#include "glbinding/gl/gl.h"

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/debug.h>

using namespace std;
using namespace gl;
using namespace glbinding;

constexpr uint32_t windowStartWidth = 400;
constexpr uint32_t windowStartHeight = 400;

struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* messageTex, * imageTex;
    SDL_FRect messageDest;
    SDL_AudioDeviceID audioDevice;
    // MIX_Track* track;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return SDL_Fail();
    }

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL Game",
        windowStartWidth,
        windowStartHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (not window)
        return SDL_Fail();

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (not context)
        return SDL_Fail();

    glbinding::initialize(SDL_GL_GetProcAddress);
    SDL_Log("OpenGL version: %s", aux::ContextInfo::version().toString().c_str());
    SDL_Log("OpenGL vendor: %s", aux::ContextInfo::vendor().c_str());
    SDL_Log("OpenGL renderer: %s", aux::ContextInfo::renderer().c_str());

    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth)
        {
            SDL_Log("This is a highdpi environment.");
        }
    }

    *appstate = new AppContext{
        .window = window,
        .app_quit = SDL_APP_CONTINUE,
    };

    SDL_Log("App initialized successfully!");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto* app = (AppContext*)appstate;

    if (event->type == SDL_EVENT_QUIT)
    {
        app->app_quit = SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* app = (AppContext*)appstate;
    // If app->app_quit is SDL_APP_SUCCESS or FAILURE
    // we want the program to quit as soon as possible
    // This function is the best place as it runs every single frame.
    return app->app_quit;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* app = (AppContext*)appstate;
    if (app)
    {
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Log("Application quit successfully!");
    SDL_Quit();
}