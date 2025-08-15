cmake_minimum_required(VERSION 4.0 FATAL_ERROR)
project(opengl_test)

# The name that corresponds to the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)

# Prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

# On Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# Configure SDL and other libraries by calling their CMake file.
# we use EXCLUDE_FROM_ALL so that their install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/glbinding EXCLUDE_FROM_ALL)

add_executable(${EXECUTABLE_NAME} src/main.cpp
        src/RenderEngine.cpp
        src/RenderEngine.h
        src/helperFunctions.h
        src/AppContext.h
        src/Shader.cpp
        src/Shader.h
)

# We make it so our program cannot compile without the shader files
set_property(SOURCE src/main.cpp PROPERTY OBJECT_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.vsh
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.fsh)

# We create the shaders/ directory where the compiled executable will be put
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders)
# We move the shader files one by one (I'm sure there is a better way to do this)
file(COPY_FILE src/shaders/shader.vsh ${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders/shader.vsh)
file(COPY_FILE src/shaders/shader.fsh ${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders/shader.fsh)

# We link the libraries
target_link_libraries(
        ${EXECUTABLE_NAME} PUBLIC
        glbinding::glbinding
        glbinding::glbinding-aux
        SDL3::SDL3
)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)