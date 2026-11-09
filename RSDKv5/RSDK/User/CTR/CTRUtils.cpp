#include "CTRUtils.hpp"

bool title_installed_on(FS_MediaType media, u64 tid)
{
    u32 count = 0;
    if (R_FAILED(AM_GetTitleCount(media, &count)) || count == 0) {
        return false;
    }

    u64 *ids = (u64 *)malloc(count * sizeof(u64));
    if (!ids) {
        return false;
    }

    u32 read   = 0;
    bool found = false;

    if (R_SUCCEEDED(AM_GetTitleList(&read, media, count, ids))) {
        for (u32 i = 0; i < read; i++) {
            if (ids[i] == tid) {
                found = true;
                break;
            }
        }
    }

    free(ids);
    return found;
}

u64 dlc_title_id(u32 uniqueId)
{
    // Retail DLC title IDs use the category bitmask 0004008C + the application's UniqueId:
    // * AddOnContents (0x4)
    // * CannotExecution (0x8)
    // * NotRequireRightForMount (0x80)
    const u64 dlcTitleType = 0x0004008CULL;
    return (dlcTitleType << 32) | ((u64)(uniqueId & 0xFFFFF) << 8);
}

void error_box(bool wordWrap, bool fatal, const char *u8Message)
{
    errorConf err;
    memset(&err, 0, sizeof(err)); // errorInit doesn't zero the rest
    // We do not handle language.
    if (wordWrap) {
        errorInit(&err, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_DEFAULT);
    }
    else {
        errorInit(&err, ERROR_TEXT, CFG_LANGUAGE_DEFAULT);
    }

    if (fatal) {
        err.softwareReset = true;
    }
    errorText(&err, u8Message);

    errorDisp(&err);

    if (R_FAILED(err.returnCode)) {
        // applet couldn't launch, fall back to popup
        PrintLog(PRINT_POPUP, "%s", u8Message);
    }
}

void error_box_format(bool wordWrap, bool fatal, const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    error_box(wordWrap, fatal, (const char *)buffer);
}

Result launch_browser(const char *url)
{
    PrintLog(PRINT_NORMAL, "Launch browser at URL: %s", url);

    if (!url) {
        url = "";
    }

    // The Internet Browser's applet parameter is simply the NUL-terminated URL
    // string at offset 0 (see 3dbrew).
    static u8 param[0x400] __attribute__((aligned(4)));
    u32 len = strnlen(url, sizeof(param) - 1) + 1; // NUL terminator included
    memcpy(param, url, len);

    // The Internet Browser is a system applet (APPID_WEB = 0x114), use
    // aptLaunchSystemApplet().
    aptLaunchSystemApplet(APPID_WEB, param, len, 0);

    return 0;
}
