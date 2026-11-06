#pragma once

#if RETRO_REV02

#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyAchievements.hpp"
#endif

struct CTRAchievements : DummyAchievements {};
#endif
