#pragma once

#if RETRO_REV02

#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyStorage.hpp"
#endif

struct CTRStorage : DummyUserStorage {
    bool32 GetUsername(String *name) override;
};

#endif