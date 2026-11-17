
// This code is in the public domain
// Miguel Rodriguez Gallego

#pragma once

#include "Color.hpp"
#include "Color_Buffer.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <memory>
#include <SOIL2.h>
#include <string>

namespace udit
{

    GLuint compile_shaders        (const std::string & vertex_shader_code, const std::string & fragment_shader_code);
    void   show_compilation_error (GLuint  shader_id);
    void   show_linkage_error     (GLuint program_id);

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    template< typename COLOR_FORMAT >
    std::unique_ptr< Color_Buffer< COLOR_FORMAT > > load_image (const std::string & image_path)
    {
        // Load the image file with SOIL2:
        int image_width = 0;
        int image_height = 0;
        int image_channels = 0;

        uint8_t* loaded_pixels = SOIL_load_image
        (
            image_path.c_str(),
            &image_width,
            &image_height,
            &image_channels,
            SOIL_LOAD_RGBA              // Always return the pixels as RGBA32,
        );                              // whatever the format of the file

        // A non-null loaded_pixels means the image was loaded:
        if (loaded_pixels)
        {
            auto image = std::make_unique< Color_Buffer<COLOR_FORMAT> >(image_width, image_height);

            // Copy the bytes from one buffer to the other:
            std::copy_n
            (
                loaded_pixels,
                size_t(image_width) * size_t(image_height) * sizeof(typename Color_Buffer<COLOR_FORMAT>::Color),
                reinterpret_cast<uint8_t*>(image->colors())
            );

            // Free the memory SOIL2 allocated for the image:
            SOIL_free_image_data(loaded_pixels);

            return image;
        }

        return nullptr;
    }

    // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

    /// Loads an image file into a mipmapped RGBA 2D texture.
    /// Returns 0 if the image could not be loaded.
    GLuint create_texture_2d(const std::string& texture_path);

}
