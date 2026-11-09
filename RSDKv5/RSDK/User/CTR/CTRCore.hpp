#pragma once

#if RETRO_REV02

#include <3ds.h>

struct CTRCore : UserCore {
    CTRCore();
    ~CTRCore() override;

    void StageLoad();
    bool32 CheckFocusLost();
    int32 GetUserLanguage();
    int32 GetUserRegion();
    int32 GetUserPlatform();
    bool32 GetConfirmButtonFlip();
    void LaunchManual();
    bool32 ShowExtensionOverlay(int32 overlay);
    void ExitGame();
    int32 GetDefaultGamepadType();
    bool32 IsOverlayEnabled(uint32 overlay) {
        return false;
    }

private:
    Result amStatus;
    Result cfguStatus;
    Result frdStatus;
    Result fsStatus;

    bool32 CheckDLCCIAInstalled();
};

extern CTRCore *ctrCore;

CTRCore *InitCTRCore();

#endif
