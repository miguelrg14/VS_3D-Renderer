
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "opengl-recipes.hpp"

#include <SDL.h>

using namespace std;

namespace udit
{

    GLuint compile_shaders(const std::string& vertex_shader_code, const std::string& fragment_shader_code)
    {
        GLint succeeded = GL_FALSE;

        // Create the shader objects:
        GLuint   vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        // Load the shader source code:
        const char*    vertex_shaders_code[] = { vertex_shader_code.c_str() };
        const char*  fragment_shaders_code[] = { fragment_shader_code.c_str() };
        const GLint    vertex_shaders_size[] = { GLint(vertex_shader_code.size()) };
        const GLint  fragment_shaders_size[] = { GLint(fragment_shader_code.size()) };

        glShaderSource(vertex_shader_id, 1, vertex_shaders_code, vertex_shaders_size);
        glShaderSource(fragment_shader_id, 1, fragment_shaders_code, fragment_shaders_size);

        // Compile the shaders:
        glCompileShader(vertex_shader_id);
        glCompileShader(fragment_shader_id);

        // Check that compilation succeeded:
        glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(vertex_shader_id);

        glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(fragment_shader_id);

        // Create a program object:
        GLuint program_id = glCreateProgram();

        // Attach the compiled shaders to the program:
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);

        // Link the shaders:
        glLinkProgram(program_id);

        // Check that linking succeeded:
        glGetProgramiv(program_id, GL_LINK_STATUS, &succeeded);
        if (!succeeded) show_linkage_error(program_id);

        // Release the compiled shaders once they are linked:
        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        return (program_id);
    }

    GLuint create_texture_2d(const std::string& texture_path)
    {
        // Decode the image as RGBA (4 bytes per pixel):
        auto image = load_image< Rgba8888 >(texture_path);

        if (!image)
        {
            return 0;       // 0 is never a valid texture name
        }

        GLuint texture_id = 0;

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D
        (
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            GLsizei(image->get_width ()),
            GLsizei(image->get_height()),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image->colors()
        );

        glGenerateMipmap(GL_TEXTURE_2D);

        return texture_id;
    }

    /// ------------------ Error utilities -----------------

    void show_compilation_error (GLuint shader_id)
    {
        static auto message = "Error compiling a shader.";

        string info_log;
        GLint  info_log_length;

        glGetShaderiv (shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetShaderInfoLog (shader_id, info_log_length, NULL, &info_log.front ());

        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, message, info_log.c_str (), nullptr);

        throw message;
    }

    void show_linkage_error (GLuint program_id)
    {
        static auto message = "Error linking the shaders.";

        string info_log;
        GLint  info_log_length;

        glGetProgramiv (program_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetProgramInfoLog (program_id, info_log_length, NULL, &info_log.front ());

        SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, message, info_log.c_str (), nullptr);

        throw message;
    }

    /// ------------------------------------------------------

}
