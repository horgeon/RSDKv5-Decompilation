#include "CTRStorage.hpp"

#if RETRO_REV02
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
