// This code is in the public domain
// miguelrodriguezgallego1@gmail.com

#ifndef CYLINDER_HEADER
#define CYLINDER_HEADER

#include <glad/glad.h>

namespace udit
{

    class Cylinder
    {
    private:

        enum
        {
            COORDINATES_VBO,
            COLORS_VBO,
            INDICES_EBO,
            VBO_COUNT
        };

        // These arrays are kept for the classic fixed-data init()
        static const GLfloat coordinates[];
        static const GLfloat colors[];
        static const GLuint  indices[];

        int indexCount = 0;

        GLuint vbo_ids[VBO_COUNT];
        GLuint vao_id;

    public:

        Cylinder();
        ~Cylinder();

        void init();                // Fills the buffers with the fixed data
        void Subdivision(int slices); // Fills the buffers with a subdivided cylinder
        void render();              // Draws the cylinder

    };

}

#endif