
// This code is in the public domain
// angel.rodriguez@udit.es

#pragma once

#include "Scene.hpp"

#include <iostream>
#include <cassert>

#include <glm.hpp>                          // vec3, vec4, ivec4, mat4
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr

namespace udit
{
    
    using namespace std;

    const string Scene::vertex_shader_code =

        "#version 330\n"
        ""
        "uniform mat4 model_view_matrix;"
        "uniform mat4 projection_matrix;"
        ""
        "layout (location = 0) in vec3 vertex_coordinates;"
        "layout (location = 1) in vec3 vertex_color;"
        ""
        "out vec3 front_color;"
        ""
        "void main()"
        "{"
        "   gl_Position = projection_matrix * model_view_matrix * vec4(vertex_coordinates, 1.0);"
        "   front_color = vertex_color;"
        "}";

    const string Scene::fragment_shader_code =

        "#version 330\n"
        ""
        "in  vec3    front_color;"
        "out vec4 fragment_color;"
        ""
        "void main()"
        "{"
        "    fragment_color = vec4(front_color, 1.0);"
        "}";

    Scene::Scene(unsigned width, unsigned height)
    :
        angle(0)
    {
        // Basic render state:
        glEnable     (GL_CULL_FACE);
        glDisable    (GL_DEPTH_TEST);
        glClearColor (.1f, .1f, .1f, 1.f);

        // Compile and activate the shaders:
        GLuint program_id = compile_shaders ();

        glUseProgram (program_id);

        model_view_matrix_id = glGetUniformLocation (program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation (program_id, "projection_matrix");

        resize (width, height);
    }

    void Scene::update ()
    {
        angle += 0.01f; // Real-time scene rotation
    }

    void Scene::render ()
    {
        glClear (GL_COLOR_BUFFER_BIT);

        /// Cube ///
        //// Rotate the cube and push it back:
        //glm::mat4 model_view_matrix(1);

        //model_view_matrix = glm::translate (model_view_matrix, glm::vec3(-2.f, 0.f, -4.f));
        //model_view_matrix = glm::rotate    (model_view_matrix, angle, glm::vec3(1.f, 2.f, 1.f));

        //glUniformMatrix4fv (model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));

        //// Draw the cube:
        //cube.render();

        /// Plane ///
        // Rotate the plane and push it back:
        glm::mat4 model_view_matrix(1);

        model_view_matrix = glm::translate(model_view_matrix, glm::vec3(0.0f, -2.5f, -4.f)    );
        model_view_matrix = glm::rotate   (model_view_matrix, -1.6f, glm::vec3(1.f, 0.f, 0.f));

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));

        // Draw the plane:
        plane.render ();

        /// Cone ///
        // Rotate the cone and push it back:
        glm::mat4 model_view_matrix2(1);

        model_view_matrix2 = glm::translate(model_view_matrix2, glm::vec3(-1.2f, -2.5f, -7.f)   );
        model_view_matrix2 = glm::rotate   (model_view_matrix2, angle, glm::vec3(0.f, 2.f, 0.f));

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix2));

        // Draw the cone:
        cone.render();

        /// Cylinder ///
        // Rotate the cylinder and push it back:
        glm::mat4 model_view_matrix3(1);

        model_view_matrix3 = glm::translate(model_view_matrix3, glm::vec3(1.2f, -2.5f, -7.f)    );
        model_view_matrix3 = glm::rotate   (model_view_matrix3, angle, glm::vec3(0.f, 2.f, 0.f));

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix3));

        // Draw the cylinder:
        cylinder.render();
    }

    /// <summary>
    ///  Adapts the projection and viewport to the new window size
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void Scene::resize (unsigned width, unsigned height)
    {
        glm::mat4 projection_matrix = glm::perspective (20.f, GLfloat(width) / height, 1.f, 5000.f);

        glUniformMatrix4fv (projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glViewport (0, 0, width, height);
    }

    GLuint Scene::compile_shaders ()
    {
        /// Compile each shader stage
        GLint succeeded = GL_FALSE;

        // Create the shader objects:
        GLuint   vertex_shader_id = glCreateShader (GL_VERTEX_SHADER  );
        GLuint fragment_shader_id = glCreateShader (GL_FRAGMENT_SHADER);

        // Load the shader source code:
        const char *   vertex_shaders_code[] = {          vertex_shader_code.c_str () };
        const char * fragment_shaders_code[] = {        fragment_shader_code.c_str () };
        const GLint    vertex_shaders_size[] = { (GLint)  vertex_shader_code.size  () };
        const GLint  fragment_shaders_size[] = { (GLint)fragment_shader_code.size  () };

        glShaderSource  (  vertex_shader_id, 1,   vertex_shaders_code,   vertex_shaders_size);
        glShaderSource  (fragment_shader_id, 1, fragment_shaders_code, fragment_shaders_size);

        // Compile the shaders:
        glCompileShader (  vertex_shader_id);
        glCompileShader (fragment_shader_id);

        /// ---------------------------------------------------
        // Check that compilation succeeded:
        glGetShaderiv   (  vertex_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error (  vertex_shader_id);

        glGetShaderiv   (fragment_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error (fragment_shader_id);
        /// ---------------------------------------------------

        /// Link the compiled shaders into a GPU program
        // Create a program object:
        GLuint program_id = glCreateProgram ();

        // Attach the compiled shaders to the program:
        glAttachShader  (program_id,   vertex_shader_id);
        glAttachShader  (program_id, fragment_shader_id);

        // Link the shaders:
        glLinkProgram   (program_id);

        // Check that linking succeeded:
        glGetProgramiv  (program_id, GL_LINK_STATUS, &succeeded);
        if (!succeeded) show_linkage_error (program_id);

        // Release the compiled shaders once they are linked:
        glDeleteShader (  vertex_shader_id);
        glDeleteShader (fragment_shader_id);

        return (program_id);
    }

    /// ------------------ Error utilities ---------------------------------

    void Scene::show_compilation_error (GLuint shader_id)
    {
        string info_log;
        GLint  info_log_length;

        glGetShaderiv (shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetShaderInfoLog (shader_id, info_log_length, NULL, &info_log.front ());

        cerr << info_log.c_str () << endl;

        #ifdef _MSC_VER
            //OutputDebugStringA (info_log.c_str ());
        #endif

        assert(false);
    }

    void Scene::show_linkage_error (GLuint program_id)
    {
        string info_log;
        GLint  info_log_length;

        glGetProgramiv (program_id, GL_INFO_LOG_LENGTH, &info_log_length);

        info_log.resize (info_log_length);

        glGetProgramInfoLog (program_id, info_log_length, NULL, &info_log.front ());

        cerr << info_log.c_str () << endl;

        #ifdef _MSC_VER
            //OutputDebugStringA (info_log.c_str ());
        #endif

        assert(false);
    }

    /// ---------------------------------------------------
}
