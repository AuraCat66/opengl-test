#ifndef OPENGL_TEST_SHADER_H
#define OPENGL_TEST_SHADER_H

#include <string>

#include "SDL3/SDL.h"

class Shader {
public:
    unsigned int ID{};

    // Constructor reads and builds the shader from the specified paths
    SDL_AppResult init(const char* vertexPath, const char* fragmentPath);

    // Use/activate the shader
    void use() const;
    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
};


#endif //OPENGL_TEST_SHADER_H
