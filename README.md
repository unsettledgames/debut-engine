# debut-engine
Simple, work in progress, learning-oriented C++ graphics engine.
![image](https://user-images.githubusercontent.com/47360416/188104677-eedcb60b-7352-4230-a2d9-58c64f04b379.png)

![image](https://user-images.githubusercontent.com/47360416/171034625-d7da8f08-74dc-4685-b8e9-a207f5f43b2b.png)

Build instructions (Windows only atm, latest branch is `directional-point-lights`):
- Visual Studio 2019 or above required, CMake required
- Clone this repository
- Run `git checkout --latest_branch_name--`, where --latest_branch_name-- is the name written above.
- Run `git submodule init`
- Run `git submodule update`
- Run the relevant script in the scripts folder. Win-GenMSVC19.bat generates a Visual Studio 2019 solution, Win-GenMSVC22.bat generates a Visual Studio 2022 solution.
- Open DebutEngine.sln, right click on Debutant, click on "Set as startup project"
- Compile the solution
- Copy the Resources/assets folder in Build/Debutant/Build
- If you want to open the default projects too, move the Lib and Debut folders there as well. 

Yes, I should really add some CMake stuff to automatically copy things. I should also probably make a proper installer and put the Resources in a better place. 

TODO List:
- Architecture
  - Multithreading
  - Vulkan
  - SPIR-V
- 2D
  - Tilemaps (editor too)
- Physics
  - 3D physics
  - Triggers
- Scripting
  - Lua
  - C#
- VFX
  - Particle systems
- Rendering
  - Culling
  - PBR rendering
  - Deferred rendering
  - LODs
    - Veeery ambitious but I'd like to have a look at Nanite
  - Fog
- Animation
  - 2D Skeletal
  - 2D Traditional
  - 3D
- AssetManager
  - Release resources when not used
  - Detect file system changes?
- UI system
  - Probably use an external library
- Utility
  - Prefabs
  - Event system
- Editor
  - Camera preview
  - Debug Console
  - Shader / material graph
  - Sprite slicing
  - Move files (don't break the AssetManager while doing so)
  - Terrain editor :O
- Debugging
  - Profiler
  - Memory usage measuring (custom allocators, GC)
