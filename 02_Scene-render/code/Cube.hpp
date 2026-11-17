
// This code is in the public domain
// Miguel Rodriguez Gallego

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
                NORMALS_VBO,
                INDICES_IBO,
                VBO_COUNT
            };

            // Base cube data arrays:
            static const GLfloat coordinates[];
            static const GLfloat normals    [];
            static const GLubyte indices    [];

        private:

            GLuint vbo_ids[VBO_COUNT];      // Ids of the VBOs in use
            GLuint vao_id;                  // Cube VAO id

        public:

            Cube();
           ~Cube();

            void render ();

        };
        //struct IMesh { virtual void draw(const glm::mat4&, GLuint) = 0; };
        //class Cube : public IMesh 
        //{
        //        void draw(const glm::mat4& modelMatrix, GLuint program) override {
        //        GLint loc = glGetUniformLocation(program, "model_view_matrix");
        //        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        //        glBindVertexArray(vao_id);
        //        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
        //    }
        //};


    }

#endif
