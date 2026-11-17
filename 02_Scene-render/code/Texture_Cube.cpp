
// This code is in the public domain
// Miguel Rodriguez Gallego

#include <algorithm>
#include <vector>
#include <SOIL2.h>
#include "Texture_Cube.hpp"

namespace udit
{

    Texture_Cube::Texture_Cube(const std::string & texture_base_path)
    {
        texture_is_loaded = false;

        // Try to load the bitmaps of every face:

        std::vector< std::shared_ptr< Color_Buffer > > texture_sides(6);

        for (size_t texture_index = 0; texture_index < 6; texture_index++)
        {
            texture_sides[texture_index] = load_image (texture_base_path + char('0' + texture_index) + ".png");

            if (!texture_sides[texture_index])
            {
                return;
            }
        }

        // Create a texture object:

        glEnable        (GL_TEXTURE_CUBE_MAP);

        glGenTextures   (1, &texture_id);

        glActiveTexture (GL_TEXTURE0);
        glBindTexture   (GL_TEXTURE_CUBE_MAP, texture_id);

        // Texture settings: linear filtering, (s,t,r) clamped to the edge:

        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);

        // Upload the bitmaps to the GPU:

        static const GLenum texture_target[] =
        {
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        };

        for (size_t texture_index = 0; texture_index < 6; texture_index++)
        {
            Color_Buffer & texture = *texture_sides[texture_index];

            glTexImage2D
            (
                texture_target[texture_index], 
                0, 
                GL_RGBA, 
                texture.get_width  (),
                texture.get_height (),
                0, 
                GL_RGBA, 
                GL_UNSIGNED_BYTE, 
                texture.colors ()
            );
        }

        texture_is_loaded = true;
    }

    
    std::shared_ptr< Texture_Cube::Color_Buffer > Texture_Cube::load_image (const std::string & image_path)
    {
        // Load the image file with SOIL2:

        int image_width    = 0;
        int image_height   = 0;
        int image_channels = 0;

        uint8_t * loaded_pixels = SOIL_load_image
        (
            image_path.c_str (),
           &image_width, 
           &image_height, 
           &image_channels,
            SOIL_LOAD_RGBA              // Always return the pixels as RGBA32,
        );                              // whatever the format of the file

        // A non-null loaded_pixels means the image was loaded:

        if (loaded_pixels)
        {
            auto image = std::make_shared< Color_Buffer > (image_width, image_height);
            
            // Copy the bytes directly (same pixel format):

            std::copy_n
            (
                loaded_pixels, 
                size_t(image_width) * size_t(image_height) * sizeof(Color_Buffer::Color),
                reinterpret_cast< uint8_t * >(image->colors ())
            );

            // Free the memory SOIL2 allocated for the image:

            SOIL_free_image_data (loaded_pixels);

            return image;
        }

        return nullptr;
    }


    Texture_Cube::~Texture_Cube()
    {
        if (texture_is_loaded)
        {
            glDeleteTextures (1, &texture_id);
        }
    }

}
