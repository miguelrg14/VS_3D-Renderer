// This code is in the public domain
// miguelrodriguezgallego1@gmail.com

#ifndef CONE_HEADER
#define CONE_HEADER

#include <glad/glad.h>

namespace udit
{

    class Cone
    {
    private:

        // Indices into the vbo_ids array:
        enum
        {
            COORDINATES_VBO,
            COLORS_VBO,
            INDICES_EBO,
            VBO_COUNT
        };

        int indexCount = 0;

        GLuint vbo_ids[VBO_COUNT];   // VBO ids (coordinates, colours, indices)
        GLuint vao_id;               // Cone VAO id

    public:

        Cone();
        ~Cone();

        void Subdivision(int subdivisions);

        void render();

    };

}

#endif