#include "glvm/win_api/window_win_vulkan.hpp"

#include "glvm/event.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
);

#include <iostream>
#include <iterator>

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_I 0x49

namespace glvm::core {
WindowWinVulkan* WindowWinVulkan::instance = nullptr;

WindowWinVulkan::WindowWinVulkan() {
    instance = this;
    const char* _title = "Game";
    int _width = width / 2, _height = height / 2;
    // Register the window class for the main window.
    window_Class_.style = 0;
    window_Class_.lpfnWndProc = MainWndProc;
    window_Class_.cbClsExtra = 0;
    window_Class_.cbWndExtra = 0;
    window_Class_.hInstance = NULL;
    window_Class_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_Class_.hCursor = LoadCursor(NULL, NULL);
    window_Class_.hbrBackground = NULL;
    window_Class_.lpszMenuName = NULL;
    window_Class_.lpszClassName = "Game";

    RegisterClassA(&window_Class_);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX | WS_THICKFRAME;

    RECT rect;
    SetRect(&rect, 0, 0, _width, _height);
    AdjustWindowRect(&rect, style, FALSE);
    int _x_position = (width - (rect.right - rect.left)) / 2;
    int _y_position = (height - (rect.bottom - rect.top)) / 2;

    // Create the main window.
    pModern_Window_ = CreateWindowA(
        "Game",
        _title,
        style,
        _x_position,
        _y_position,
        rect.right - rect.left,
        rect.bottom - rect.top,
        (HWND)NULL,
        (HMENU)NULL,
        NULL,
        (LPVOID)NULL
    );

    // Show the window and paint its contents.
    ShowWindow(pModern_Window_, SW_SHOWDEFAULT);
    UpdateWindow(pModern_Window_);
}

void WindowWinVulkan::SwapBuffers() {}

void WindowWinVulkan::ClearDisplay() {}

bool WindowWinVulkan::HandleEvent(CEvent& _Event) {
    // Create message struct object.
    MSG msg;

    SetWindowLongPtrW(pModern_Window_, GWLP_USERDATA, (LONG_PTR)&_Event);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Input_Stack_->ControlInput(_Event);
        // DispatchMessage may not have set the event (e.g. a WM_CHAR left
        // over from TranslateMessage, or WM_KEYUP for an unhandled key). The
        // stale value would otherwise be re-pushed by ControlInput on the next
        // message/frame and toggle the cursor a second time.
        _Event.SetEvent(EEvents::eDEFAULT);
    }
    return false;
}

void WindowWinVulkan::Close() {
    DestroyWindow(pModern_Window_);
    PostQuitMessage(0);
}

HWND WindowWinVulkan::GetClassicWindowHWND() {
    return pClassic_Window_;
}

HWND WindowWinVulkan::GetModernWindowHWND() {
    return pModern_Window_;
}

void WindowWinVulkan::CursorLock(
    int _x_position,
    int _y_position,
    int* _x_offset,
    int* _y_offset
) {
    RECT clientRect;
    GetClientRect(pModern_Window_, &clientRect);
    const int centerX = clientRect.right / 2;
    const int centerY = clientRect.bottom / 2;
    POINT point_position {centerX, centerY};
    ClientToScreen(pModern_Window_, &point_position);
    // Solve a problem with endlessly growing numbers in the start game run.
    if (_x_position > clientRect.right || _x_position < 0
        || _y_position > clientRect.bottom || _y_position < 0) {
        return;
    }
    int iOffset_X = 0, iOffset_Y = 0;
    iOffset_X = _x_position - previous_X;
    iOffset_Y = _y_position - previous_Y;
    previous_X = _x_position;
    previous_Y = _y_position;

    // The per-frame delta is measured against the cursor's actual position
    // after the previous warp, not against the computed center: the two can
    // differ by a few pixels (DPI rounding), and accumulating that constant
    // error would slowly drift the view until it hits the pitch clamp below.
    // A >250px jump between frames is a cursor teleport, not mouse movement:
    // discard it so the camera doesn't snap toward the new position (startup,
    // refocus, stale sample after the warp, and the first sample after the
    // inventory closes - the cursor was free while the inventory was open).
    // Discard the sample, just re-warp to the center.
    if (iOffset_X > 250 || iOffset_X < -250 || iOffset_Y > 250
        || iOffset_Y < -250) {
    } else {
        *_x_offset += iOffset_X;
        *_y_offset -= iOffset_Y;
    }
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    SetCursorPos(point_position.x, point_position.y);
    SetCursor(NULL);
    // Baseline for the next frame: where the cursor actually ended up after the
    // warp (matches what the next WM_MOUSEMOVE will report).
    POINT actual_position;
    GetCursorPos(&actual_position);
    ScreenToClient(pModern_Window_, &actual_position);
    previous_X = actual_position.x;
    previous_Y = actual_position.y;
}

// Callback method for events handling.
LRESULT CALLBACK WindowWinVulkan::MainWndProc(
    HWND _pHwnd,
    UINT _pMsg,
    WPARAM _pWParam,
    LPARAM _pLParam
) {
    ImGui_ImplWin32_WndProcHandler(_pHwnd, _pMsg, _pWParam, _pLParam);
    CEvent* pEvent = (CEvent*)GetWindowLongPtrW(_pHwnd, GWLP_USERDATA);

    if (_pMsg == WM_KEYDOWN && _pWParam == VK_ESCAPE && pEvent != nullptr
        && (_pLParam & (1 << 30)) == 0) {
        pEvent->SetEvent(EEvents::eCURSOR_RELEASED);
        return 0;
    }

    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        const bool isMouseMessage =
            (_pMsg >= WM_MOUSEFIRST && _pMsg <= WM_MOUSELAST)
            || _pMsg == WM_MOUSEWHEEL || _pMsg == WM_MOUSEHWHEEL;
        const bool isKeyboardMessage =
            (_pMsg >= WM_KEYFIRST && _pMsg <= WM_KEYLAST) || _pMsg == WM_CHAR
            || _pMsg == WM_SYSCHAR || _pMsg == WM_SYSKEYDOWN
            || _pMsg == WM_SYSKEYUP;
        if ((isMouseMessage && io.WantCaptureMouse)
            || (isKeyboardMessage && io.WantCaptureKeyboard)) {
            return 0;
        }
    }

    if (pEvent == nullptr) {
        return DefWindowProcA(_pHwnd, _pMsg, _pWParam, _pLParam);
    }
    int iMouse_Position_X, iMouse_Position_Y;
    switch (_pMsg) {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            return 0;
        case WM_LBUTTONDOWN:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
            return 0;

        case WM_SETFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->isFocused = true;
            }
            return 0;
        case WM_KILLFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->isFocused = false;
            }
            return 0;
        case WM_LBUTTONUP:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
            pEvent->isLeftMouseButtonReleased = true;
            return 0;
        case WM_MOUSEMOVE:
            iMouse_Position_X = GET_X_LPARAM(_pLParam);
            iMouse_Position_Y = GET_Y_LPARAM(_pLParam);
            pEvent->SetEvent(EEvents::eMOUSE_POINTER_POSITION);
            pEvent->mousePointerPosition.position_X = iMouse_Position_X;
            pEvent->mousePointerPosition.position_Y = iMouse_Position_Y;
            return 0;
        case WM_KEYDOWN:
            switch (_pWParam) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_ESCAPE:
                    break;
                case VK_W:
                    pEvent->SetEvent(EEvents::eMOVE_FORWARD);
                    break;
                case VK_S:
                    pEvent->SetEvent(EEvents::eMOVE_BACKWARD);
                    break;
                case VK_A:
                    pEvent->SetEvent(EEvents::eMOVE_LEFT);
                    break;
                case VK_D:
                    pEvent->SetEvent(EEvents::eMOVE_RIGHT);
                    break;
                case VK_SPACE:
                    pEvent->SetEvent(EEvents::eJUMP);
                    break;
                case VK_I:
                    pEvent->SetEvent(EEvents::eINVENTORY);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_KEYUP:
            switch (_pWParam) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_W:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_W);
                    break;
                case VK_S:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_S);
                    break;
                case VK_A:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_A);
                    break;
                case VK_D:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_D);
                    break;
                case VK_SPACE:
                    pEvent->SetEvent(EEvents::eKEYRELEASE_JUMP);
                    break;
                case VK_I:
                    pEvent->SetEvent(EEvents::eINVENTORY_RELEASE);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            pEvent->SetEvent(EEvents::eGAME_LOOP_KILL);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        // Process other messages.
        default:
            return DefWindowProc(_pHwnd, _pMsg, _pWParam, _pLParam);
    }
    return 0;
}
} // namespace glvm::core
