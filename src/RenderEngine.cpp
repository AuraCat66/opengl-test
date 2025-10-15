#include "RenderEngine.h"

#include <cmath>

#include "glbinding/glbinding.h"
#include "glbinding/gl33core/gl.h"

#include "glbinding-aux/ContextInfo.h"
#include "glbinding-aux/Meta.h"
#include "glbinding-aux/types_to_string.h"
#include "glbinding-aux/ValidVersions.h"
#include "glbinding-aux/debug.h"

#include "../vendored/stb_image.h"

#include "AppContext.h"
#include "helperFunctions.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

float vertices[] = {
    // positions          // colors           // texture coords
    0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
   -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
   -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

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
    this->window = SDL_CreateWindow(
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

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create our vertex buffer object which will store vertices
    // TODO: Later on we could put it in a model loading function
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("./textures/container.jpg", &width, &height, &nrChannels, 0);

    glGenTextures(1, &this->texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        SDL_Log("Render engine error: Failed to load texture");
        return SDL_APP_FAILURE;
    }
    stbi_image_free(data);

    if (this->shader.init("./shaders/shader.vsh", "./shaders/shader.fsh") == SDL_APP_FAILURE) {
        return SDL_APP_FAILURE;
    };

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    SDL_Log("OpenGL renderer successfully initialized");

    return SDL_APP_CONTINUE;
}

SDL_AppResult RenderEngine::render(const AppContext *app) const {
    glClear(ClearBufferMask::GL_COLOR_BUFFER_BIT);

    glUseProgram(this->shader.ID);

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // We swap the buffers
    if (not SDL_GL_SwapWindow(this->window)) {
        return SDL_Fail();
    }
    return SDL_APP_CONTINUE;
}
