#include "CTRPresence.hpp"

#if RETRO_REV02

CTRRichPresence::~CTRRichPresence()
{
    // Reset description when the engine quits.
    if (lastLength) {
        FriendGameModeDescription description = {};
        FRD_UpdateGameModeDescription(&description);
    }
}

void CTRRichPresence::SetPresence(int32 id, String *message)
{
    curID = id;

    // Sanitize the SKU string: drop the trailing '\r' and clamp to the friends module's 127-character limit.
    // The message is already UTF-16 so no conversion needed.
    uint16 *chars = (message && message->chars) ? message->chars : NULL;
    int32 len     = (chars && message->length > 0) ? message->length : 0;
    if (len > 0 && chars[len - 1] == '\r') {
        --len;
    }
    if (len > FRIEND_GAME_MODE_DESCRIPTION_LEN - 1) {
        len = FRIEND_GAME_MODE_DESCRIPTION_LEN - 1;
    }

    FriendGameModeDescription description = {}; // zeroed: NUL terminator included
    for (int32 i = 0; i < len; ++i) {
        description[i] = chars[i];
    }

    // If same text is already displayed, skip it.
    if (len == (int32)lastLength && !memcmp(description, lastDescription, (uint32)len * sizeof(uint16))) {
        return;
    }

    Result result = FRD_UpdateGameModeDescription(&description);
    if (R_FAILED(result)) {
        if (!sendFailedShown) {
            sendFailedShown = true;

            PrintLog(PRINT_NORMAL, "Friends status send failed: %08lx", (unsigned long)result);
        }
        return;
    }

    PrintLog(PRINT_NORMAL, "Friend status %d chars sent: %08lx", len, (unsigned long)result);

    // Successfully sent, update cache.
    memcpy(lastDescription, description, sizeof(description));
    lastLength = (uint16)len;
}

#endif
