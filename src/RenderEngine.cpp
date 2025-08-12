#include "RenderEngine.h"

#include <glbinding/glbinding.h>
#include <glbinding/gl33core/gl.h>

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/debug.h>

#include "AppContext.h"
#include "helperFunctions.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

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

const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec4 ourColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (not context) {
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

    // Element array buffer, indices
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Shader stuff

    // We create the vertex shader (shader = GPU program)
    const unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // We attach it to the vertex shader source code
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    // We compile the shader
    glCompileShader(vertexShader);

    // Now we do the same but for the fragment shader
    const unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // We just verify if everything went well
    {
        int success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            SDL_LogError(0, "OpenGL: Vertex shader error: compilation failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("OpenGL: Vertex shader successfully compiled");

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            SDL_LogError(0, "OpenGL: Fragment shader error: compilation failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("OpenGL: Fragment shader successfully compiled");
    }

    // We can then create the shader program and link the shader objects to it
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    {
        int success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            SDL_LogError(0, "OpenGL: Shader program error: link failed!\n%s", infoLog);
            return SDL_APP_FAILURE;
        }
        SDL_Log("OpenGL: Shader objects successfully linked to shader program");
    }

    // Poor OpenGL doesn't know how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // We delete the leftover shader objects (I'm sorry for them)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    SDL_Log("OpenGL renderer successfully initialized");

    return SDL_APP_CONTINUE;
}

SDL_AppResult RenderEngine::render(const AppContext *app) const {
    glClear(ClearBufferMask::GL_COLOR_BUFFER_BIT);

    glUseProgram(this->shaderProgram);
    glBindVertexArray(this->VAO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);

    glBindVertexArray(0);
    if (not SDL_GL_SwapWindow(this->window)) {
        return SDL_Fail();
    }
    return SDL_APP_CONTINUE;
}
