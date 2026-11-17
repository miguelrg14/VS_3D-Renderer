
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

    // High-resolution timer used to measure the time between frames:
    const  double counter_frequency = double(SDL_GetPerformanceFrequency());
    Uint64 previous_counter         = SDL_GetPerformanceCounter();

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

        // Seconds since the previous frame. Clamped so a long pause (dragging the
        // window, a breakpoint...) doesn't make the camera or the animation jump:
        Uint64 current_counter = SDL_GetPerformanceCounter();
        float  delta_time      = float(double(current_counter - previous_counter) / counter_frequency);
        previous_counter       = current_counter;

        if (delta_time > 0.1f) delta_time = 0.1f;

        // Read the current keyboard state
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        scene.camera.process_keyboard(keystate, delta_time);

        // Update the scene:
        scene.update(delta_time);

        // Render the scene:
        scene.render();

        // Present the frame:
        window.swap_buffers();
    } while (not exit);

    SDL_Quit();

    return 0;
}
