#pragma once

#if RETRO_REV02

#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyAchievements.hpp"
#endif

// 3DS has no achievements support, use the default/Dummy implementation.
struct CTRAchievements : DummyAchievements {};
#endif
