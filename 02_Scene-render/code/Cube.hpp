
// This code is in the public domain
// Miguel Rodriguez Gallego

#ifndef CUBE_HEADER
#define CUBE_HEADER

    #include <glad/glad.h>
    #include <glm.hpp>
    #include "Mesh.hpp"

    namespace udit
    {

        class Cube : public Mesh
        {
        private:

            // Indices into the vbo_ids array:
            enum
            {
                COORDINATES_VBO,
                NORMALS_VBO,
                UVS_VBO,
                INDICES_IBO,
                VBO_COUNT
            };

            // Base cube data arrays (4 vertices per face, so each face has its own normal and UVs):
            static const GLfloat coordinates[];
            static const GLfloat normals    [];
            static const GLfloat uvs        [];
            static const GLubyte indices    [];

        private:

            GLuint vbo_ids[VBO_COUNT];      // Ids of the VBOs in use
            GLuint vao_id;                  // Cube VAO id
            GLuint texture_id = 0;          // Diffuse texture (not owned by the cube)

        public:

            Cube();
           ~Cube();

            Cube(const Cube &) = delete;
            Cube & operator = (const Cube &) = delete;

            void set_texture (GLuint new_texture_id) { texture_id = new_texture_id; }

            void draw (const glm::mat4 & modelMatrix, GLuint shaderProgram) const override;

        };

    }

#endif
