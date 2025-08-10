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

// #include "RenderEngine.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

constexpr uint32_t windowStartWidth = 800;
constexpr uint32_t windowStartHeight = 600;

float vertices[] = {
    0.5f, 0.5f, 0.0f, // top right
    0.5f, -0.5f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f, 0.5f, 0.0f // top left
};
unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3 // second triangle
};

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

bool wireframe = false;

struct AppContext {
    SDL_Window *window;
    SDL_AudioDeviceID audioDevice;
    unsigned int shaderProgram;
    unsigned int VAO;
    SDL_AppResult controlFlow = SDL_APP_CONTINUE;
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

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    SDL_Window *window = SDL_CreateWindow(
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Shader stuff

    // We create the vertex shader (shader = GPU program)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // We attach it to the vertex shader source code
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // We compile the shader
    glCompileShader(vertexShader);

    // Now we do the same but for the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // We just verify if everything went well
    {
        int success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            SDL_LogError(0, "Vertex shader error: compilation failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("Vertex shader successfully compiled");

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            SDL_LogError(0, "Fragment shader error: compilation failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("Fragment shader successfully compiled");
    }

    // We can then create the shader program and link the shader objects to it
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    {
        int success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            SDL_LogError(0, "Shader program error: link failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("Shader objects successfully linked to shader program");
    }

    // Poor OpenGL doesn't know how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // We delete the leftover shader objects (I'm sorry for them)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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
        .window = window,
        .shaderProgram = shaderProgram,
        .VAO = VAO,
        .controlFlow = SDL_APP_CONTINUE,
    };

    SDL_Log("App initialized successfully!");
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
            SDL_GetWindowSizeInPixels(app->window, &width, &height);
            framebuffer_resize(app->window);
            SDL_Log("Window resized: %ix%i", width, height);
            break;
        case SDL_EVENT_QUIT:
            app->controlFlow = SDL_APP_SUCCESS;
            break;
    }

    return app->controlFlow;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    const auto *app = (AppContext *) appstate;

    glClear(ClearBufferMask::GL_COLOR_BUFFER_BIT);

    glUseProgram(app->shaderProgram);
    glBindVertexArray(app->VAO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    SDL_GL_SwapWindow(app->window);

    return app->controlFlow;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    const auto *app = (AppContext *) appstate;
    if (app) {
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Log("Application quit successfully!");
    SDL_Quit();
}
