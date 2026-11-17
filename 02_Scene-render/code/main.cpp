
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "Scene.hpp"
#include "Window.hpp"

using udit::Scene;
using udit::Window;

int main(int, char* [])
{
    constexpr unsigned viewport_width = 1024;
    constexpr unsigned viewport_height = 576;

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
    int  mouse_x = 0;
    int  mouse_y = 0;

    bool camera_active = true;  // FPS camera mode on at startup
    SDL_SetRelativeMouseMode(SDL_TRUE);

    do
    {
        // Process pending events:

        SDL_Event event;

        while (SDL_PollEvent(&event) > 0)
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                //SDL_GetMouseState(&mouse_x, &mouse_y);

                if (camera_active) 
                {
                    mouse_x = event.motion.xrel;
                    mouse_y = event.motion.yrel;
                    scene.camera.process_mouse(mouse_x, mouse_y);
                }

                break;
            }

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) 
                {
                case SDLK_ESCAPE:
                    camera_active = !camera_active; // Toggle mode
                    SDL_SetRelativeMouseMode(camera_active ? SDL_TRUE : SDL_FALSE);
                    break;

                //case SDLK_w || SDLK_a || SDLK_s || SDLK_d:
                //    scene.camera.process_keyboard(keystate, delta_time);
                //    // add more cases for other keys here
                default:
                    break;
                }
                break;

            case SDL_QUIT:
            {
                exit = true;
            }
            }
        }

        // Read the current keyboard state
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        // Fixed time step (replace with a real timer)
        float delta_time = 1.0f / 60.0f;

        scene.camera.process_keyboard(keystate, delta_time);

        // Update the scene:
        scene.update();

        // Render the scene:
        scene.render();

        // Present the frame:
        window.swap_buffers();
    } while (not exit);

    SDL_Quit();

    return 0;
}
