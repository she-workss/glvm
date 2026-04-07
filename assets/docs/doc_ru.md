# Game Loop Versetile Module (GLVM) documentation

## Common engine description

Движок GLVM основан на простой сущностно-компонентной системе (ECS), которая использует разреженные массивы обеспечивающие cache-friendly данные. ECS даёт возможность легко добавлять новые и удалять уже существующие системы, работающие с компонентами. Базовая работа с GLVM основывается на создании трёх супер объектов. Один из них - образец самого движка, и, также, нам нужен менеджер сущностей ("EntityManager") и менеджер компонентов ("ComponentManager"). Болишинство базовых объектов GLVM располагаются в специальных пространствах имён.

## Базовые функции GLVM

### Создание инстанса движка GLVM

This is main super object of GLVM engine. Actualy its engine itself.
Это главный супер объект движка GLVM. По факту это он и есть.

```c++
GLVM::core::Engine* GLVM = GLVM::core::Engine::GetInstance();
```

### Создание инстанса менеджера сущностей

С этим супер-объектом вы можете управлять сущностями.

```c++
GLVM::ecs::EntityManager* EntityManager = GLVM::ecs::EntityManager::GetInstance();
```

### Создание инстанса менеджера компонентов

С помощью этого супер-объекта вы можете управлять компонентами.

```c++
GLVM::ecs::ComponentManager* ComponentManager  = GLVM::ecs::ComponentManager::GetInstance();
```

### Создание сущности

Все сущности в GLVM представлены просто как целочисленные значения. Когда вы вызываете функцию "CreateEntity" необходимо поймать возвращаемое значение, потому что оно помечено аттрибутом [discard].

```c++
Entity uiPlayer = EntityManager->CreateEntity();
```

### Создание компонента

Когда вы создаёте сущность вы можете связать компоненты с ней с помощью вызова функции "CreateComponent<...передайте сюда список компонентов...>(entity)". Эта функция является методом класса "ComponentManager" и она поддерживает передачу лобого количества компонентов в качестве шаблонных параметров. Укажите все компоненты которые необходимы в <> и отправьте целевую сущность, которую хотите наделить указанными копонентами, в качестве аргумента в ().

```c++
GLVM::ecs::ComponentManager->CreateComponent<GLVM::ecs::components::mesh,
                                             GLVM::ecs::components::transform,
                                             GLVM::ecs::components::rigidBody>(entity);
```

### Получить компонент

После того как вы привязали компоненты к какой-либо сущности вы можете получить доступ к любой из них для того чтобы изменить ее данные в своих целях. Это может быть достигнуто с помощью вызова к "GetComponent" функции. Эта функция является методом класса "ComponentManager". При ее вызове отправьте целевую сущность, компонент которой вы хотите получить, в качестве оргумента в () и укажите целевой компонент, привязанный к этой сущности в <>.

```c++
*GLVM::ecs::ComponentManager->GetComponent<cm::transform>(uiPlayer) = {
      .tPosition = { 5.7f, 10.0f, 7.0f }, .fScale = 0.1f };
```

### Загрузка текстур

#### Первый способ

GLVM принимает массивы сырых данных для загрузки текстур. Если вы хотите загрузить типичный формат, такой как PNG или JPG, вам необходимо преобразовать его в сырые данные. Это можно легко сделать с помощью простых инструментов. Таких как ImageMagic и XXD (для XXD необходимо установить Vim). Предположим у нас есть текстура в файле robot.png.

1. Преобразуйте её к .dat формату:

```sh
convert robot.png rgba:robot.dat
```

2. Преобразуйте получившийся файл robot.dat к c-header файлу со статическим массивом внутри:

```sh
xxd -i robot.dat > robot.h
```

Затем создайте новый файл с названием robot.cpp и поместите в него все содержимое файла robot.h. Потом удалите весь код внутри robot.h кроме определения массива и переменной хранящей размер массива, и затем добавьте вначале этих переменных ключевое слово "extern". Например, если мы имеет что-то наподобие этого:

```c++
unsigned char robot_dat[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    .......................................
    0x30, 0x48, 0x2d, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int robot_dat_len = 43997;
```

После всех вышеупомянутых действий должно получиться следующее:

```c++
extern unsigned char robot_dat[];
extern unsigned int robot_dat_len;
```

3. Теперь положите оба файла в директорию "textures":

```sh
mv robot.h /path/to/GLVM/textures
```

4. И затем просто подключите этот хэдер в "sprites_data.hpp":

```c++
#ifndef SPRITES_DATA
#define SPRITES_DATA

#include "robot.h"

#endif
```

Когда вы сделаете все вышеупомянутое нужно узнать ширину и высоту загружаемой текстуры. Это эти данные легко получить с помощью ImageMagick.

5. Просто укажите в качестве аргумента файл robot.dat или robot.png с флагом "identify".

```sh
magick identify robot.png
```

Вы получите простой выхлоп наподобие такого:

```sh
robot.png PNG 120x279 120x279+0+0 8-bit sRGB 44103B 0.000u 0:00.000
```

Нас интересуют цифры между которыми находится символ "x". В данном примере это 120x279. Ширина - 120, и высота - 279.

6. Теперь мы можем вызвать функцию "LoadTextureFromAddress" которая загрузит текстуру в GLV. Первыми вам нужно указать в качестве аргументов ширину, высоту. Затем длинну массива текстуры и сам массив которые находятся в robot.h.

```c++
[[maybe_unused]] ecs::TextureHandle robotTextureHandle = GLVM->load_texture_from_address(120, 279, robot_dat_len, robot_dat);
```

#### Второй вариант

Если первый способ вам показался достаточно трудоёмким то можете загружать текстуры с помощью библиотеки stb_image. Для этого вам нужно просто добавить define и include в файлу исходного кода выбранной графической API:

Для Vulkan.cpp:

```c++
.........
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace GLVM::core
........
```

Для Opengl.cpp:

```c++
.........
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace GLVM::core
........
```

2. Теперь вы можете вызвать функцию "LoadTextureFromFile". Просто укажите ей в качестве аргумента путь до текстуры.

```c++
[[maybe_unused]] ecs::TextureHandle grayTextureHandle = GLVM->LoadTextureFromFile("../textures/data/gray.png");
```

### Загрузка файлов хранящих данные моделей

Вы можете использовать Wavefront.obj и GLTF форматы для загрузки моделей в GLVM. Просто укажите путь к файлу в качестве аргумента при вызове соответствующей функции.

Для Wavefront.obj:

```c++
[[maybe_unused]] cm::MeshHandle coneHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/cone.obj");
```

Для GLTF:

```c++
[[maybe_unused]] cm::MeshHandle megaChelHandle_GLTF = GLVM->load_gltf("../gltf/mega_chel.gltf");
```

### Создание новых систем

GLVM из "коробки" имеет чертыре базовых системы: `CollisionSystem`, `MovementSystem`, `PhysicsSystem`, `ProjectileSystem`. При желании вы моежет создавать свои собственные системы.

1. Добавьте в Engine.hpp указатель на вашу новую систему:

В Engine.hpp:

```c++
class Engine
{
    ......
    ecs::NewSystem* newSystem;
    ......
};
```

2. Инициализируйте указатель и активируйте его с помощью вызова метода `ActivateSystem(newSystem)` класса `SystemManager` в конструкторе класса Engine:

```c++
Engine::Engine() {
    ......
    newSystem          = new ecs::NewSystem();

    pSystem_Manager->ActivateSystem(newSystem);
    ......
};
```

### Выбор библиотеки Xlib или XCB для оконной системы X11 (Только для Linux)

#### Для Vulkan API откройте Vulkan.hpp файл и определите одну из макро-комманд:

```c++
...
#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
//#define VK_USE_PLATFORM_XCB_KHR
#endif
...
```

#### Для Opengl API откройте Opengl.hpp файл и определите одну из макро-комманд:

```c++
...
#ifdef __linux__
//#include "unix_api/window_x_open_gl.hpp"
#include "unix_api/window_xcb_open_gl.hpp"
#endif
...
```

### Выбор типа рендерера

Вы можете выбрать Vulkan или Opengl в качестве рендерера вызвав метод `GameLoop` и передав сообветствующий аргумент:

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

### Загрузка звуковых файлов в wav формате

Звуковые файлы работают в отдельном потоке. Вам необходимо создать инстанс специального класса `CSoundSample` и проинициализировать необходимые поля. Затем вызовите метод `GetSoundContainer` класса `ISoundEngine` и запушьте в него созданный инстанс.

```c++
...
core::Sound::CSoundSample* pSound_Sample = new core::Sound::CSoundSample();
pSound_Sample->kPath_to_File_ = "../laser2.wav";
pSound_Sample->uiDuration_ = 5;
pSound_Sample->uiRate_ = 22050;
soundEngine->GetSoundContainer().Push(pSound_Sample);
...
