
// This code is in the public domain
// angel.rodriguez@udit.es

#include "Scene.hpp"
#include "Window.hpp"

using udit::Scene;
using udit::Window;

int main (int , char * [])
{
    constexpr unsigned viewport_width  = 1024;
    constexpr unsigned viewport_height =  576;

    Window window
    (
        "OpenGL example",
        Window::Position::CENTERED,
        Window::Position::CENTERED,
        viewport_width,
        viewport_height, 
        { 3, 3 }
    );
    
    Scene scene(viewport_width, viewport_height);

    bool exit = false;

    do
    {
        // Process pending events:
        SDL_Event event;

        while (SDL_PollEvent (&event) > 0)
        {
            if (event.type == SDL_QUIT)
            {
                exit = true;
            }
        }

        // Update the scene:
        scene.update ();

        // Render the scene:
        scene.render ();

        // Present the frame:
        window.swap_buffers ();
    }
    while (not exit);

    SDL_Quit ();

    return 0;
}
