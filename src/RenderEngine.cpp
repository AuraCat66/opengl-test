#include "RenderEngine.h"

#include <cmath>

#include "glbinding/glbinding.h"
#include "glbinding/gl33core/gl.h"

#include "glbinding-aux/ContextInfo.h"
#include "glbinding-aux/Meta.h"
#include "glbinding-aux/types_to_string.h"
#include "glbinding-aux/ValidVersions.h"
#include "glbinding-aux/debug.h"

#include "AppContext.h"
#include "helperFunctions.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

float vertices[] = {
    // positions         // colors
    0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
   -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
    0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 ourColor;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "   ourColor = aColor;\n"
        "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main() {\n"
        "   FragColor = vec4(ourColor, 1.0);\n"
        "}\0";

void RenderEngine::viewport_resize() const {
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    glViewport(0, 0, width, height);
}

SDL_AppResult RenderEngine::setAttributes() {
    if (not SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)
        || not SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)
        || not SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)
        || not SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)) {
        return SDL_Fail();
    }

    return SDL_APP_CONTINUE;
}

/**
 *
 * @param title The title of the window
 * @param w default width
 * @param h default height
 * @param flags SDL window flags - it automatically adds the SDL_WINDOW_OPENGL flag
 * @return
 */
SDL_Window *RenderEngine::createWindow(
    const char *title,
    const int w, const int h,
    const SDL_WindowFlags flags = 0
) {
    window = SDL_CreateWindow(
        title,
        w,
        h,
        flags | SDL_WINDOW_OPENGL
    );
    return window;
}

SDL_AppResult RenderEngine::init() {
    SDL_Log("OpenGL renderer initializing");

    if (SDL_GLContext context = SDL_GL_CreateContext(this->window); not context) {
        return SDL_Fail();
    }

    // Load OpenGL functions
    initialize(SDL_GL_GetProcAddress);
    aux::enableGetErrorCallback();

    SDL_Log("OpenGL version: %s", aux::ContextInfo::version().toString().c_str());
    SDL_Log("OpenGL vendor: %s", aux::ContextInfo::vendor().c_str());
    SDL_Log("OpenGL renderer: %s", aux::ContextInfo::renderer().c_str());

    if (not SDL_GL_SetSwapInterval(1)) {
        return SDL_Fail();
    }
    SDL_Log("OpenGL: VSync activated");

    this->viewport_resize();

    // Color used when clearing the framebuffer
    glClearColor(0.3f, 0.4f, 0.7f, 1.0f);

    // Vertex array object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create our vertex buffer object which will store vertices
    // TODO: Later on we could put it in a model loading function
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Shader stuff

    if (this->shader.init("./shaders/shader.vs", "./shaders/shader.fs") == SDL_APP_FAILURE) {
        return SDL_APP_FAILURE;
    };

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    SDL_Log("OpenGL renderer successfully initialized");

    return SDL_APP_CONTINUE;
}

SDL_AppResult RenderEngine::render(const AppContext *app) const {
    glClear(ClearBufferMask::GL_COLOR_BUFFER_BIT);

    glUseProgram(this->shader.ID);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // We swap the buffers
    if (not SDL_GL_SwapWindow(this->window)) {
        return SDL_Fail();
    }
    return SDL_APP_CONTINUE;
}
