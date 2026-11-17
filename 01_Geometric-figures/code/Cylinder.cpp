
#include "Cylinder.hpp"
#include <vector>
#include <cmath>

namespace udit
{
    // Fixed values, kept for the classic init()
    // Coordinates: 6 bottom points, 6 top points, 2 centres
    const GLfloat Cylinder::coordinates[] =
    {
        // Bottom ring (y = 0)
         1.0f, 0.0f,  0.0f,
         0.5f, 0.0f,  0.866f,
        -0.5f, 0.0f,  0.866f,
        -1.0f, 0.0f,  0.0f,
        -0.5f, 0.0f, -0.866f,
         0.5f, 0.0f, -0.866f,

         // Top ring (y = 1)
          1.0f, 1.0f,  0.0f,
          0.5f, 1.0f,  0.866f,
         -0.5f, 1.0f,  0.866f,
         -1.0f, 1.0f,  0.0f,
         -0.5f, 1.0f, -0.866f,
          0.5f, 1.0f, -0.866f,

          // Bottom cap centre
           0.0f, 0.0f,  0.0f,

           // Top cap centre
            0.0f, 1.0f,  0.0f
    };

    // Plain colours
    const GLfloat Cylinder::colors[] =
    {
        // Bottom ring
        0.7f, 0.2f, 0.2f,
        0.7f, 0.2f, 0.2f,
        0.7f, 0.2f, 0.2f,
        0.7f, 0.2f, 0.2f,
        0.7f, 0.2f, 0.2f,
        0.7f, 0.2f, 0.2f,

        // Top ring
        0.2f, 0.7f, 0.2f,
        0.2f, 0.7f, 0.2f,
        0.2f, 0.7f, 0.2f,
        0.2f, 0.7f, 0.2f,
        0.2f, 0.7f, 0.2f,
        0.2f, 0.7f, 0.2f,

        // Bottom cap centre
        1.0f, 1.0f, 0.0f,

        // Top cap centre
        1.0f, 1.0f, 0.0f
    };

    const GLuint Cylinder::indices[] =
    {
        // SIDES (counter-clockwise triangles)
        6, 1, 0,
        7, 1, 6,

        7, 2, 1,
        8, 2, 7,

        8, 3, 2,
        9, 3, 8,

        9, 4, 3,
        10, 4, 9,

        10, 5, 4,
        11, 5, 10,

        11, 0, 5,
        6, 0, 11,

        // BOTTOM CAP (counter-clockwise seen from below)
        12, 0, 1,
        12, 1, 2,
        12, 2, 3,
        12, 3, 4,
        12, 4, 5,
        12, 5, 0,

        // TOP CAP (counter-clockwise seen from above)
        13, 7, 6,
        13, 8, 7,
        13, 9, 8,
        13,10, 9,
        13,11,10,
        13, 6,11
    };

    Cylinder::Cylinder() : indexCount(0), vao_id(0)
    {
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);
        init(); // Start with the fixed data

        Subdivision(6);

        glBindVertexArray(0);
    }

    Cylinder::~Cylinder()
    {
        glDeleteBuffers(VBO_COUNT, vbo_ids);
        glDeleteVertexArrays(1, &vao_id);
    }

    void Cylinder::init()
    {
        indexCount = sizeof(indices) / sizeof(GLuint);

        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COLORS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void Cylinder::Subdivision(int slices)
    {
        if (slices < 3) slices = 3; // a cap needs at least 3 vertices

        std::vector<GLfloat> coords;
        std::vector<GLfloat> cols;
        std::vector<GLuint> inds;

        const float radius = 1.0f;
        const float height = 1.0f;
        const float PI2 = 6.2831853f;

        // Indices of the cap centres
        const GLuint baseCenterIndex = 2 * slices;
        const GLuint topCenterIndex = 2 * slices + 1;

        // Bottom ring vertices
        for (int i = 0; i < slices; ++i)
        {
            float angle = PI2 * i / slices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            coords.push_back(x);
            coords.push_back(0.0f);
            coords.push_back(z);

            // Bottom ring colour (red)
            cols.push_back(0.7f);
            cols.push_back(0.2f);
            cols.push_back(0.2f);
        }

        // Top ring vertices
        for (int i = 0; i < slices; ++i)
        {
            float angle = PI2 * i / slices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            coords.push_back(x);
            coords.push_back(height);
            coords.push_back(z);

            // Top ring colour (green)
            cols.push_back(0.2f);
            cols.push_back(0.7f);
            cols.push_back(0.2f);
        }

        // Bottom cap centre
        coords.push_back(0.0f); coords.push_back(0.0f); coords.push_back(0.0f);
        cols.push_back(1.0f); cols.push_back(1.0f); cols.push_back(0.0f); // yellow

        // Top cap centre
        coords.push_back(0.0f); coords.push_back(height); coords.push_back(0.0f);
        cols.push_back(1.0f); cols.push_back(1.0f); cols.push_back(0.0f); // yellow

        // Build the indices

        // Sides
        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;

            GLuint bottom_current = i;
            GLuint bottom_next = next;
            GLuint top_current = i + slices;
            GLuint top_next = next + slices;

            // Two triangles per side quad
            inds.push_back(top_current);
            inds.push_back(bottom_next);
            inds.push_back(bottom_current);

            inds.push_back(top_next);
            inds.push_back(bottom_next);
            inds.push_back(top_current);
        }

        // Bottom cap (triangle fan from the centre)
        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;
            inds.push_back(baseCenterIndex);
            inds.push_back(i);
            inds.push_back(next);
        }

        // Top cap (triangle fan from the centre)
        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;
            inds.push_back(topCenterIndex);
            inds.push_back(next + slices);
            inds.push_back(i + slices);
        }

        indexCount = inds.size();

        // Upload the data to OpenGL
        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORDINATES_VBO]);
        glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(GLfloat), coords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COLORS_VBO]);
        glBufferData(GL_ARRAY_BUFFER, cols.size() * sizeof(GLfloat), cols.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void Cylinder::render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}
