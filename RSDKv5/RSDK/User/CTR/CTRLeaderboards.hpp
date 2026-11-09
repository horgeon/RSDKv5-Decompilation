#pragma once

#if RETRO_REV02
#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyLeaderboards.hpp"
#endif

// 3DS has no leader board support, use the default/Dummy implementation.
struct CTRLeaderboards : DummyLeaderboards {};
#endif
