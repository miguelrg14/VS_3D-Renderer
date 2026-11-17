
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "Scene.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm.hpp>                          // vec3, vec4, ivec4, mat4
#include <gtc/matrix_transform.hpp>         // translate, rotate, scale, perspective
#include <gtc/type_ptr.hpp>                 // value_ptr

// Import (.obj) models into the scene
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Texture loading
#include <SDL.h>
#include <SOIL2.h>

#include "opengl-recipes.hpp"

#include "AssimpMesh.hpp"

using namespace std;
using namespace glm;

namespace udit
{
    const std::string Scene::vertex_shader_code = 
        R"(
            #version 330

            // Matrices sent from the CPU
            uniform mat4 model_view_matrix;   // Model + view: model space to eye space
            uniform mat4 projection_matrix;   // Camera projection
            uniform mat4 normal_matrix;       // Matrix used to transform normals

            // Fog range (distances from the camera)
            uniform float fog_near;           // distance where the fog starts
            uniform float fog_far;            // distance where the fog is fully opaque

            // Vertex attributes (VAO inputs)
            layout(location = 0) in vec3 vertex_coordinates;
            layout(location = 1) in vec3 vertex_normal;
            layout(location = 2) in vec2 vertex_uv;

            // Outputs to the fragment shader (interpolated per pixel)
            out vec3  view_position;          // position in eye space
            out vec3  view_normal;            // normal in eye space
            out vec2  texture_uv;             // UV coordinates
            out float fog_factor;             // fog amount [0..1]

            void main() 
            {
                // 1) Transform the position and the normal to eye space
                vec4 pos_view = model_view_matrix * vec4(vertex_coordinates, 1.0);

                view_position = pos_view.xyz;
                view_normal   = (normal_matrix * vec4(vertex_normal, 0.0)).xyz;
                texture_uv    = vertex_uv;

                // 2) Fog factor from the eye-space distance
                float d = -pos_view.z;
                fog_factor = clamp((d - fog_near) / (fog_far - fog_near), 0.0, 1.0);

                // 3) Final clip-space position
                gl_Position = projection_matrix * pos_view;
            }
        )";

    const std::string Scene::fragment_shader_code = 
        R"(
            #version 330

            // Point light description
            struct Light
            {
                vec4 position;                // Light position in eye space
                vec3 color;                   // Light colour/intensity (RGB)
            };

            uniform Light light;
            uniform float ambient_intensity;  // Ambient light intensity
            uniform float diffuse_intensity;  // Diffuse light intensity
            uniform float specular_intensity; // Overall specular strength
            uniform float shininess;          // Shininess exponent
            uniform vec3  specular_color;     // Specular highlight colour
            uniform vec3  material_color;     // Base (diffuse) material colour

            // Texture and fog
            uniform sampler2D sampler;        // unit 0: the 2D texture
            uniform vec3      fog_color;      // fog colour

            // Inputs from the vertex shader
            in vec3  view_position;
            in vec3  view_normal;
            in vec2  texture_uv;
            in float fog_factor;              // [0 = clear, 1 = full fog]

            // Output
            out vec4 fragment_color;

            void main() 
            {
                // Discard fragments that are fully inside the fog
                if (fog_factor >= 1.0)
                {
                    discard;
                }

                // 1) Blinn-Phong lighting, evaluated for every pixel
                vec3 N = normalize(view_normal);                          // re-normalised after interpolation
                vec3 L = normalize(light.position.xyz - view_position);
                vec3 V = normalize(-view_position);
                vec3 H = normalize(L + V);

                float diffuse  = diffuse_intensity  * max(dot(N, L), 0.0);
                float specular = specular_intensity * pow(max(dot(N, H), 0.0), shininess);

                vec3 lit_color = ambient_intensity * material_color
                               + diffuse * light.color * material_color
                               + specular * specular_color;

                // 2) Texture
                vec4 texcol = texture(sampler, texture_uv);
                vec3 littex = lit_color * texcol.rgb;

                // 3) Blend with the fog colour and fade the alpha with the fog
                vec3  final_rgb   = mix(littex, fog_color, fog_factor);
                float final_alpha = texcol.a * (0.5 - fog_factor);

                fragment_color = vec4(final_rgb, final_alpha);
            }
        )";

    /// Vertex shader for the post-processing quad
    const std::string Scene::effect_vertex_shader_code = 
        R"(
            #version 330

            /// Input attributes (VAO):
            layout (location = 0) in vec3 vertex_coordinates; // Vertex position in clip space (-1 to +1)
            layout (location = 1) in vec2 vertex_texture_uv;  // UVs used to sample the texture

            /// Output to the fragment shader:
            out vec2 texture_uv;  // UV passed on for sampling

            void main()
            {
                // 1) The position is already in clip space
                gl_Position = vec4(vertex_coordinates, 1.0);
                // 2) Pass the UV to the fragment shader
                texture_uv  = vertex_texture_uv;
            }
        )";

    /// Fragment shader with a simple full-screen effect
    const std::string Scene::effect_fragment_shader_code = 
        R"(
            #version 330

            /// Texture the scene was rendered into
            uniform sampler2D sampler2d;

            /// Toggled at runtime with the P key
            uniform bool effect_enabled;

            /// Input from the vertex shader:
            in  vec2 texture_uv;      // Interpolated UVs

            /// Fragment shader output:
            out vec4 fragment_color;  // Final fragment colour

            void main()
            {
                // 1) Sample the original colour
                vec3 color = texture (sampler2d, texture_uv.st).rgb;

                if (!effect_enabled)
                {
                    fragment_color = vec4(color, 1.0);      // original image
                    return;
                }

                /// Effect: soft sepia tone
                // 2) Convert to average luminance
                float i = (color.r + color.g + color.b) * 0.3333333333;
                // 3) Apply a brownish (soft sepia) tint
                vec3 sepia = vec3(1.0, 0.75, 0.5);
                fragment_color = vec4(vec3(i, i, i) * sepia, 1.0);
            }
        )";

    // Terrain shaders
    const std::string udit::Scene::terrain_vertex_shader_code = 
        R"(
            #version 330

            uniform mat4 model_view_matrix;
            uniform mat4 projection_matrix;

            layout(location = 0) in vec2 vertex_xz;
            layout(location = 1) in vec2 vertex_uv;

            uniform sampler2D sampler;
            uniform float     max_height;

            out float intensity;

            void main()
            {
                float sample = texture(sampler, vertex_uv).r;
                intensity    = sample * 0.75 + 0.25;
                float height = sample * max_height;
                vec4  xyzw   = vec4(vertex_xz.x, height, vertex_xz.y, 1.0);

                gl_Position  = projection_matrix * model_view_matrix * xyzw;
            }
        )";
    const std::string udit::Scene::terrain_fragment_shader_code = 
        R"(
            #version 330

            in  float intensity;
            out vec4  fragment_color;

            void main()
            {
                fragment_color = vec4(intensity, intensity, intensity, 1.0);
            }
        )";

    const string Scene::texture_path_terrain = "../assets/height-map.png";

    Scene::Scene(unsigned width, unsigned height)
        : 
        terrain(10.f, 10.f, 50, 50),
        angle(0),
        camera(float(width) / float(height)),
        skybox("../assets/sky-cube-map-")
    {
        /// Scene graph
        rootNode = std::make_unique<SceneNode>();

        /// Terrain
        // 1) Compile the terrain shaders
        terrain_program_id = compile_shaders(terrain_vertex_shader_code,terrain_fragment_shader_code);

        // 2) Load the height map (red channel of a PNG)
        height_texture_id = create_texture_2d(texture_path_terrain);

        // 3) Set the static uniforms
        glUseProgram(terrain_program_id);
        // sampler on texture unit 1
        GLint loc = glGetUniformLocation(terrain_program_id, "sampler");
        glUniform1i(loc, 1);
        // maximum height = 5 units
        loc = glGetUniformLocation(terrain_program_id, "max_height");
        glUniform1f(loc, 5.0f);
        ///

        /// Post-processing
        // Create the off-screen framebuffer:
        build_framebuffer();

        // Compile and activate the shaders:
               program_id = compile_shaders(vertex_shader_code, fragment_shader_code);
        effect_program_id = compile_shaders(effect_vertex_shader_code, effect_fragment_shader_code);

        glUseProgram(program_id);

        /// Shader uniform ids
        // Models
        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");
            normal_matrix_id = glGetUniformLocation(program_id, "normal_matrix"    );
           material_color_id = glGetUniformLocation(program_id, "material_color"   );
           light_position_id = glGetUniformLocation(program_id, "light.position"   );
        // Terrain
        // Post-processing
        effect_enabled_id = glGetUniformLocation(effect_program_id, "effect_enabled");
        // Terrain
        terrain_projection_matrix_id = glGetUniformLocation(terrain_program_id, "projection_matrix");
        terrain_model_view_matrix_id = glGetUniformLocation(terrain_program_id, "model_view_matrix");

        /// Fog
        // Fog settings:
        GLint  fog_near = glGetUniformLocation(program_id, "fog_near" );
        GLint   fog_far = glGetUniformLocation(program_id, "fog_far"  );
        GLint fog_color = glGetUniformLocation(program_id, "fog_color");

        glUniform1f( fog_near, 30.0f);   // Where the fog starts
        glUniform1f(  fog_far, 50.0f);   // Where the fog is complete
        glUniform3f(fog_color, 0.8f, 0.8f, 0.9f);

        configure_material(program_id);
           configure_light(program_id);

        // Basic render state:
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.f, 0.f, 0.f, 1.f);

        resize(width, height);

        load_mesh("../assets/Ground.obj"  , "../assets/Stone_Base_Color.png",                   glm::translate(glm::mat4(1.0f), glm::vec3( 15.0f,  0.0f,  0.0f)));
        load_mesh("../assets/Painting.obj", "../assets/Frame1.jpg",                             glm::translate(glm::mat4(1.0f), glm::vec3( 10.0f,  0.0f, -3.0f)));
        load_mesh("../assets/Plant.obj"   , "../assets/plant1_Material.001_BaseColor.png",      glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -1.0f,  0.0f)));
        load_mesh("../assets/Umbrella.obj", "../assets/Umbrella_DefaultMaterial_BaseColor.png", glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f,  0.0f,  0.0f)));

        /// Animated cubes. One cube mesh is shared by three nodes:
        ///   rootNode
        ///   +-- spinning cube
        ///   |   +-- orbiting cube (inherits the spin of its parent)
        ///   transparentRoot
        ///   +-- transparent cube
        cube_texture_id = create_texture_2d("../assets/wood.png");

        cube_mesh = std::make_shared<Cube>();
        cube_mesh->set_texture(cube_texture_id);

        auto spinning = std::make_unique<SceneNode>();
        auto orbiting = std::make_unique<SceneNode>();
        auto see_through = std::make_unique<SceneNode>();

        spinning   ->addMesh(cube_mesh);
        orbiting   ->addMesh(cube_mesh);
        see_through->addMesh(cube_mesh);

        spinning_cube    = spinning.get();
        orbiting_cube    = orbiting.get();
        transparent_cube = see_through.get();

        spinning->addChild(std::move(orbiting));
        rootNode->addChild(std::move(spinning));

        transparentRoot = std::make_unique<SceneNode>();
        transparentRoot->addChild(std::move(see_through));

        update(0.f);    // Initial transforms
    }

    Scene::~Scene()
    {
        // Post-processing resources
        glDeleteVertexArrays (1, &framebuffer_quad_vao);
        glDeleteBuffers      (2,  framebuffer_quad_vbos);
        glDeleteTextures     (1, &out_texture_id);
        glDeleteRenderbuffers(1, &depthbuffer_id);
        glDeleteFramebuffers (1, &framebuffer_id);

        // Terrain height map
        glDeleteTextures(1, &height_texture_id);

        // Cube texture (the cube mesh itself frees its buffers)
        glDeleteTextures(1, &cube_texture_id);

        // Shader programs
        glDeleteProgram(program_id);
        glDeleteProgram(effect_program_id);
        glDeleteProgram(terrain_program_id);
    }

    void Scene::update (float delta_time)
    {
        // Real-time scene rotation: 0.6 rad/s, the speed it had at a fixed 60 FPS
        angle += 0.6f * delta_time;

        const glm::vec3 y_axis(0.f, 1.f, 0.f);

        // Spinning cube
        spinning_cube->localTransform = glm::translate(glm::mat4(1.f), glm::vec3(-10.f, 0.f, -10.f))
                                      * glm::rotate   (glm::mat4(1.f), angle, y_axis);

        // Orbiting cube: relative to its parent, so it circles around it
        orbiting_cube->localTransform = glm::translate(glm::mat4(1.f), glm::vec3(5.f, 0.f, 0.f))
                                      * glm::scale    (glm::mat4(1.f), glm::vec3(0.5f))
                                      * glm::rotate   (glm::mat4(1.f), angle * 5.f, y_axis);

        // Transparent cube
        transparent_cube->localTransform = glm::translate(glm::mat4(1.f), glm::vec3(10.f, 0.f, 0.f))
                                         * glm::scale    (glm::mat4(1.f), glm::vec3(0.5f))
                                         * glm::rotate   (glm::mat4(1.f), angle * 5.f, y_axis);
    }


    void Scene::render()
    {
        /// --- PASS 1: render into the FRAMEBUFFER ---
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /// 1.2) Camera matrices
        const glm::mat4              view = camera.get_transform_matrix_inverse();
        const glm::mat4 camera_projection = camera.get_projection_matrix();
        /// 1.1) Skybox (always drawn behind everything)
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skybox.render(camera);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        /// 1.3) Height-mapped terrain (displaced in the shader)
        // 1.3.1) Terrain
        glUseProgram(terrain_program_id);

        // height map on texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, height_texture_id);

        // Upload the terrain matrices
        glUniformMatrix4fv(terrain_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(camera_projection));
        glUniformMatrix4fv(terrain_model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(view));

        // Draw the terrain grid
        terrain.render();
        ///

        // 1.4) Opaque objects (cubes and meshes)
        glUseProgram(program_id);

        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(camera_projection));

        // The light is fixed in the world; the shader works in eye space, so it is
        // moved into eye space with the current view matrix:
        glm::vec4 light_view_position = view * light_world_position;
        glUniform4fv(light_position_id, 1, glm::value_ptr(light_view_position));

        rootNode->draw(view, program_id);

        /// 1.5) Transparent objects
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        transparentRoot->draw(view, program_id);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        ///

        /// --- PASS 2: post-process to the screen ---
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);

        glDisable(GL_DEPTH_TEST);
        glUseProgram(effect_program_id);
        glUniform1i(effect_enabled_id, post_effect_enabled ? 1 : 0);

        glBindVertexArray(framebuffer_quad_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, out_texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    /// <summary>
    ///  Adapts the projection and viewport to the new window size
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void Scene::resize (unsigned width, unsigned height)
    {
        window_width  = width;
        window_height = height;

        camera.set_ratio(float(width) / float(height));

        if (!low_resolution)
        {
            resize_framebuffer(GLsizei(width), GLsizei(height));
        }

        glViewport (0, 0, width, height);
    }

    void Scene::toggle_post_effect()
    {
        post_effect_enabled = !post_effect_enabled;
    }

    void Scene::toggle_low_resolution()
    {
        low_resolution = !low_resolution;

        if (low_resolution) resize_framebuffer(low_resolution_size, low_resolution_size);
        else                resize_framebuffer(window_width, window_height);
    }

    /// ------------------ POST-PROCESSING ------------------

    void Scene::resize_framebuffer(GLsizei width, GLsizei height)
    {
        if (width <= 0 || height <= 0) return;

        framebuffer_width  = width;
        framebuffer_height = height;

        // Re-allocate the colour texture and the depth buffer. They stay attached
        // to the framebuffer, which picks up the new size:
        glBindTexture(GL_TEXTURE_2D, out_texture_id);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glBindRenderbuffer   (GL_RENDERBUFFER, depthbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    }

    void Scene::build_framebuffer()
    {
        // Create a framebuffer to render into:
        {
            glGenFramebuffers(1, &framebuffer_id);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        }

        // Create the texture used as the framebuffer colour buffer:
        {
            glGenTextures(1, &out_texture_id);
            glBindTexture(GL_TEXTURE_2D, out_texture_id);

            // RGB colour buffer:

            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGB,
                framebuffer_width,
                framebuffer_height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                0
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        // Create a depth buffer for the framebuffer:
        {
            glGenRenderbuffers(1, &depthbuffer_id);
            glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer_id);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer_id);
        }

        // Configure the framebuffer:
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out_texture_id, 0);

            const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;

            glDrawBuffers(1, &draw_buffer);
        }

        // Make sure the framebuffer is complete:

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        // Unbind the framebuffer:

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Build the full-screen quad used to draw the framebuffer:

        static const GLfloat quad_positions[] =
        {
            +1.0f, -1.0f, 0.0f,
            +1.0f, +1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f,
            -1.0f, +1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            +1.0f, -1.0f, 0.0f,
        };

        static const GLfloat quad_texture_uvs[] =
        {
            +1.0f,  0.0f,
            +1.0f, +1.0f,
             0.0f, +1.0f,
             0.0f, +1.0f,
             0.0f,  0.0f,
             1.0f,  0.0f,
        };

        glGenVertexArrays(1, &framebuffer_quad_vao);
        glGenBuffers(2, framebuffer_quad_vbos);

        glBindVertexArray(framebuffer_quad_vao);

        glBindBuffer(GL_ARRAY_BUFFER, framebuffer_quad_vbos[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_positions), quad_positions, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, framebuffer_quad_vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texture_uvs), quad_texture_uvs, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    ///----------------------------------------------------

    /// <summary>
    ///     Imports a 3D model into the scene
    /// </summary>
    /// <param name="path"></param>
    void Scene::load_mesh
    (
        const std::string& mesh_file_path,
        const std::string& texture_file_path,
        const glm::mat4&   localTransform
    )
    {
        auto mesh = std::make_shared<AssimpMesh>();

        // Load the texture and hand it to the mesh (the mesh owns it from now on):
        GLuint texture = create_texture_2d(texture_file_path);

        if (texture == 0)
        {
            SDL_Log("[Scene] failed to load texture: %s", texture_file_path.c_str());
        }

        mesh->setTextureID(texture);

        // Then load the geometry. A missing or broken model is skipped instead of
        // stopping the whole scene:
        try
        {
            mesh->load(mesh_file_path);
        }
        catch (const std::exception & error)
        {
            SDL_Log("[Scene] skipping model: %s", error.what());
            return;
        }

        auto node = std::make_unique<SceneNode>();
        node->addMesh(mesh);
        node->localTransform = localTransform;
        rootNode->addChild(std::move(node));
    }

    void Scene::configure_material(GLuint program_id)
    {
        GLint material_color = glGetUniformLocation(program_id, "material_color");

        glUniform3f(material_color, 1.f, 1.f, 1.f);
    }

    void Scene::configure_light(GLuint program_id)
    {
        GLint       light_color = glGetUniformLocation(program_id, "light.color"       );
        GLint ambient_intensity = glGetUniformLocation(program_id, "ambient_intensity" );
        GLint diffuse_intensity = glGetUniformLocation(program_id, "diffuse_intensity" );
        GLint      spec_int_loc = glGetUniformLocation(program_id, "specular_intensity");
        GLint     shininess_loc = glGetUniformLocation(program_id, "shininess"         );
        GLint    spec_color_loc = glGetUniformLocation(program_id, "specular_color"    );

        glUniform3f(light_color      , 1.0f, 1.f, 1.f        );
        glUniform1f(ambient_intensity, 0.2f                  );
        glUniform1f(diffuse_intensity, 0.8f                  );
        glUniform1f(spec_int_loc     , 1.0f                  ); // highlight strength
        glUniform1f(shininess_loc    , 32.0f                 ); // material shininess
        glUniform3f(spec_color_loc   , 1.0f, 1.0f, 1.0f      ); // highlight colour (white)
    }

}

