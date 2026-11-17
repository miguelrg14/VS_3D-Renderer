# Known issues & roadmap

An honest list of current limitations, ordered roughly by impact. Each one points to where it lives in the code.

## Bugs

### `.obj` models are not in the repository
`.gitignore` contains `*.obj` (meant for MSVC object files), so `Terreno.obj`, `Painting.obj`, `Plant.obj` and `Umbrella.obj` were never committed. `AssimpMesh::load` throws when a file is missing and nothing catches the exception, so **project 02 crashes on a fresh clone**.

**Fix:** add an exception to `.gitignore` and commit the models:
```gitignore
*.obj
!02_Scene-render/assets/*.obj
```
Optionally, catch the exception in `Scene::load_mesh` and skip the model, so one missing asset doesn't stop the whole scene.

### `Window` move-assignment has no return value
`Window::operator=(Window&&)` in `Window.hpp` (both projects) is missing `return *this;`. That's undefined behaviour if it's ever used.

### Texture created twice in `create_texture_2d`
`opengl-recipes.cpp` calls `SOIL_load_OGL_texture` (which creates a GL texture) and then `glGenTextures`, which overwrites the id. The first texture is never deleted, and every image is also decoded twice.

### Wireframe state leaks (project 01)
`Plane::render()` and `Cube::render()` call `glPolygonMode(GL_LINE)` without restoring `GL_FILL`, so every later draw is also wireframe.

### Projection set with degrees passed as radians
`Scene::resize` (both projects) calls `glm::perspective(20.f, …)`, which takes radians. In project 02 the camera's projection overrides it every frame. Project 01 uses it directly. The fix is `glm::radians(...)`.

### Normal matrix not updated for the orbiting cube
In `Scene::render`, the second cube uploads a new `model_view_matrix` but reuses the parent's `normal_matrix`, so its lighting is slightly off.

## Limitations

- **Per-vertex lighting:** Blinn-Phong is evaluated in the vertex shader (Gouraud), so specular highlights look faceted on low-poly meshes. Moving it to the fragment shader gives per-pixel lighting.
- **Single light, fixed in eye space:** `light.position` is never transformed by the view matrix, so the light moves with the camera.
- **First mesh only:** `AssimpMesh::load` reads only `scene->mMeshes[0]`. Models with several meshes or materials are partly loaded.
- **16-bit indices:** indices are `GLushort`, so meshes with more than 65,535 vertices will be corrupted. Use `GLuint` / `GL_UNSIGNED_INT`.
- **Fixed timestep:** `delta_time` is hard-coded to `1/60`, so camera speed depends on frame rate. Measure it with `SDL_GetTicks64()` or `SDL_GetPerformanceCounter()`.
- **Framebuffer size is fixed** (256×256) and isn't rebuilt when the window is resized.
- **Terrain is wireframe only** (`GL_LINE_STRIP`). It has no normals or texture, and it isn't lit.
- **Scene graph only partly used:** the imported models go through `SceneNode`, but the cubes are still drawn by hand in `Scene::render`.
- **Shaders in strings:** GLSL is embedded in C++ raw strings. Moving it to `.vert`/`.frag` files would allow hot-reloading and editor syntax highlighting.

## Portability

The build depends on Visual Studio and on prebuilt Windows x64 libraries. A `CMakeLists.txt` that gets SDL2, Assimp, GLM and SOIL2 through `FetchContent` or vcpkg would make the project build on Linux and macOS and in CI.

## Roadmap ideas

- [ ] Commit the `.obj` models and fix `.gitignore`
- [ ] Per-fragment lighting and several lights (point and directional)
- [ ] Normal mapping
- [ ] Shadow mapping
- [ ] Frustum culling with scene-graph bounding volumes
- [ ] Instanced rendering
- [ ] Switchable post-process effects at runtime (none / sepia / greyscale / blur)
- [ ] Dear ImGui panel for light, fog and framebuffer settings
- [ ] glTF 2.0 support
- [ ] CMake build and GitHub Actions CI
