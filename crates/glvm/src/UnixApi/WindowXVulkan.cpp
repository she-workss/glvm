// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/UnixApi/WindowXVulkan.hpp"

#include <X11/Xlib.h>
#include <bits/types/time_t.h>
#include <bits/types/wint_t.h>
#include <iostream>

namespace GLVM::core {
WindowXVulkan::WindowXVulkan() {
    // const int aAttrib[] =
    // {
    //     GLX_RENDER_TYPE, GLX_RGBA_BIT,
    //     GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    //     GLX_DOUBLEBUFFER, true,
    //     GLX_RED_SIZE, 1,
    //     GLX_GREEN_SIZE, 1,
    //     GLX_BLUE_SIZE, 1,
    //     None
    // };

    pDisp_ = XOpenDisplay(NULL);
    Root_Window_ = DefaultRootWindow(pDisp_);
    Set_Window_Attributes_.event_mask = KeyPressMask | KeyReleaseMask
        | PointerMotionMask | StructureNotifyMask | ButtonPressMask
        | ButtonReleaseMask | FocusChangeMask;

    const int screenNumber = XDefaultScreen(pDisp_);
    width = DisplayWidth(pDisp_, screenNumber);
    height = DisplayHeight(pDisp_, screenNumber);
    Win_ = XCreateWindow(
        pDisp_,
        Root_Window_,
        0,
        0,
        width,
        height,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask,
        &Set_Window_Attributes_
    );
    ///< Show_the_window

    XMapWindow(pDisp_, Win_);

    XWarpPointer(pDisp_, None, Win_, 0, 0, 0, 0, 0, 0);

    Cursor invisibleCursor;
    Pixmap bitmapNoData;
    XColor black;
    static char noData[] = {0, 0, 0, 0, 0, 0, 0, 0};
    black.red = black.green = black.blue = 0;

    bitmapNoData = XCreateBitmapFromData(pDisp_, Win_, noData, 8, 8);
    invisibleCursor = XCreatePixmapCursor(
        pDisp_,
        bitmapNoData,
        bitmapNoData,
        &black,
        &black,
        0,
        0
    );
    XDefineCursor(pDisp_, Win_, invisibleCursor);

    XFreeCursor(pDisp_, invisibleCursor);
    XFreePixmap(pDisp_, bitmapNoData);

    XGetWindowAttributes(pDisp_, Win_, &GWindow_Attributes_);
    //		const int kInterval = 1;
}

WindowXVulkan::~WindowXVulkan() = default;

Window WindowXVulkan::GetWindow() {
    return Win_;
}

Display* WindowXVulkan::GetDisplay() {
    return pDisp_;
}

void WindowXVulkan::CursorLock(
    int _x_position,
    int _y_position,
    int* _x_offset,
    int* _y_offset
) {
    int iOffset_X = 0, iOffset_Y = 0;
    iOffset_X = _x_position - (int)(width / 2);
    iOffset_Y = _y_position - (int)(height / 2);

    *_x_offset += iOffset_X;
    *_y_offset -= iOffset_Y;

    ///< Pitch is limited by angle in Engine::SetViewMatrix(), so this offset
    ///< may accumulate freely; no pixel clamp here (resolution-independent).

    XWarpPointer(pDisp_, None, Win_, 0, 0, 0, 0, (int)(width / 2), (int)(height / 2));
    XFlush(pDisp_);
}

void WindowXVulkan::SwapBuffers() {}

void WindowXVulkan::ClearDisplay() {}

bool WindowXVulkan::HandleEvent(CEvent& _Event) {
    XEvent uXEvent;

    while (XPending(pDisp_)) {
        XNextEvent(pDisp_, &uXEvent);
        KeySym ulKey;
        unsigned int uiMouse_Button;
        XMotionEvent motion;

        switch (uXEvent.type) {
            case MotionNotify:
                motion = uXEvent.xmotion;

                _Event.SetEvent(EEvents::eMOUSE_POINTER_POSITION);
                _Event.mousePointerPosition.position_X = motion.x;
                _Event.mousePointerPosition.position_Y = motion.y;

                ///< Search MapNotify events depend on XMapWindow(pDisp_, Win_)
                ///< function.
                //				break;
            case MapNotify:
                ///< Link mouse cursor to specified window.
                XGrabPointer(
                    pDisp_,
                    Win_,
                    True,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    Win_,
                    None,
                    CurrentTime
                );
                break;
            case FocusIn:
                isFocused = true;
                XGrabPointer(
                    pDisp_,
                    Win_,
                    True,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    Win_,
                    None,
                    CurrentTime
                );
                break;
            case FocusOut:
                isFocused = false;
                XUngrabPointer(pDisp_, CurrentTime);
                break;
            case ButtonPress:
                uiMouse_Button = uXEvent.xbutton.button;
                switch (uiMouse_Button) {
                    case 1:
                        _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
                        break;
                }
                break;

            case ButtonRelease:
                uiMouse_Button = uXEvent.xbutton.button;
                switch (uiMouse_Button) {
                    case 1:
                        _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
                        _Event.isLeftMouseButtonReleased = true;
                        break;
                }
                break;

            case KeyPress:
                ulKey = XLookupKeysym(&uXEvent.xkey, 0);
                switch (ulKey) {
                    case XKEY_I:
                        _Event.SetEvent(EEvents::eINVENTORY);
                        break;
                    case XKEY_ESCAPE:
                        _Event.SetEvent(EEvents::eGAME_LOOP_KILL);
                        break;
                    case XKEY_A:
                        _Event.SetEvent(EEvents::eMOVE_LEFT);
                        break;
                    case XKEY_D:
                        _Event.SetEvent(EEvents::eMOVE_RIGHT);
                        break;
                    case XKEY_S:
                        _Event.SetEvent(EEvents::eMOVE_BACKWARD);
                        break;
                    case XKEY_W:
                        _Event.SetEvent(EEvents::eMOVE_FORWARD);
                        break;
                    case XKEY_SPACE:
                        _Event.SetEvent(EEvents::eJUMP);
                        break;
                }
                break;

            case KeyRelease:
                if (XEventsQueued(pDisp_, QueuedAfterReading)) {
                    XEvent uXNext_Event;
                    XPeekEvent(pDisp_, &uXNext_Event);

                    if (uXNext_Event.type == KeyPress
                        && uXNext_Event.xkey.time == uXEvent.xkey.time
                        && uXNext_Event.xkey.keycode == uXEvent.xkey.keycode) {
                        ///< Key wasn’t actually released
                        XNextEvent(pDisp_, &uXNext_Event);
                        continue;
                    }
                }
                ulKey = XLookupKeysym(&uXEvent.xkey, 0);
                switch (ulKey) {
                    case XKEY_I:
                        _Event.SetEvent(EEvents::eINVENTORY_RELEASE);
                        break;
                    case XKEY_A:
                        _Event.SetEvent(GLVM::core::eKEYRELEASE_A);
                        break;
                    case XKEY_D:
                        _Event.SetEvent(GLVM::core::eKEYRELEASE_D);
                        break;
                    case XKEY_S:
                        _Event.SetEvent(GLVM::core::eKEYRELEASE_S);
                        break;
                    case XKEY_W:
                        _Event.SetEvent(GLVM::core::eKEYRELEASE_W);
                        break;
                    case XKEY_SPACE:
                        _Event.SetEvent(GLVM::core::eKEYRELEASE_JUMP);
                        break;
                }
                break;
        }

        Input_Stack_.ControlInput(_Event);
    }
    return false;
}

void WindowXVulkan::Close() {
    XDestroyWindow(pDisp_, Win_);
    //        XFreeColormap(pDisp_, Color_Map_);
    //        XFree(pVisual_);
    //        XFree(pFbc_);
    XCloseDisplay(pDisp_);
}
} // namespace GLVM::core
