#include "CTRStorage.hpp"

#if RETRO_REV02
// Device name the save archive is mounted under in the C stdio device tree.
static const char saveDeviceName[] = "save";

struct CFileHandle {
    FileIO *handle = NULL;

    CFileHandle(const char *path, const char *mode) { handle = fOpen(path, mode); }

    ~CFileHandle() { Close(); }

    void Close()
    {
        if (handle) {
            fClose(handle);
            handle = NULL;
        }
    }
};

CTRSaveArchive::CTRSaveArchive() {}

CTRSaveArchive::~CTRSaveArchive()
{
    if (archiveOpen) {
        archiveUnmount(saveDeviceName); // also closes the archive
    }
}

bool32 CTRSaveArchive::Open()
{
    if (archiveOpen) {
        return true;
    }

    // On first use, the save archive has to be formatted.
    Result result = archiveMount(ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""), saveDeviceName);
    if (R_FAILED(result)) {
        PrintLog(PRINT_NORMAL, "3DS save data not available (%08lx), formatting save data...", (unsigned long)result);

        // "blocks" argument is a block size on real firmware (0x200 on official titles). Size is specified in TMD.
        result = FSUSER_FormatSaveData(ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""), 0x200, 8, 32, 8, 32, false);
        if (R_SUCCEEDED(result)) {
            result = archiveMount(ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""), saveDeviceName);
        }
    }

    if (R_FAILED(result)) {
        PrintLog(PRINT_FATAL, "Unable to open 3DS title save archive: %08lx", (unsigned long)result);
        return false;
    }

    archiveOpen = true;
    return true;
}

bool32 CTRSaveArchive::Commit()
{
    // Save data writes only reach NAND on commit.
    return archiveOpen && R_SUCCEEDED(archiveCommitSaveData(saveDeviceName));
}

int32 CTRStorage::TryAuth()
{
    authStatus = STATUS_OK;
    return authStatus;
}

int32 CTRStorage::TryInitStorage()
{
    storageStatus = saveArchive.Open() ? STATUS_OK : STATUS_ERROR;
    return storageStatus;
}

bool32 CTRStorage::LoadUserFile(const char *filename, void *buffer, uint32 size)
{
    // Read from save archive.

    if (!saveArchive.IsOpen()) {
        return false;
    }

    char path[0x40];
    snprintf(path, sizeof(path), "%s:/%s", saveDeviceName, filename);

    CFileHandle file(path, "rb");
    if (!file.handle)
        return false;

    fSeek(file.handle, 0, SEEK_END);
    uint32 fileSize = (uint32)fTell(file.handle);
    fSeek(file.handle, 0, SEEK_SET);
    fRead(buffer, 1, fileSize < size ? fileSize : size, file.handle);

    return true;
}

bool32 CTRStorage::SaveUserFile(const char *filename, void *buffer, uint32 size)
{
    // Write from save archive.

    if (!saveArchive.IsOpen()) {
        return false;
    }

    char path[0x40];
    snprintf(path, sizeof(path), "%s:/%s", saveDeviceName, filename);

    CFileHandle file(path, "wb");
    if (!file.handle) {
        return false;
    }

    uint32 written = (uint32)fWrite(buffer, 1, size, file.handle);
    file.Close(); // flush stdio buffers into the archive before the commit below
    if (written != size) {
        return false;
    }

    return saveArchive.Commit();
}

bool32 CTRStorage::DeleteUserFile(const char *filename)
{
    // Delete from save archive.

    if (!saveArchive.IsOpen()) {
        return false;
    }

    char path[0x40];
    snprintf(path, sizeof(path), "%s:/%s", saveDeviceName, filename);
    if (remove(path) != 0) {
        return false;
    }

    return saveArchive.Commit();
}

bool32 CTRLoadUserFile(const char *filename, void *buffer, uint32 size)
{
    return userStorage && static_cast<CTRStorage *>(userStorage)->LoadUserFile(filename, buffer, size);
}

bool32 CTRSaveUserFile(const char *filename, void *buffer, uint32 size)
{
    return userStorage && static_cast<CTRStorage *>(userStorage)->SaveUserFile(filename, buffer, size);
}

bool32 CTRDeleteUserFile(const char *filename)
{
    return userStorage && static_cast<CTRStorage *>(userStorage)->DeleteUserFile(filename);
}

bool32 CTRStorage::GetUsername(String *name)
{
    MiiScreenName screenName{};
    Result out = FRD_GetMyScreenName(&screenName);
    if (R_FAILED(out)) {
        std::string str = __FILE__;
        str += ": GetUsername() # FRD_GetMyScreenName() failed with error code: ";
        str += std::to_string(out);
        str += "\r\n";
        PrintLog(PRINT_ERROR, str.c_str());
        return false;
    }
    // Convert from UTF-16 to UTF-8
    // MiiScreenName is 11 uint16_t long
    uint8_t u8ScreenName[32] = {};
    ssize_t u8ScreenNameLen = utf16_to_utf8(u8ScreenName, reinterpret_cast<const uint16_t *>(&screenName), 31);
    InitString(name, reinterpret_cast<const char *>(u8ScreenName), u8ScreenNameLen);

    return true;
}
#endif
