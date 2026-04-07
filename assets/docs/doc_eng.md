# Game Loop Versatile Module (GLVM) documentation

## Common engine description

The GLVM engine is based on a simple entity-component system (ECS) that uses sparse arrays to provide cache-friendly data. The ECS offers an opportunity to easily add and remove systems that interact with our components. To get started with GLVM, you need to create three key objects: an instance of the engine itself, an EntityManager, and a ComponentManager for using the ECS system. Most common GLVM objects are located in special namespaces.

## Common GLVM functions

### Creating a new engine instance

This object is the main `super object` of the GLVM engine. In fact, it represents the engine itself.

```c++
GLVM::core::Engine* GLVM = GLVM::core::Engine::GetInstance();
```

### Creating a new entity manager instance

The main purpose of this object is to manage all the entities.

```c++
GLVM::ecs::EntityManager* EntityManager = GLVM::ecs::EntityManager::GetInstance();
```

### Creating a new component manager instance

This key object allows us to manage all the components.

```c++
GLVM::ecs::ComponentManager* ComponentManager  = GLVM::ecs::ComponentManager::GetInstance();
```

### Creating a new entity

All entities in GLVM are stored as integer numbers. When you call the `CreateEntity` function, you have to assign the returned value to a variable because the function is marked with `[discard]`.

```c++
Entity uiPlayer = EntityManager->CreateEntity();
```

### Creating a new component

When you create an entity, you can attach components to it by calling the following function: `CreateComponent<...place your list of components here...>(entity)`. This function is a method of the `ComponentManager` and supports any number of components as template variadic parameters. Place all the components you need within `< >` and pass the target entity within `( )`.

```c++
GLVM::ecs::ComponentManager->CreateComponent<GLVM::ecs::components::mesh,
                                             GLVM::ecs::components::transform,
                                             GLVM::ecs::components::rigidBody>(entity);
```

### Getting a component

After attaching your components to an entity, you can access and modify any of them. This can be achieved by calling the `GetComponent` function, which is a method of the `ComponentManager`. When calling it, place your target entity within `( )` and pass the target component linked to the target entity within `< >`.

```c++
*GLVM::ecs::ComponentManager->GetComponent<cm::transform>(uiPlayer) = {
      .tPosition = { 5.7f, 10.0f, 7.0f }, .fScale = 0.1f };
```

### Loading textures

#### First approach

GLVM can take raw data arrays for loading images. If you want to load some common formats like PNG or JPG, you need to convert it into raw data. This can be easily achieved with simple tools like ImageMagick and XXD (for XXD you need to install Vim). Imagine that we have an image called `robot.png`.

1. Convert it into the `.dat` format:

   ```sh
   convert robot.png rgba:robot.dat
   ```

2. Convert robot.dat into a C header file containing a static array:

   ```sh
   xxd -i robot.dat > robot.h
   ```

   Then, create a new file named `robot.cpp` and copy all the content of `robot.h` into it. Next, remove all the code from `robot.h` except for the declaration of the array and its size, and add the `extern` keyword to them. For example, if we have something like this in `robot.h`:

   ```c++
   unsigned char robot_dat[] = {
       0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
       .......................................
       0x30, 0x48, 0x2d, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
       0x44, 0xae, 0x42, 0x60, 0x82
   };
   unsigned int robot_dat_len = 43997;
   ```

   After all the steps, it should look like the following:

   ```c++
   extern unsigned char robot_dat[];
   extern unsigned int robot_dat_len;
   ```

3. Place it into the `textures` directory:

   ```sh
   mv robot.h /path/to/GLVM/textures
   ```

4. Then, just include the header in "sprites_data.hpp":

   ```c++
   #ifndef SPRITES_DATA
   #define SPRITES_DATA

   #include "robot.h"

   #endif
   ```

When it's done, you need to check the actual height and width of your texture. It's easy to do with ImageMagick.

5. Just pass `robot.dat` or `robot.png` with the "identify" flag.

   ```sh
   magick identify robot.png
   ```

   It will return you a simple output like this:

   ```sh
   robot.png PNG 120x279 120x279+0+0 8-bit sRGB 44103B 0.000u 0:00.000
   ```

   We need the numbers that have `x` between them. In this case, it is 120x279. The width is 120 and the height is 279.

6. Now, we can call the `load_texture_from_address` function, which loads the image into GLVM. You need to specify the width, height, length of the image array, and the actual array contained in `robot.h`.

   ```c++
   [[maybe_unused]] ecs::TextureHandle robotTextureHandle = GLVM->load_texture_from_address(120, 279, robot_dat_len, robot_dat);
   ```

#### Second approach

If you are a bit struggling with the first approach to loading textures, you can use the `stb_image` library. To do that, you need to add the following define and include in the API source of your choice:

For Vulkan.cpp:

```c++
.........
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace GLVM::core
........
```

For Opengl.cpp:

```c++
.........
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace GLVM::core
........
```

2. Now, you can simply call the `LoadTextureFromFile` function. Just specify the path to your texture.

   ```c++
   [[maybe_unused]] ecs::TextureHandle grayTextureHandle = GLVM->LoadTextureFromFile("../textures/data/gray.png");
   ```

### Loading 3D models

You can use Wavefront.obj and GLTF file formats to load your models into GLVM. Just pass the path to your file as an argument to the corresponding function.

For Wavefront.obj:

```c++
[[maybe_unused]] cm::MeshHandle coneHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/cone.obj");
```

For GLTF:

```c++
[[maybe_unused]] cm::MeshHandle megaChelHandle_GLTF = GLVM->load_gltf("../gltf/mega_chel.gltf");
```

### Creating new systems

GLVM from the box have four common systems: `CollisionSystem`, `MovementSystem`, `PhysicsSystem`, `ProjectileSystem`. If you want you can make your own custom systems.

1. Add in Engine.hpp pointer on your new system class:

   In Engine.hpp:

   ```c++
   class Engine
   {
       ......
       ecs::NewSystem* newSystem;
       ......
   };
   ```

2. Initialize pointer and activate it with calling `ActivateSystem(newSystem)` method of `SystemManager` class in constructor of `Engine` class:

   ```c++
   Engine::Engine() {
       ......
       newSystem          = new ecs::NewSystem();

       pSystem_Manager->ActivateSystem(newSystem);
       ......
   };
   ```

### Choosing Xlib or XCB library for X11 window system (For Linux only)

#### For Vulkan API

Open Vulkan.hpp file and define one of macro command:

```c++
...
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
//#define VK_USE_PLATFORM_XCB_KHR
#endif
...
```

#### For Opengl API

Open Opengl.hpp file and define one of macro command:

```c++
...
#ifdef __linux__
//#include "unix_api/window_x_open_gl.hpp"
#include "unix_api/window_xcb_open_gl.hpp"
#endif
...
```

### Choosing renderer type

You can choose Vulkan or Opengl as renderer when calling `GameLoop` method and give it corresponding argument:

1. For Vulkan:

   ```c++
   ...
   ///< Game rendering loop
   GLVM->GameLoop(GLVM::core::VULKAN_RENDERER);
   ...
   ```

2. For Opengl:

   ```c++
   ...
   ///< Game rendering loop
   GLVM->GameLoop(GLVM::core::OPENGL_RENDERER);
   ...
   ```

### Loading sound files in wav format

Sound files working in separate thread. You need to create instance of special class `CSoundSample` and set all necessary fields. Then call `GetSoundContainer` method of `ISoundEngine` class and push to it created instance.

```c++
...
core::Sound::CSoundSample* pSound_Sample = new core::Sound::CSoundSample();
pSound_Sample->kPath_to_File_ = "../laser2.wav";
pSound_Sample->uiDuration_ = 5;
pSound_Sample->uiRate_ = 22050;
soundEngine->GetSoundContainer().Push(pSound_Sample);
...
```
