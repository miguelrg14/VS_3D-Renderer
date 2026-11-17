
// This code is in the public domain
// miguelrodriguezgallego1@gmail.com

#ifndef PLANE_HEADER
#define PLANE_HEADER

    #include <glad/glad.h>

    namespace udit
    {

        class Plane
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

            // Base plane data arrays:
            static const GLfloat    coordinates [];
            static const GLfloat    colors      [];
            static const GLuint     indices     [];

            int indexCount = 0;

        private:

            GLuint vbo_ids[VBO_COUNT];      // Ids of the VBOs in use
            GLuint vao_id;                  // Plane VAO id

        public:

            Plane();
            ~Plane();

            void Subdivision(int amount);

            void render();

        };

    }

#endif