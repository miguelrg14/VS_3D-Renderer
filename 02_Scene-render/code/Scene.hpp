
// This code is in the public domain
// Miguel Rodriguez Gallego

#pragma once

#include <memory>
#include <string>
#include <glad/glad.h>
#include <glm.hpp>
#include "Color.hpp"
#include "Color_Buffer.hpp"
#include "Camera.hpp"
#include "Skybox.hpp"
#include "Cube.hpp"
#include "Terrain.hpp"

#include "SceneNode.hpp"

namespace udit
{

    class Scene
    {
    private:

        std::unique_ptr<SceneNode> rootNode;

        // Post-processing: off-screen framebuffer resolution
        GLsizei  framebuffer_width = 256;
        GLsizei framebuffer_height = 256;

        static const std::string           vertex_shader_code;
        static const std::string         fragment_shader_code;
        static const std::string    effect_vertex_shader_code;
        static const std::string  effect_fragment_shader_code;
        static const std::string   terrain_vertex_shader_code;
        static const std::string terrain_fragment_shader_code;
        static const std::string         texture_path_terrain;

        /// Shader programs
        GLuint          program_id = 0;
        GLuint   effect_program_id = 0;
        GLuint  terrain_program_id = 0;

        /// Uniform locations
        GLint           model_view_matrix_id = -1;
        GLint           projection_matrix_id = -1;
        GLint               normal_matrix_id = -1;
        GLint              material_color_id = -1;
        GLint   terrain_projection_matrix_id = -1;
        GLint   terrain_model_view_matrix_id = -1;

        /// Terrain (height-mapped grid)
        GLuint height_texture_id = 0;
        Terrain terrain;

        Cube  cube;
        float angle;

        /// Post-processing
        GLuint           framebuffer_id = 0;
        GLuint           depthbuffer_id = 0;
        GLuint           out_texture_id = 0;
        GLuint     framebuffer_quad_vao = 0;
        GLuint framebuffer_quad_vbos[2] = { 0, 0 };

        int window_width  = 0;
        int window_height = 0;

    public:

        /// Camera
        Camera camera;
        Skybox skybox;

        Scene(unsigned width, unsigned height);
        ~Scene();

        Scene(const Scene &) = delete;
        Scene & operator = (const Scene &) = delete;

        void   update();
        void   render();
        void   resize(unsigned width, unsigned height);

    private:

        /// Post-processing
        void   build_framebuffer();

        void load_mesh
        (
            const std::string& mesh_file_path, 
            const std::string& texture_file_path, 
            const glm::mat4& localTransform = glm::mat4(1.0f)
        );

        void   configure_material(GLuint program_id);
        void   configure_light(GLuint program_id);
    };
}