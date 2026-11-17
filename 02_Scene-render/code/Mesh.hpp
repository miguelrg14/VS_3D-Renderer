
// This code is in the public domain
// Miguel Rodriguez Gallego

#ifndef MESH_HEADER
#define MESH_HEADER

    #pragma once

    #include <string>
    #include <glm.hpp>
    #include <glad/glad.h>

    namespace udit 
    {
        class Mesh 
        {
        public:
            virtual ~Mesh() = default;

            /// Loads the model from a file
            virtual void load(const std::string& mesh_file_path) = 0;

            /// Draws the mesh with a model matrix and a shader
            virtual void draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const = 0;
        };

    }

#endif