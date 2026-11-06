#pragma once

#if RETRO_REV02

#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyCore.hpp"
#endif

struct CTRCore : DummyCore {};

extern CTRCore *ctrCore;

CTRCore *InitCTRCore();
#endif
