
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

            /// Draws the mesh. The node that owns it has already uploaded the
            /// model-view and normal matrices to the shader.
            virtual void draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const = 0;
        };

    }

#endif