#pragma once

#if RETRO_REV02
#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyStats.hpp"
#endif

// 3DS has no custom per-game stat support, use the default/Dummy implementation.
struct CTRStats : DummyStats {};
#endif
