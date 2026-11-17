# Building & troubleshooting

## Requirements

| | |
|---|---|
| OS | Windows 10 / 11, **x64** |
| IDE | Visual Studio 2022 (platform toolset **v143**) with *Desktop development with C++* |
| GPU | OpenGL **3.3 Core Profile** or newer |

The prebuilt libraries in `libraries/` are **static x64 MSVC** builds, so only the x64 platform links. Win32/x86, MinGW, Linux and macOS would need the dependencies rebuilt (see [known-issues.md](known-issues.md#portability)).

## Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/miguelrg14/VS_3D-Renderer.git
   ```
2. Open the solution you want:
   - `01_Geometric-figures/project/UDIT-4º_PG_P1_Figuras-geométricas.sln`
   - `02_Scene-render/project/UDIT-4º_PG_P2_Renderizar-escena.sln`
3. Select **x64** and **Debug** or **Release**.
4. Build and run (F5).

The project files already point at the vendored dependencies with relative paths:

```text
Include:  ../../libraries/{assimp,glad,glm,half,sdl,soil2}/include
Lib:      ../../libraries/assimp/lib/windows/visual-studio-2022/static-x64
          ../../libraries/glad/lib/windows/visual-studio-2022/static-x64
          ../../libraries/sdl/lib/windows/visual-studio-2022/static-x64
          ../../libraries/soil2/lib/windows/visual-studio-2019/static-x64
Link:     SDL2-static(d).lib  SDL2main(d).lib  gladd.lib  opengl32.lib
          imm32.lib setupapi.lib version.lib winmm.lib
          assimp-vc143-mt(d).lib  zlibstatic(d).lib  soil2(-debug).lib   ← project 02 only
```

## Working directory and assets

Project 02 loads every asset with a path relative to the **working directory**:

```text
../assets/sky-cube-map-0.png … sky-cube-map-5.png
../assets/height-map.png
../assets/Terreno.obj, Painting.obj, Plant.obj, Umbrella.obj  (+ their textures)
```

Visual Studio's default working directory is `$(ProjectDir)`, which is `02_Scene-render/project/`, so `../assets` resolves correctly when you run from the IDE. To run the built `.exe` on its own, launch it from a folder that sits next to `assets/`, or copy `assets/` one level above the executable.

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Unhandled `std::runtime_error: No se pudo cargar mesh` on startup | The `.obj` models are missing (see [known-issues.md](known-issues.md#obj-models-are-not-in-the-repository)) or the working directory is wrong | Add the models to `02_Scene-render/assets/` and run from the IDE |
| Black skybox or textures | Images not found relative to the working directory | Check *Project → Properties → Debugging → Working Directory* = `$(ProjectDir)` |
| `LNK2038` / `LNK1112` link errors | Building Win32, or a different toolset | Use **x64** and toolset **v143** |
| Very pixelated output | Intended: the scene renders into a 256×256 framebuffer | Raise `framebuffer_width/height` in `Scene.hpp` |
| Everything is sepia | Intended post-process | Use the commented-out line in `effect_fragment_shader_code` |
| Window opens and closes immediately | OpenGL 3.3 context could not be created | Update GPU drivers. Check that the machine isn't falling back to a basic display adapter (remote desktop, VM). |
