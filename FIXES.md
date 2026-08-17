# FIXES

## 1. Падение на `vkBindBufferMemory` при старте (буфер нулевого размера)

**Файл:** `crates/glvm/src/GraphicAPI/Vulkan.cpp` - `CVulkanRenderer::createBuffer()` (строка ~2413), `CVulkanRenderer::createVertexBuffer()` (~2003), `CVulkanRenderer::createIndexBuffer()` (~2047)

**Ошибка:** при создании вершинного/индексного буфера для меша без геометрии (0 вершин или 0 индексов) вызывалась цепочка `vkCreateBuffer(size=0)` → `vkAllocateMemory(0)` → `vkBindBufferMemory(offset=0 >= allocationSize=0)`. Валидация сообщала `VUID-VkBufferCreateInfo-size-00912`, `VUID-VkMemoryAllocateInfo-allocationSize-07897`, `VUID-vkBindBufferMemory-memoryOffset-01031`; на драйвере AMDVLK это заканчивалось access violation. «Иногда не падает» - зависит от данных (какие меши пустые в данном запуске) и от драйвера.

**Исправление:**
- `createBuffer()` - при `size == 0` размер принудительно становится 1 (спека требует `size > 0`).
- `createVertexBuffer()` / `createIndexBuffer()` - если данных нет, буфер создаётся размером 1 байт, `memcpy` пропускается, в консоль печатается `warning: empty vertex mesh, skipping buffer copy` (помогает найти виновника).

## 2. Access violation при движении WASD (неинициализированный указатель Input_Stack_)

**Файл:** `crates/glvm/src/Engine.cpp` - `Engine::RenderVulkan()` (строка ~261); `crates/glvm/include/glvm/WinApi/WindowWinVulkan.hpp`

**Ошибка:** `WindowWinVulkan::Input_Stack_` - сырой указатель `CStack*`, который нигде не присваивался (единственная строка была закомментирована). На Linux окно использует глобальный `Input_Stack_` (Globals.hpp), поэтому баг не проявлялся. На Windows `HandleEvent()` вызывал `Input_Stack_->ControlInput(_Event)` через мусорный указатель: события мыши не писали в стек (ветка `default`), а WASD вызывали `Push()` → запись в случайный адрес → 0xC0000005.

**Исправление:** раскомментировано присваивание:
```cpp
vulkanRenderer->Window->Input_Stack_ = &Input_Stack_;
```
Окно теперь шлёт события в тот же глобальный стек, что и игровые системы.

## 3. GLTF-модели невидимы на Windows (жёстко зашитый путь к .bin файлам)

**Файл:** `crates/glvm/src/JsonParser.cpp` - `CJsonParser::LoadGLTF()` (строка ~662)

**Ошибка:** бинарные буферы моделей открывались по пути `"../gltf/" + uri`, захардкоженному относительно текущей рабочей директории, а не относительно пути `.gltf` файла. При запуске из `build/debug/examples` (как настроен run-таргет, `WORKING_DIRECTORY = $<TARGET_FILE_DIR>`) этот путь указывает на несуществующий `build/debug/gltf/` → `ifstream::read` молча не читал ничего, а `buffer` (неинициализированный `new char[]`) содержал мусор → вершины/индексы моделей - мусор/нули → вырожденная геометрия, модели невидимы (платформы из процедурно сгенерированных вершин при этом рендерились). На Linux работало только потому, что бинарник запускался из другой директории, где `../gltf/` резолвился в `assets/gltf`.

**Исправление:** путь к `.bin` выводится из пути к `.gltf` (через `find_last_of("/\\")`), плюс явная проверка `is_open()` с исключением - молчаливые мусорные данные исключены:
```cpp
size_t lastSeparator = std::string(pathsGLTF_).find_last_of("/\\");
std::string binary_full_path =
    lastSeparator == std::string::npos
    ? binary_path
    : std::string(pathsGLTF_).substr(0, lastSeparator + 1) + binary_path;
```

## 4. Валидационные ошибки `nestedCommandBufferSimultaneousUse` каждый кадр

**Файл:** `crates/glvm/src/GraphicAPI/Vulkan.cpp` - `CVulkanRenderer::directionalLightRecordCoomandBuffer()` (строка ~4374), `CVulkanRenderer::spotLightRecordCommandBuffer()` (~4514), `CVulkanRenderer::pointLightRecordCommandBuffer()` (~4652)

**Ошибка:** вторичные командные буферы записывались с флагом `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`, но фича `nestedCommandBufferSimultaneousUse` не включена → 12 ошибок валидации `VUID-vkCmdExecuteCommands-nestedCommandBufferSimultaneousUse-09378` каждый кадр. Буферы перезаписываются каждый кадр и исполняются ровно один раз - флаг не нужен.

**Исправление:** флаг `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT` убран (оставлен только `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`).

## 5. Out-of-bounds чтение в `CStack::Pop()` при пустом стеке

**Файл:** `crates/glvm/include/glvm/EventsStack.hpp` - `CStack::Pop()` (строка ~36)

**Ошибка:** `Pop()` возвращал `aStack_[iHead_ - 1]`; при пустом стеке (`iHead_ == 0`) это чтение за границей массива (UB). Вызывается каждый кадр из `CEvent::SetLastEvent()` (Event.cpp) → на Windows могло упасть или прочитать мусор.

**Исправление:** при пустом стеке возвращается `aStack_[0]` (значение `eDEFAULT`, безопасно для switch в `SetLastEvent`):
```cpp
EEvents& Pop() {
    if (iHead_ == 0) {
        return aStack_[0];
    }
    return aStack_[iHead_ - 1];
}
```

## Замечено, но не исправлено (не влияет на текущий запуск)

- `initializeGameLevelVertices()` (Vulkan.cpp) вызывается **каждый кадр** из Engine.cpp и добавляет новые записи в `aVertices_`/`aIndices_`/`vertexBufferContainer`, создавая буферы по индексу `wavefrontObjCounter + gltfCounter + m` - возможна утечка памяти и рассинхрон индексов при смене количества генерируемых мешей.
- `warning: empty vertex mesh` - один из мешей действительно пустой (вероятно, глиф шрифта для символа без геометрии, например `'J'`/`'r'`); с фиксом №1 это безвредно.
