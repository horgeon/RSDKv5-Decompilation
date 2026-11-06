#if RETRO_REV02
#include "CTRCore.hpp"

CTRCore *ctrCore = NULL;

CTRCore *InitCTRCore()
{
    // Initalize API subsystems
    CTRCore *core = new CTRCore;

    if (achievements) {
        delete achievements;
    }
    achievements = new CTRAchievements;

    if (leaderboards) {
        delete leaderboards;
    }
    leaderboards = new CTRLeaderboards;

    if (richPresence) {
        delete richPresence;
    }
    richPresence = new CTRRichPresence;

    if (stats) {
        delete stats;
    }
    stats = new CTRStats;

    if (userStorage) {
        delete userStorage;
    }
    userStorage = new CTRStorage;

    return core;
}

#endif
