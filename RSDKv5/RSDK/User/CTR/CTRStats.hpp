#pragma once

#if RETRO_REV02
#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyStats.hpp"
#endif

struct CTRStats : DummyStats {};
#endif
