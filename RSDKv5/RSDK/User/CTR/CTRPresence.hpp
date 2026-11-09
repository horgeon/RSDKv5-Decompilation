#pragma once

#if RETRO_REV02

// 3DS do have a "rich presence" system: the game mode description, using frd:u command UpdateGameModeDescription.
struct CTRRichPresence : UserRichPresence {
public:
    ~CTRRichPresence();

    void SetPresence(int32 id, String *message);

private:
    // Identical sends are skipped, to save an IPC call.
    FriendGameModeDescription lastDescription = {};
    uint16 lastLength                         = 0;

    // The friends module failing a runtime update is reported once per launch.
    bool32 sendFailedShown = false;
};
#endif
