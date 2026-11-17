
// This code is in the public domain
// Miguel Rodriguez Gallego

#ifndef GROUND_HEADER
#define GROUND_HEADER

    #include <glad/glad.h>

    namespace udit
    {

        class Terrain
        {
        private:

            // Indices into the vbo_ids array:
            enum
            {
                COORDINATES_VBO,
                TEXTURE_UVS_VBO,
                VBO_COUNT
            };

        private:

            GLuint  vao_id;
            GLuint  vbo_ids[VBO_COUNT];

            GLsizei number_of_vertices;

        public:

            Terrain(float width, float depth, unsigned x_slices, unsigned z_slices);
           ~Terrain();

        public:

            void render ();

        };

    }

#endif
