#pragma once

#if RETRO_REV02
#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyLeaderboards.hpp"
#endif

struct CTRLeaderboards : DummyLeaderboards {};
#endif
