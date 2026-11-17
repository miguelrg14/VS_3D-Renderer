
// This code is in the public domain
// angel.rodriguez@udit.es

#include "Cube.hpp"

namespace udit
{

    const GLfloat Cube::coordinates[] =
    {
       -1,-1,+1,            // 0
       +1,-1,+1,            // 1
       +1,+1,+1,            // 2
       -1,+1,+1,            // 3
       -1,-1,-1,            // 4
       +1,-1,-1,            // 5
       +1,+1,-1,            // 6
       -1,+1,-1,            // 7
    };

    const GLfloat Cube::colors[] =
    {
        0, 0, 1,            // 0    RGB-coloured cube
        1, 0, 1,            // 1
        1, 1, 1,            // 2
        0, 1, 1,            // 3
        0, 0, 0,            // 4
        1, 0, 0,            // 5
        1, 1, 0,            // 6
        0, 1, 0,            // 7
    };

    const GLubyte Cube::indices[] =
    {
        0, 1, 2,            // front
        0, 2, 3,
        4, 0, 3,            // left
        4, 3, 7,
        7, 5, 4,            // back
        7, 6, 5,
        1, 5, 6,            // right
        1, 6, 2,
        3, 2, 6,            // top
        3, 6, 7,
        5, 0, 4,            // bottom
        5, 1, 0,
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

        // Upload the colours to a VBO and link them to the VAO:
        glBindBuffer (GL_ARRAY_BUFFER, vbo_ids[COLORS_VBO]);
        glBufferData (GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

        glEnableVertexAttribArray (1);
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Upload the indices to an EBO:
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray (0);
    }

    Cube::~Cube()
    {
        // Release the VBOs and the VAO:
        glDeleteVertexArrays (1, &vao_id);
        glDeleteBuffers      (VBO_COUNT, vbo_ids);
    }

    void Cube::render ()
    {
        // Bind the VAO holding the object data and draw its elements:
        glBindVertexArray (vao_id);
        glDrawElements    (GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
        glBindVertexArray (0);
    }

}
