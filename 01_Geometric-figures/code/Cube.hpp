
// This code is in the public domain
// angel.rodriguez@udit.es

#ifndef CUBE_HEADER
#define CUBE_HEADER

    #include <glad/glad.h>

    namespace udit
    {

        class Cube
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

            // Base cube data arrays:
            static const GLfloat coordinates[];
            static const GLfloat colors     [];
            static const GLubyte indices    [];

        private:

            GLuint vbo_ids[VBO_COUNT];      // Ids of the VBOs in use
            GLuint vao_id;                  // Cube VAO id

        public:

            Cube();
           ~Cube();

            void render ();

        };

    }

#endif
