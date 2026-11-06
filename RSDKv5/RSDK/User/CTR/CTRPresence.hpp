#pragma once

#if RETRO_REV02
#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyPresence.hpp"
#endif

struct CTRRichPresence : DummyRichPresence {};
#endif
