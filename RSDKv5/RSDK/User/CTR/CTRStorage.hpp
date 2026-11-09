#pragma once

#if RETRO_REV02

#if !RETRO_USERCORE_DUMMY
#include "RSDK/User/Dummy/DummyStorage.hpp"
#endif

struct CTRSaveArchive {
    CTRSaveArchive();
    ~CTRSaveArchive();
    bool32 Open();
    bool IsOpen() const { return archiveOpen; }
    bool32 Commit();

private:
    bool archiveOpen = false;
};

// We still inherit from default/dummy implementation, as it has loads of code to deal with files, so we don't have to
// do it.
struct CTRStorage : DummyUserStorage {
    CTRStorage() {}
    int32 TryAuth();
    int32 TryInitStorage();
    bool32 GetUsername(String *name) override;
    bool32 LoadUserFile(const char *filename, void *buffer, uint32 size);
    bool32 SaveUserFile(const char *filename, void *buffer, uint32 size);
    bool32 DeleteUserFile(const char *filename);

private:
    CTRSaveArchive saveArchive;
};

bool32 CTRLoadUserFile(const char *filename, void *buffer, uint32 size);
bool32 CTRSaveUserFile(const char *filename, void *buffer, uint32 size);
bool32 CTRDeleteUserFile(const char *filename);

#endif
