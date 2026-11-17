
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "Cube.hpp"

namespace udit
{

    const GLfloat Cube::coordinates[] =
    {
        -1.f, +1.f, +1.f,
        +1.f, +1.f, +1.f,
        +1.f, -1.f, +1.f,
        -1.f, -1.f, +1.f,
        -1.f, +1.f, -1.f,
        -1.f, +1.f, +1.f,
        -1.f, -1.f, +1.f,
        -1.f, -1.f, -1.f,
        +1.f, +1.f, -1.f,
        -1.f, +1.f, -1.f,
        -1.f, -1.f, -1.f,
        +1.f, -1.f, -1.f,
        +1.f, +1.f, +1.f,
        +1.f, +1.f, -1.f,
        +1.f, -1.f, -1.f,
        +1.f, -1.f, +1.f,
        -1.f, -1.f, +1.f,
        +1.f, -1.f, +1.f,
        +1.f, -1.f, -1.f,
        -1.f, -1.f, -1.f,
        -1.f, +1.f, -1.f,
        +1.f, +1.f, -1.f,
        +1.f, +1.f, +1.f,
        -1.f, +1.f, +1.f,
    };

    const GLfloat Cube::normals[] =
    {
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
         0.f,  0.f, +1.f,
        -1.f,  0.f,  0.f,
        -1.f,  0.f,  0.f,
        -1.f,  0.f,  0.f,
        -1.f,  0.f,  0.f,
         0.f,  0.f, -1.f,
         0.f,  0.f, -1.f,
         0.f,  0.f, -1.f,
         0.f,  0.f, -1.f,
        +1.f,  0.f,  0.f,
        +1.f,  0.f,  0.f,
        +1.f,  0.f,  0.f,
        +1.f,  0.f,  0.f,
         0.f, -1.f,  0.f,
         0.f, -1.f,  0.f,
         0.f, -1.f,  0.f,
         0.f, -1.f,  0.f,
         0.f, +1.f,  0.f,
         0.f, +1.f,  0.f,
         0.f, +1.f,  0.f,
         0.f, +1.f,  0.f,
    };

    const GLfloat Cube::uvs[] =
    {
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
         0.f, 0.f,
         1.f, 0.f,
         1.f, 1.f,
         0.f, 1.f,
    };

    const GLubyte Cube::indices[] =
    {
         1,  0,  3,     // front
         1,  3,  2,
         5,  4,  7,     // left
         5,  7,  6,
         9,  8, 11,     // back
         9, 11, 10,
        13, 12, 15,     // right
        13, 15, 14,
        17, 16, 19,     // bottom
        17, 19, 18,
        20, 23, 22,     // top
        20, 22, 21,
    };

    Cube::Cube()
    {
        // Generate the cube VBO ids:
        glGenBuffers (VBO_COUNT, vbo_ids);
        glGenVertexArrays (1, &vao_id);

        // Bind the cube VAO to configure it:
        glBindVertexArray (vao_id);

        // Upload the coordinates to a VBO and link them to the VAO:
        glBindBuffer (GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData (GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);

        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Upload the normals to a VBO and link them to the VAO:
        glBindBuffer (GL_ARRAY_BUFFER, vbo_ids[NORMALS_VBO]);
        glBufferData (GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

        glEnableVertexAttribArray (1);
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Upload the texture coordinates to a VBO and link them to the VAO:
        glBindBuffer (GL_ARRAY_BUFFER, vbo_ids[UVS_VBO]);
        glBufferData (GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

        glEnableVertexAttribArray (2);
        glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Upload the indices to an IBO:
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_IBO]);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray (0);
    }

    Cube::~Cube()
    {
        // Release the VBOs and the VAO:
        glDeleteVertexArrays (1, &vao_id);
        glDeleteBuffers      (VBO_COUNT, vbo_ids);
    }

    void Cube::draw (const glm::mat4 & , GLuint shaderProgram) const
    {
        // Bind the cube texture on unit 0:
        if (texture_id)
        {
            glActiveTexture (GL_TEXTURE0);
            glBindTexture   (GL_TEXTURE_2D, texture_id);
            glUniform1i     (glGetUniformLocation(shaderProgram, "sampler"), 0);
        }

        // Bind the VAO holding the object data and draw its elements:
        glBindVertexArray (vao_id);
        glDrawElements    (GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
        glBindVertexArray (0);
    }

}
