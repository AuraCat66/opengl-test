#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>

#include "glbinding/glbinding.h"
#include "glbinding/gl33core/gl.h"

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/debug.h>

using namespace std;
using namespace gl33core;
using namespace glbinding;

constexpr uint32_t windowStartWidth = 800;
constexpr uint32_t windowStartHeight = 600;

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

struct AppContext {
    SDL_Window* window;
    SDL_AudioDeviceID audioDevice;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

void framebuffer_resize(SDL_Window *window) {
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    glViewport(0, 0, width, height);
}

SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return SDL_Fail();
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL Test",
        windowStartWidth,
        windowStartHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (not window)
        return SDL_Fail();

    // From this point this is OpenGL stuff

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (not context)
        return SDL_Fail();

    // Load OpenGL functions
    glbinding::initialize(SDL_GL_GetProcAddress);
    aux::enableGetErrorCallback();

    SDL_Log("OpenGL version: %s", aux::ContextInfo::version().toString().c_str());
    SDL_Log("OpenGL vendor: %s", aux::ContextInfo::vendor().c_str());
    SDL_Log("OpenGL renderer: %s", aux::ContextInfo::renderer().c_str());

    if (not SDL_GL_SetSwapInterval(1)) {
        return SDL_Fail();
    }
    SDL_Log("VSync activated");

    // It's maybe useless we'll see
    framebuffer_resize(window);
    // We now make the window visible
    SDL_ShowWindow(window);

    // Color used when clearing the framebuffer
    glClearColor(0.3f, 0.4f, 0.7f, 1.0f);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // We create the shader (shader = GPU program)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // We attach it to the vertex source code
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // We compile the shader
    glCompileShader(vertexShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        SDL_LogError(0,"Vertex shader error: compilation failed!\n%s", infoLog);
        return SDL_APP_FAILURE;
    }
    SDL_Log("Vertex shader successfully compiled");

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

void processInput(AppContext* app, const SDL_Event* event) {
    if (event->key.key == SDLK_ESCAPE) {
        app->app_quit = SDL_APP_SUCCESS;
    }
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* app = (AppContext*)appstate;

    switch (event->type) {
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
            processInput(app, event);
        case SDL_EVENT_WINDOW_RESIZED:
            int width, height;
            SDL_GetWindowSizeInPixels(app->window, &width, &height);
            framebuffer_resize(app->window);
            SDL_Log("Window resized: %ix%i", width, height);
            break;
        case SDL_EVENT_QUIT:
            app->app_quit = SDL_APP_SUCCESS;
            break;
    }

    return app->app_quit;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    const auto* app = (AppContext*)appstate;

    glClear(ClearBufferMask::GL_COLOR_BUFFER_BIT);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    SDL_GL_SwapWindow(app->window);
    return app->app_quit;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    const auto* app = (AppContext*)appstate;
    if (app)
    {
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Log("Application quit successfully!");
    SDL_Quit();
}