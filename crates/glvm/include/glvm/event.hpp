#pragma once

namespace glvm::core {

class CStack;

enum EEvents {
    eDEFAULT,
    eKEYRELEASE_A,
    eKEYRELEASE_D,
    eKEYRELEASE_S,
    eKEYRELEASE_W,
    eKEYRELEASE_JUMP,
    eGRAVITY_COLLISION_FLAG,
    eRENDER,
    eATACK,
    eSPAWN,
    eJUMP,
    eINVENTORY,
    eINVENTORY_RELEASE,
    eMOVE_FORWARD,
    eMOVE_BACKWARD,
    eMOVE_LEFT,
    eMOVE_RIGHT,
    eMOVE_DIAGONAL_FB,
    eMOVE_DIAGONAL_FL,
    eMOVE_DIAGONAL_LB,
    eMOVE_DIAGONAL_BR,
    eMOUSE_POINTER_POSITION,
    eMOUSE_LEFT_BUTTON_RELEASE,
    eMOUSE_LEFT_BUTTON,
    eMOUSE_RIGHT_BUTTON_RELEASE,
    eMOUSE_RIGHT_BUTTON,
    eCURSOR_RELEASED,
    eGAME_LOOP_KILL,
    eEmpty,
};

struct SMousePointerPosition {
    int position_X;
    int position_Y;
    int offset_X = 0;
    int offset_Y = 0;
    float pitch;
    float yaw;
};

class CEvent {
    EEvents eEvent_;
    EEvents nextEvent;

public:
    SMousePointerPosition mousePointerPosition;
    bool nextEventFlag = false;

    CEvent();
    EEvents& GetEvent();
    void SetEvent(EEvents _eEvent);
    void SetNextEvent(EEvents _eEvent);
    EEvents GetNextEvent();
    void SetLastEvent(CStack _Stack);

    bool isLeftMouseButtonReleased = true;
};

} // namespace glvm::core
