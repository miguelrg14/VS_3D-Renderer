#include "Cone.hpp"
#include <vector>
#include <cmath>

namespace udit
{
    Cone::Cone()
    {
        indexCount = 0;
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);

        Subdivision(6);

        glBindVertexArray(0);
    }

    Cone::~Cone()
    {
        glDeleteBuffers(VBO_COUNT, vbo_ids);
        glDeleteVertexArrays(1, &vao_id);
    }

    void Cone::Subdivision(int slices)
    {
        std::vector<GLfloat> coords;
        std::vector<GLfloat> cols;
        std::vector<GLuint> inds;

        const float radius = 1.0f;
        const float height = 1.0f;
        const float PI2 = 6.2831853f;

        // Indices:
        const GLuint apexIndex = 0;
        const GLuint baseCenterIndex = 1;

        // Cone apex
        coords.push_back(0.0f); coords.push_back(height); coords.push_back(0.0f); // index 0
        cols.push_back(1.0f); cols.push_back(1.0f); cols.push_back(1.0f);

        // Base centre
        coords.push_back(0.0f); coords.push_back(0.0f); coords.push_back(0.0f); // index 1
        cols.push_back(0.5f); cols.push_back(0.5f); cols.push_back(0.5f);

        // Base ring vertices
        for (int i = 0; i < slices; ++i)
        {
            float angle = PI2 * i / slices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            coords.push_back(x);
            coords.push_back(0.0f);
            coords.push_back(z);

            cols.push_back(1.0f - float(i) / slices);
            cols.push_back(0.3f);
            cols.push_back(float(i) / slices);
        }

        // Side triangles
        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;
            inds.push_back(apexIndex);
            inds.push_back(2 + next);
            inds.push_back(2 + i);
        }

        // Base triangles
        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;
            inds.push_back(baseCenterIndex);
            inds.push_back(2 + i);
            inds.push_back(2 + next);
        }

        indexCount = inds.size();

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

    void Cone::render()
    {
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}
