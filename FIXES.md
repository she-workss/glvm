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

## 6. Инвентарь не открывался на Windows (нет обработки клавиши I)

**Файл:** `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `WindowWinVulkan::MainWndProc()` (WM_KEYDOWN / WM_KEYUP)

**Ошибка:** инвентарь открывается клавишей **I** (`eINVENTORY`), но в Windows-обработчике клавиш не было `VK_I` — на Linux он был (XKEY_I / keysym 105), на Windows нет. Тоггл `isInventoryOpened` (Engine.cpp:313) срабатывал только от `eINVENTORY` в стеке, который Windows-окно никогда не отправляло.

**Исправление:** добавлены `#define VK_I 0x49` и обработка в обоих переключателях:
```cpp
case VK_I:
    pEvent->SetEvent(EEvents::eINVENTORY);        // WM_KEYDOWN
    break;
case VK_I:
    pEvent->SetEvent(EEvents::eINVENTORY_RELEASE); // WM_KEYUP
    break;
```

---

## 7. Курсор-прицел «отзеркаливался» и быстро моргал при открытом инвентаре

**Файл:** `crates/glvm/src/Engine.cpp` — `Engine::updateDataHudScreenUBO()`

**Ошибка:** `hud_screen_x = -hud_screen_x;` мутировал член класса, а `computeHudScreeenCoordinates()` на следующем кадре продолжает прибавлять дельты от уже инвертированного значения → курсор осциллировал между зеркальными позициями каждый кадр (мигание ~30 раз/с). При закрытом инвентаре прицел стоит в центре — баг был невидим; при открытом — явный. Подобранный предмет копирует позицию прицела (ItemSystem.cpp:199-203), поэтому он мигал вместе с ним. На Linux не проявлялось, т.к. там используется Wayland-ветка без инверсии.

**Исправление:** инверсия применяется к локальной копии, член `hud_screen_x` больше не меняется:
```cpp
float hudScreenX = hud_screen_x;
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    hudScreenX = -hud_screen_x;
#endif
```

---

## 8. Инвентарь открывался и моментально закрывался по клавише I

**Файл:** `crates/glvm/src/Engine.cpp` — `Engine::RenderVulkan()`; `crates/glvm/include/glvm/Engine.hpp`

**Ошибка:** тоггл срабатывал от присутствия `eINVENTORY` в стеке и сразу удалял его (`Input_Stack_.Remove(...)`). Повторные `WM_KEYDOWN` (автоповтор клавиши в Windows) снова добавляли событие → инвентарь моргал открыт/закрыт, пока клавиша удерживалась.

**Исправление:** тоггл стал фронтовым (срабатывает один раз на нажатие) через защёлку `isInventoryKeyHeld`; удаление события из стека теперь выполняет `eINVENTORY_RELEASE` (отпускание клавиши) в `CStack::ControlInput()`:
```cpp
bool inventoryKeyPressed = (Input_Stack_.SearchElement(EEvents::eINVENTORY) == EEvents::eINVENTORY);
if (inventoryKeyPressed && !isInventoryKeyHeld) {
    vulkanRenderer->isInventoryOpened = !vulkanRenderer->isInventoryOpened;
    ...
}
isInventoryKeyHeld = inventoryKeyPressed;
```

---

## 9. Краш при развёртывании окна на весь экран («failed to allocate descriptor sets»)

**Файл:** `crates/glvm/src/GraphicAPI/Vulkan.cpp` — `CVulkanRenderer::createMainRenderDescriptorSets()`

**Ошибка:** при ресайзе/максимизации `recreateSwapChain()` повторно вызывал `createMainRenderDescriptorSets()`, которая аллоцировала дескрипторные сеты из того же пула (`maxSets = 10000`), уже занятого первым раундом → `vkAllocateDescriptorSets` падал с `VK_ERROR_OUT_OF_POOL_MEMORY` → `std::runtime_error` → terminate. Раньше не проявлялось: окно было фиксированного размера, свопчейн не пересоздавался.

**Исправление:** сброс пула перед аллокацией:
```cpp
void CVulkanRenderer::createMainRenderDescriptorSets() {
    vkResetDescriptorPool(device, descriptorPool, 0);
    ...
```

---

## 10. Окно на Windows нельзя было ресайзить и разворачивать

**Файл:** `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — конструктор `WindowWinVulkan::WindowWinVulkan()`

**Ошибка:** в стиле окна не было `WS_MAXIMIZEBOX` и `WS_THICKFRAME` — кнопка развёртывания неактивна, рамка не тянулась.

**Исправление:** `WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME`. Свопчейн пересоздаётся по `VK_ERROR_OUT_OF_DATE_KHR`/`VK_SUBOPTIMAL_KHR` (уже было реализовано) — ресайз работает.

---

## 11. Захардкоженные разрешения/аспект 1920×1080 (~1.77) исправлены на реальные размеры окна

**Файлы:**
- `crates/glvm/src/Engine.cpp` — `Engine::SetProjectionMatrix()`: `(float)1920 / (float)1080` → `vulkanRenderer->aspectRate`
- `crates/glvm/src/Engine.cpp` — `Engine::computeHudScreeenCoordinates()`: делители `/ 1920.0f`, `/ 1080.0f` → `/ (float)vulkanRenderer->Window->width/height`
- `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `WindowWinVulkan::CursorLock()`: центр/границы `960, 540, 1911, 1052` → реальная клиентская область (`GetClientRect`)
- `crates/glvm/src/GraphicAPI/Vulkan.cpp` — `CVulkanRenderer::recreateSwapChain()`: `aspectRate` и `Window->width/height` обновляются из фактического размера свопчейна после пересоздания

**Ошибка:** при изменении размера окна (п. 10) проекция, HUD-курсор и замок мыши считались от фиксированных 1920×1080 — сцена растягивалась, прицел отставал от мыши. Также ресайз не обновлял `aspectRate`.

---

## 12. Предметы в инвентаре «мигали»: случайные исчезновения/прыжки (коллизия UBO между кадрами)

**Файл:** `crates/glvm/src/GraphicAPI/Vulkan.cpp` — `CVulkanRenderer::uiIconsRecordCommandBuffer()`

**Ошибка:** индекс UBO-дескриптора для иконок считался как `currentFrame * MAX_FRAMES_IN_FLIGHT + i`, т.е. шаг по кадрам брался равным числу кадров (2), а не числу предметов. При N ≥ 3 предметах слоты UBO кадров 0 и 1 пересекались (предметы 2..N делили одни и те же дескрипторы): пока кадр f ещё исполнялся на GPU, кадр f+1 перезаписывал его UBO моделью другого предмета → предметы случайно «прыгали»/исчезали, иногда все сразу, иногда ни один (при N ≤ 2 коллизий нет). Слоты (UI_PIPELINE) используют правильную формулу с `uiUboDescriptorsNumber` — баг был только в иконках.

**Исправление:** шаг = фактическое число предметов:
```cpp
unsigned int uboIndex = currentFrame * items.GetSize() + i;
```
Потолок — 64 предмета (пул 128 слотов / 2 кадра), для текущей игры с запасом.

---

## 13. Поднятый предмет нельзя было положить в слот или выкинуть (флаг отпускания ЛКМ не выставлялся)

**Файл:** `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `WindowWinVulkan::MainWndProc()` (WM_LBUTTONUP)

**Ошибка:** обработчик `WM_LBUTTONUP` на Windows ставил событие `eMOUSE_LEFT_BUTTON_RELEASE`, но НЕ выставлял `pEvent->isLeftMouseButtonReleased = true` (в X11-обработчике это есть). Флаг инициализирован как `true` (Event.hpp:78), поэтому первый клик поднимал предмет, поднятие сбрасывало флаг в `false`, а отпускание кнопки его больше никогда не возвращало → условия «положить в слот» и «выкинуть» (InventorySystem.cpp:172) никогда не срабатывали — предмет навсегда «висел» на курсоре.

**Исправление:** добавлена недостающая строка, поведение совпадает с X11:
```cpp
case WM_LBUTTONUP:
    pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
    pEvent->isLeftMouseButtonReleased = true;
    return 0;
```

---

## 14. Оставшиеся захардкоженные 1920×1080 / 1.778 в заголовках

**Файлы:**
- `crates/glvm/include/glvm/Systems/InventorySystem.hpp` — `InventorySystem::aspectRate = 1.778` — значение **не перезаписывалось** нигде (в Engine.cpp только читался `vulkanRenderer->aspectRate`), поэтому расчёт слотов инвентаря всегда шёл для соотношения 16:9, даже при окне 1920×1200 (1.6)
- `crates/glvm/include/glvm/Systems/ProjectileSystem.hpp` — `fLast_X/fLast_Y` — **мёртвый код**, нигде не использовался
- `crates/glvm/include/glvm/GraphicAPI/Vulkan.hpp` — комментарий с «must be 1920 / 1080» (значение и так вычисляется из свопчейна)

**Исправление:** `inventorySystem->aspectRate = vulkanRenderer->aspectRate;` добавлен в `Engine::RenderVulkan()` (Engine.cpp, блок инициализации систем, ~стр. 367); дефолт в заголовке заменён на `0.0f` (выставляется каждый кадр); `fLast_X/fLast_Y` удалены; комментарии очищены.

**Не трогали (это стартовые размеры окна, обновляются при ресайзе):** `WindowWinVulkan::width/height`, `WindowWaylandVulkan::width/height`, `xcb_create_window(1920, 1080)`, `WindowXVulkan` конструктор, `WindowWinVulkan.cpp` `_width/_height` — окно реально создаётся 1920×1080, а при ресайзе `recreateSwapChain()` обновляет и `Window->width/height`, и `aspectRate`.

---

## 15. Полная ликвидация захардкоженных разрешений экрана (все платформы)

**Файлы:**
- `crates/glvm/include/glvm/WinApi/WindowWinVulkan.hpp` + `src/WinApi/WindowWinVulkan.cpp` — стартовые `1920×1080` → реальный размер экрана (`GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN)`); при ресайзе размер и так обновляется в `recreateSwapChain()`
- `crates/glvm/src/UnixApi/WindowXVulkan.cpp` (Xlib) — `DisplayWidth/DisplayHeight(pDisp_, XDefaultScreen(pDisp_))`; центр замка мыши `960/540` → `width/2, height/2`
- `crates/glvm/src/UnixApi/WindowXCBVulkan.cpp` — `xcb_create_window(1920, 1080)` → `screen->width_in_pixels/height_in_pixels`; warp-центр → `width/2, height/2`
- `crates/glvm/src/UnixApi/WindowWaylandVulkan.cpp/.hpp` — стартовые `1920×1080` и `previous_X/Y = 960/540` убраны; размер берётся из `wl_output` (новый listener `output_mode`, версия 1); первый дельта-офсет курсора → `-(int)(width/2), -(int)(height/2)`; мёртвые `previous_X/previous_Y` удалены
- `crates/glvm/include/glvm/GraphicAPI/Vulkan.hpp` — мёртвые `WIDTH = 800, HEIGHT = 600` удалены (нигде не использовались)
- `crates/glvm/src/UnixApi/WaylandVariables.cpp` — удалены закомментированные `1920/1080`

**Ошибка:** стартовые размеры окон и центры для замка мыши были зашиты константами 1920×1080/960×540/800×600 — на мониторе другого размера окно открывалось несоразмерно, а курсор при первом движении «прыгал» от неверного центра.

---

## 16. Камера «разворачивалась в случайную сторону» при закрытии инвентаря

**Файлы:** `crates/glvm/src/Engine.cpp` — `Engine::RenderVulkan()` (вызов `CursorLock`, ~стр. 334), `Engine::SetViewMatrix()` (~стр. 506); `crates/glvm/include/glvm/Engine.hpp`

**Ошибка:** `CursorLock()` вызывался каждый кадр, даже при открытом инвентаре: накапливал `offset_X/Y += (курсор − центр)` и варпил курсор в центр. При закрытии инвентаря курсор находился в произвольной точке P (свободно двигался во время перетаскивания предметов) → первый же `CursorLock` после закрытия добавлял в оффсеты скачок `(P − центр)` → `SetViewMatrix()` (крутит камеру от дельт `current − prev`) за один кадр разворачивал камеру на величину этого скачка — «рандомный» разворот, т.к. направление зависит от того, где оставили курсор. Дополнительно `CMovementSystem::CalculateVectorFB()` (тоже крутит камеру по `offset_X`) не работает при открытом инвентаре (система деактивирована) → его `prev_X` протухал, и первый W/S после закрытия давал гигантскую дельту.

**Исправление:**
- `CursorLock()` вызывается только при закрытом инвентаре (курсор свободен во время инвентаря — без дёрганья к центру, перетаскивание работает по реальной позиции курсора).
- При фронте «инвентарь закрылся» (защёлка `wasInventoryOpened` в Engine.hpp) оффсеты мыши и `prev_X/prev_Y/current_X/current_Y` (`vulkanRenderer` и `movementSystem->prev_X`) обнуляются — первый «залоченный» сэмпл отбрасывается, камера не прыгает.
- `SetViewMatrix()` не считает дельты вращения, пока инвентарь открыт (delta = 0 → поворот не применяется, `prev` продолжает синхронизироваться).

---

## 17. Прицел замер на месте при открытом инвентаре + «рандомное» начальное положение

**Файлы:** `crates/glvm/src/Engine.cpp` — `Engine::RenderVulkan()` (тоггл инвентаря, ~стр. 319), `Engine::computeHudScreeenCoordinates()` (~стр. 2188)

**Ошибка:** после фикса №16 (отключение `CursorLock` при открытом инвентаре) координаты прицела `hud_screen_x/y` перестали обновляться: они интегрируются из дельт `offset_X/Y`, а те накапливаются только в `CursorLock` — при открытом инвентаре оффсеты заморожены → прицел неподвижен (и перетаскивание предметов сломано). Плюс при каждом открытии прицел стартовал из старого «накопленного» положения.

**Исправление:**
- При открытии инвентаря `hud_screen_x/y` сбрасываются в 0 (центр экрана) — предсказуемый старт.
- `computeHudScreeenCoordinates()` при открытом инвентаре считает прицел от **реальной позиции курсора** (`position_X/Y`, свободная мышь) в NDC, а не от оффсетов; при закрытом — как раньше (оффсеты). `previousMouseOffsetX/Y` синхронизируются в обоих ветках, скачка при закрытии нет.
- Знак X в позиционной ветке инвертирован (`1.0f − position_X / (width/2)`): старая оффсетная ветка накапливала X со встроенной инверсией, а `updateDataHudScreenUBO` для не-Wayland ещё раз отрицает X — иначе курсор двигался зеркально по горизонтали (Y при этом корректен).
- Wayland-ветка не тронута (`#ifndef VK_USE_PLATFORM_WAYLAND_KHR` вокруг пропуска `CursorLock` и сброса при закрытии) — там относительный указатель, поведение прежнее.

---

## 18. Курсор не должен лочиться при потере фокуса окна (alt-tab), все платформы

**Файлы:** `crates/glvm/include/glvm/IWindow.hpp` (флаг `isFocused`), `src/WinApi/WindowWinVulkan.cpp` (WM_SETFOCUS/WM_KILLFOCUS), `src/UnixApi/WindowXVulkan.cpp` (FocusIn/FocusOut + ungrab), `src/UnixApi/WindowXCBVulkan.cpp` (XCB_FOCUS_IN/OUT + ungrab), `src/UnixApi/WindowWaylandVulkan.cpp` (keyboard_enter/leave), `src/Engine.cpp` (условие вызова `CursorLock`)

**Ошибка:** `CursorLock()` вызывался каждый кадр независимо от того, в фокусе ли окно: при alt-tab игра продолжала `SetCursorPos`-варпить курсор в центр экрана, не давая пользоваться мышью в других приложениях. На X11/XCB дополнительно активен `XGrabPointer` — захватывает события мыши даже над чужими окнами.

**Исправление:** в `IWindow` добавлен флаг `isFocused` (по умолчанию `true`), каждый бэкенд обновляет его по событиям фокуса:
- Windows: `WM_SETFOCUS`/`WM_KILLFOCUS` (через статический `WindowWinVulkan::instance`)
- Xlib: `FocusIn`/`FocusOut` (+`FocusChangeMask`) — при потере фокуса `XUngrabPointer`, при возврате повторный `XGrabPointer`
- XCB: `XCB_FOCUS_IN`/`XCB_FOCUS_OUT` (+`XCB_EVENT_MASK_FOCUS_CHANGE`) — `xcb_ungrab_pointer`/повторный grab
- Wayland: `keyboard_enter`/`keyboard_leave` (аналога абсолютного варпа нет — относительный указатель и так приходит только при фокусе)

`CursorLock` вызывается только при `isFocused` (и закрытом инвентаре). При возврате фокуса позиция мыши ≈ центр (был залочен), дельты нулевые — скачка нет.

---

## 19. Серый экран (мир невидим) после правок замка мыши + чистка флага `discardNextSample`

**Файлы:** `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `WindowWinVulkan::CursorLock()`; `crates/glvm/include/glvm/GraphicAPI/Vulkan.hpp` (строка `vec3 forward`)

**Ошибка (часть 1 — дрейф):** дельта вращения считалась как `(позиция курсора − вычисленный центр)`, а после `SetCursorPos` фактическая позиция курсора на Windows на пару пикселей не совпадала с вычисленным центром (округление DPI). Эта постоянная ошибка ~2px накапливалась в `offset_Y` каждый кадр → упиралась в кламп 890 → камера за ~время уводилась питчем на ~89° в небо → экран становился серым. «Обычно работает» — только потому, что начальная позиция курсора давала стартовый поворот.

**Ошибка (часть 2 — серый экран с первого кадра):** `vulkanRenderer->forward` — `vec3 forward;` (Vulkan.hpp) → `{0,0,0}`. `SetViewMatrix()` каждый кадр безусловно делает `cameraComponent->forward = Normalize(vulkanRenderer->forward)`, затирая валидный `forward = {0,0,-1}`, заданный сущности игрока (hello_world.cpp). Пока дрейф существовал, первый же не-нулевой поворот камеры «бутстрапил» `forward` в не-нулевое значение и мир рендерился. После устранения дрейфа поворотов не происходило вовсе → `forward` навсегда `{0,0,0}` → вырожденная view-матрица → мир невидим (инвентарь/HUD рендерятся отдельными UBO и были видны).

**Исправление:**
- `CursorLock()` переписан без флага `discardNextSample` (хак удалён из `IWindow.hpp`, `WindowWinVulkan.cpp` и close-edge в `Engine.cpp`): дельта каждого кадра меряется от **фактической позиции курсора после предыдущего варпа** (read-back через `GetCursorPos` + `ScreenToClient`), а не от вычисленного центра → при неподвижной мыши дельта ровно 0, дрейфа нет. Прыжок дельты >250px между кадрами — это телепорт курсора (старт, возврат фокуса, первый сэмпл после закрытия инвентаря) → сэмпл отбрасывается, выполняется только повторный варп.
- `vec3 forward = {0.0f, 0.0f, -1.0f};` — стартовое направление камеры совпадает с `beholder.forward` игрока, мир виден с первого кадра без движения мыши.

---

## 20. Персонаж «иногда» проваливался сквозь платформы (коррупция spatial grid)

**Файлы:** `crates/glvm/src/Systems/SpatialGridSystem.cpp` — `SpatialGridSystem::Update()` (строки ~50-59); `crates/glvm/src/Systems/MovementSystem.cpp` — `CMovementSystem::Update()` (строки ~124-127)

**Ошибка:** в spatial grid при перерегистрации сущности удаление из старых ячеек шло по сохранённому индексу `entityLocation.cellEntityIndices[i2]`. Этот индекс устаревает, как только из ячейки удаляют любую другую сущность (вектор ячейки сдвигается вниз) → `Remove(индекс)` удалял чужую сущность или уходил за границы. Дополнительно цикл удаления проходил по `maxGridCellNumber` (8), а не по фактическому `gridCellCounter` — слоты `gridCellIndicies[1..7]`, оставшиеся от предыдущего положения сущности (не обнуляемые при перерегистрации), тоже срабатывали и «вычищали» сущности из случайных ячеек. Итог: из ячеек исчезали статичные чанки-платформы, `CollisionSystem` переставал их находить → игрок проваливался под пол. «Иногда» — потому что зависело от пути игрока (какие ячейки разделялись с другими сущностями и какие устаревшие слоты попадали под удаление). Дополнительно `entityLocation.isDirty = true;` для rigid bodies при движении по гравитации был закомментирован — падающие сущности не перерегистрировались в сетке, пока не нажималась клавиша WASD.

**Исправление:**
- Удаление из ячеек — только по фактическим ячейкам (`i2 < gridCellCounter`) и **по значению** (поиск сущности в векторе ячейки), а не по сохранённому индексу — устаревший индекс больше не может удалить чужую сущность:
  ```cpp
  for (u32 i2 = 0; i2 < entityLocation.gridCellCounter; ++i2) {
      u32 z = entityLocation.gridCellIndicies[i2][0];
      u32 y = entityLocation.gridCellIndicies[i2][1];
      u32 x = entityLocation.gridCellIndicies[i2][2];
      core::vector<u32>& chunkEntities = spatialGrid.grid[z][y][x].entities;
      for (u32 i3 = 0; i3 < chunkEntities.GetSize(); ++i3) {
          if (chunkEntities[i3] == entity) {
              chunkEntities.Remove(i3);
              break;
          }
      }
  }
  entityLocation.gridCellCounter = 0;
  ```
- Раскомментировано `entityLocation.isDirty = true;` в цикле rigid bodies (`MovementSystem.cpp:124-127`) — сущности с гравитацией корректно перерегистрируются в сетке каждый кадр, в том числе при падении без ввода.

---

## 21. Персонаж проходил сквозь большинство кубов-стен сбоку (коллайдер всегда читался по индексу 0)

**Файл:** `crates/glvm/src/Systems/PhysicsSystem.cpp` — обработка `colliders[i].colliders` в цикле walls (строка ~189)

**Ошибка:** в `PhysicsSystem` после разрешения `entityLocations` брались **базовые указатели** массивов компонентов, но чтение шло без индексации `[collidedIndex]`:
```cpp
cm::transform* collidedTransform = (cm::transform*)collidedArch->components[TRANSFORM_COMPONENT];
...
const vec3 collidedPosition = collidedTransform->position; // всегда элемент [0]
```
`collidedTransform->position` и `collidedMesh->handle.id` всегда читали элемент **индекса 0** архитипа независимо от того, с какой сущностью коллизия. У всех пяти кубов (id 6–10) в архитипе `StaticMeshArchetype` первым лежит куб 6 с позицией `(7,2,10)`, поэтому:
- куб 6 «работал» — он и есть index 0, его позиция совпадает;
- кубы 7–10 проверялись по боксу куба 6 → `aabbOverlap` не находил пересечения → `frameMovement` не занулялся → проход сквозь стену;
- платформа-чанк (id 21) тоже разрешалась, но она в своём архитипе стоит на индексе 0 → читалась верно, баг не проявлялся.

В `CollisionSystem` эта же сущность читалась корректно (`&(transform*)arch->components[TRANSFORM][comparedEntityIndex]`) — поэтому в логе для одного и того же куба collision показывал `pos=(7,2,14)`, а физика `pos=(7,2,10)`.

**Исправление:** после null-проверки базовые указатели сдвигаются на `collidedIndex`:
```cpp
if (!collidedTransform || !collidedMesh) {
    continue;
}
collidedTransform += collidedIndex;
collidedMesh += collidedIndex;
```

---

## 22. Assert в spatial grid при прыжке (`indexMin* < width/height/depth`)

**Файлы:** `crates/glvm/src/Systems/SpatialGridSystem.cpp` (строки ~89-114), `crates/glvm/src/Systems/CollisionSystem.cpp` (строки ~131-158)

**Ошибка:** индексы ячейки сетки считались в `u32`:
```cpp
const u32 indexMinX = (minEntityPosition[0] + halfWidth) / chunkSize;
```
и три ассерта требовали, чтобы сущность всегда находилась внутри фиксированной сетки 64×64×64 (мир ±1024 с шагом 32). Но сущность может легитимно покинуть сетку (упасть с края мира, снаряд улетел) — тогда `(min + halfWidth)/chunkSize` уходит за `[0, 63]`, а при отрицательном положении (min < −halfWidth) деление даёт отрицательное число, которое при касте в `u32` заворачивается в ~4·10⁹ → ассерт `indexMin* < width/height/depth` падает (краш игры, например, при прыжке/падении сквозь край мира).

**Исправление:** индексы считаются как `int` и клампятся в `[0, размер−1]` — сущность вне сетки регистрируется в крайней ячейке вместо падения:
```cpp
int indexMinX = (int)((minEntityPosition[0] + halfWidth) / chunkSize);
...
indexMinX = std::clamp(indexMinX, 0, (int)spatialGrid.width - 1);
```
(аналогично для min/max по всем осям в обоих системах; `#include <algorithm>` добавлен).

---

## 23. Нельзя посмотреть мышью ровно вниз/вверх (pitch упирался в ~45°, а не в 89°)

**Файлы:** `crates/glvm/src/Engine.cpp` — `Engine::SetViewMatrix()` (rotor `exp(...)`, ~стр. 602, кламп питча ~стр. 619); `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `CursorLock()`; `crates/glvm/src/UnixApi/WindowXVulkan.cpp` — тот же `CursorLock()`

**Ошибка:** угол поворота камеры делился на 2 **дважды**: `exp(rotationAngle * quatAngleCorrection, ...)` с `quatAngleCorrection = 0.5` и внутри `exp()` (`PGA.hpp:592` — `sin(theta / 2.0f)`). Итоговая чувствительность была `angleScale × 0.5` вместо `angleScale`: при `angleScale = 0.1` реально 0.05°/px, а кламп `offset_Y` = ±890px давал максимум **44.5°** по питчу — «не вертикально». Рабочий диапазон камеры по вертикали всё это время был ~45°, а не 89°.

**Первая (неверная) попытка:** поднят `angleScale 0.1 → 0.2` и кламп 890→449. Это дало чувствительность 0.1°/px (мышь стала «резкой», вдвое быстрее) и максимум 449×0.1 = 44.9° — диапазон не изменился, жалоба осталась.

**Исправление:**
- Убран лишний множитель `quatAngleCorrection` из вызова ротора — теперь поворот = `px × angleScale`, как и задумано; `angleScale` возвращён к 0.05°/px (привычная пользователю скорость, бывшая до этого фактически).
- Лимит питча перенесён из пиксельного клампа окна **в движок, по углу** (`maxPitchSin = sin(89.95°)` применяется к `forward[1]` после каждого поворота). Это убирает хардкод `1799px` из окон — ограничение больше не зависит ни от разрешения экрана, ни от чувствительности мыши, ни от `angleScale`. Ровно 90° не ставится: при `forward == (0, ±1, 0)` векторы `rightVec`/`newUpVec` вырождаются в ноль → `rotateAxis = 0` → вращение останавливается и камера «залипает» на полюсе, а базис `Cross(forward, up)` в `lookAtRH` становится неопределённым. Клампы `1799px` удалены из `WindowWinVulkan.cpp` и `WindowXVulkan.cpp`.

---

## 24. Краш при «убийстве» робота (удалённая сущность оставалась в spatial grid и в списках коллайдеров)

**Файлы:** `crates/glvm/src/ArchetypedECS/ArchECS_World.cpp` — `World::removeEntity()`; `crates/glvm/src/Systems/CollisionSystem.cpp` — `CCollisionSystem::Update()` (~стр. 190); `crates/glvm/src/Systems/PhysicsSystem.cpp` — `CPhysicsSystem::Update()` (~стр. 153)

**Ошибка:** когда робот умирал (`DamageSystem` доводил `currentHealth <= 0` и вызывал `removeEntity`), сущность удалялась из мира, но её следы оставались в двух местах:
- **spatial grid:** удалённая сущность продолжала числиться в ячейках `spatialGrid.grid[z][y][x].entities` — её «вычищал» только следующий `SpatialGridSystem::Update()`, а до этого кадра `CollisionSystem` мог прочитать `entityLocations[id].arch` уже удалённой сущности → `nullptr` → разыменование → краш;
- **списки коллайдеров:** у живых сущностей в `colliders[i].colliders` оставался `entity` робота, убитого **в том же кадре** (порядок систем: spatialGrid → collision → damage → physics): `DamageSystem` удалял робота, а следом `PhysicsSystem` проходил по его collider-списку, читал `arch->components[...]` у удалённой сущности → доступ к освобождённому/нулевому указателю.

**Исправление:**
- `World::removeEntity()` теперь сам удаляет сущность из всех ячеек spatial grid, где она была зарегистрирована (по сохранённым `gridCellIndicies`, удаление по значению), и обнуляет `gridCellCounter`.
- `CollisionSystem` и `PhysicsSystem` перед обращением к найденной сущности проверяют `location.arch == nullptr` и пропускают её (сущность удалена в этом кадре после детекта коллизии) — вместо разыменования.

---

## 25. Краш в первые кадры: `vkMapMemory` — offset больше размера font-UBO буфера (0xc2f40 > 0xc0000)

**Файл:** `crates/glvm/src/GraphicAPI/Vulkan.cpp` — `CVulkanRenderer::fontRecordCommandBuffer()` (инициализация `currentActorMemoryOffset`, ~стр. 3323, и аккумуляция ~стр. 3437)

**Ошибка:** смещение внутри font-UBO буфера накапливалось как
```cpp
currentActorMemoryOffset += currentFrame * fontUboDescriptorNumber + font.font_string.GetSize();
```
т.е. множитель `currentFrame * fontUboDescriptorNumber` прибавлялся **на каждый шрифт**. При `currentFrame = 1` (второй кадр in-flight) и 34 шрифтах это давало ~34×128 = 4352 слотов — буфер FONT_RENDER_UBO рассчитан на 4096 (`hostDescriptorNumber = 4096`). Валидация: `vkMapMemory(): offset 0xc2f40 larger than total array size 0xc0000` (0xc2f40 = 4159 × 192 = sizeof(FONT_UBO)), далее `vkQueueSubmit` падал → `std::runtime_error` → terminate в первые кадры. «Иногда запускалось» — при `currentFrame = 0` множитель нулевой и утечки нет.

**Исправление:** базовая раскладка слотов по кадрам выставляется один раз при инициализации, в цикле накапливается только длина строки (как и в соседних UBO: `currentFrame * descriptorNumber + i`, см. `updateMatrixUniformBuffer`):
```cpp
unsigned int currentActorMemoryOffset = currentFrame * fontUboDescriptorNumber;
...
currentActorMemoryOffset += font.font_string.GetSize();
```

---

## 26. Esc приходилось нажимать несколько раз, чтобы освободить курсор (двойной тоггл)

**Файл:** `crates/glvm/src/WinApi/WindowWinVulkan.cpp` — `WindowWinVulkan::HandleEvent()` (цикл `PeekMessage`)

**Ошибка:** событие `eCURSOR_RELEASED` ставилось в общий объект `CEvent` (`g_eEvent`), который **никогда не сбрасывался** после обработки сообщения. `TranslateMessage` на каждое нажатие клавиши порождает `WM_CHAR`, который диспатчится тем же `HandleEvent` (или следующим кадром) и не трогает поле события — в `ControlInput` попадало всё то же залипшее `eCURSOR_RELEASED`. После того как игровой цикл уже снял его из стека, лишний `WM_CHAR` снова пушил `eCURSOR_RELEASED` в стек → тоггл срабатывал **дважды за одно нажатие** (0→1→0): курсор освобождался и тут же снова захватывался, поэтому Esc приходилось жать по несколько раз.

**Исправление:** поле события сбрасывается в `eDEFAULT` после каждой итерации `ControlInput` — залипшее значение не может пережить сообщение и повторно дёрнуть тоггл:
```cpp
while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    Input_Stack_->ControlInput(_Event);
    _Event.SetEvent(EEvents::eDEFAULT);
}
```
Проверено: одно нажатие Esc = один тоггл (0→1 — освобождение, 1→0 — повторный захват).

---

## 27. Робот не получал урон от снарядов (баундинг-бокс не совпадал с моделью при масштабе 0.02)

**Файлы:** `crates/glvm/src/Common/CommonFunctions.cpp` — `BoxCollider()`, `computeBoxCornerBoundPoints()`; `crates/glvm/src/Systems/CollisionSystem.cpp` — `UpperActorCheck()`; `crates/glvm/src/Systems/PhysicsSystem.cpp` — `aabbOverlap()`, `isAbove()`

**Ошибка:** во всех коллизионных проверках центр бокса считался как `position + origin_offset` (offset не умножался на scale), а визуальный бокс в отладчике (`ImGuiOverlay`, через `modelMatrix` = `position + scale*local`) — как `position + scale*origin_offset`. Для моделей с масштабом 1 они совпадают (киборг/плеер/чанки), поэтому баг не проявлялся. У робота (`scene.gltf`, `scale = 0.02`) большой `origin_offset` не уменьшался масштабом: коллизионный бокс стоял далеко от визуальной модели → снаряд «пролетал» сквозь робота (отладчик показывал красный бокс, т.к. считает по `modelMatrix`), `BoxCollider` возвращал `false`, урон не наносился, робота нельзя было убить.

**Исправление:** `origin_offset` умножается на `scale` во всех коллизионных проверках — бокс совпадает с визуальной моделью:
```cpp
// BoxCollider / aabbOverlap / isAbove / UpperActorCheck:
position + origin_offset_* * scale ± absolute_* * scale
```
Проверено: снаряды наносят урон роботам, враги умирают (в логе `remove entity`).

---

## Замечено, но не исправлено (не влияет на текущий запуск)
- `initializeGameLevelVertices()` (Vulkan.cpp) вызывается **каждый кадр** из Engine.cpp и добавляет новые записи в `aVertices_`/`aIndices_`/`vertexBufferContainer`, создавая буферы по индексу `wavefrontObjCounter + gltfCounter + m` - возможна утечка памяти и рассинхрон индексов при смене количества генерируемых мешей.
- `warning: empty vertex mesh` - один из мешей действительно пустой (вероятно, глиф шрифта для символа без геометрии, например `'J'`/`'r'`); с фиксом №1 это безвредно.
