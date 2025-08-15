#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "SDL3/SDL_log.h"

#include "glbinding/glbinding.h"
#include "glbinding/gl33core/gl.h"

using namespace std;
using namespace gl33core;
using namespace glbinding;

SDL_AppResult Shader::init(const char *vertexPath, const char *fragmentPath) {
    // 1. Retrieve the vertex/fragment source code from file path

    string vertexCode;
    string fragmentCode;
    ifstream vShaderFile;
    ifstream fShaderFile;

    // Ensure ifstream objects can throw exceptions
    vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
    fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

    try {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        stringstream vShaderStream, fShaderStream;
        // Read files' buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (ifstream::failure &e) {
        SDL_LogError(0, "Shader error: File not successfully read\n%i%s", e.code().value(), e.what());
        return SDL_APP_FAILURE;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        SDL_LogError(0, "Shader error: Vertex shader compilation failed!\n%s", infoLog);
        return SDL_APP_FAILURE;
    }

    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        SDL_LogError(0, "Shader error: Fragment shader compilation failed!\n%s", infoLog);
        return SDL_APP_FAILURE;
    }

    // Shader program
    this->ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        SDL_LogError(0, "Shader error: Shader program linking failed!\n%s", infoLog);
        return SDL_APP_FAILURE;
    }

    // Delete the shaders as they are no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return SDL_APP_CONTINUE;
}

void Shader::use() const {
    glUseProgram(this->ID);
}

void Shader::setBool(const std::string &name, const bool value) const {
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
}

void Shader::setInt(const std::string &name, const int value) const {
    glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, const float value) const {
    glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
}
