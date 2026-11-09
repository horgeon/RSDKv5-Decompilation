#if RETRO_REV02
#include "CTRCore.hpp"
#include "CTRUtils.hpp"
// CMake does not acutally include platform abstractions, so we have to do it ourself.
#include "CTRUtils.cpp"

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

CTRCore::CTRCore()
    : cfguStatus(cfguInit()), amStatus(amInit()), frdStatus(frdInit(true)) // forceUser: open frd:u, not the admin-only frd:a
{
    if (R_FAILED(cfguStatus)) {
        PrintLog(PRINT_FATAL, "Unable to initialize 3DS CFGU service: %08lx", (unsigned long)cfguStatus);
        return;
    }

    if (R_FAILED(amStatus)) {
        PrintLog(PRINT_FATAL, "Unable to initialize 3DS AM service: %08lx", (unsigned long)amStatus);
        return;
    }

    if (R_FAILED(frdStatus)) {
        PrintLog(PRINT_FATAL, "Unable to initialize 3DS FRD service: %08lx", (unsigned long)frdStatus);
        return;
    }

    // Are Sonic Mania Plus features enabled? Check DLC once, as CheckDLC runs in a hot loop. We check for a DLC title
    // to enable it or not.
    values[0] = false;
    if (CheckDLCCIAInstalled()) {
        values[0] = true;
    }
}

CTRCore::~CTRCore()
{
    frdExit();
    cfguExit();
    amExit();
}

void CTRCore::StageLoad()
{
    UserCore::StageLoad();

    // Are Sonic Mania Plus features enabled?
    AddViewableVariable("Ext <PLUS>", &values[0], VIEWVAR_BOOL, false, true);
}

bool32 CTRCore::CheckFocusLost() {
    return focusState != 0;
}

int32 CTRCore::GetUserLanguage()
{
    u8 language;

    Result result = CFGU_GetSystemLanguage(&language);
    if (R_FAILED(result)) {
        return LANGUAGE_EN;
    }

    switch (language) {
        case CFG_LANGUAGE_JP: return LANGUAGE_JP;
        case CFG_LANGUAGE_EN: return LANGUAGE_EN;
        case CFG_LANGUAGE_FR: return LANGUAGE_FR;
        case CFG_LANGUAGE_DE: return LANGUAGE_GE;
        case CFG_LANGUAGE_IT: return LANGUAGE_IT;
        case CFG_LANGUAGE_ES: return LANGUAGE_SP;
        case CFG_LANGUAGE_ZH: return LANGUAGE_SC;
        case CFG_LANGUAGE_KO: return LANGUAGE_KO;
        case CFG_LANGUAGE_TW: return LANGUAGE_TC;
        default: PrintLog(PRINT_POPUP, "Unsupported system language: %u", language); break;
    }

    return LANGUAGE_EN;
}

int32 CTRCore::GetUserRegion()
{
    u8 region = CFG_REGION_USA;

    Result result = CFGU_SecureInfoGetRegion(&region);
    if (R_FAILED(result)) {
        return REGION_US;
    }

    switch (region) {
        case CFG_REGION_JPN: return REGION_JP;
        case CFG_REGION_USA: return REGION_US;
        case CFG_REGION_EUR: return REGION_EU;
        case CFG_REGION_AUS: return REGION_EU;
        case CFG_REGION_CHN: return REGION_JP;
        case CFG_REGION_KOR: return REGION_JP;
        case CFG_REGION_TWN: return REGION_JP;
    }

    return REGION_US;
}

int32 CTRCore::GetUserPlatform() {
    return PLATFORM_CTR;
}

bool32 CTRCore::GetConfirmButtonFlip() {
    return engine.confirmFlip;
}

void CTRCore::LaunchManual()
{
    // LaunchManual() just opens the Mania manual URL, that's it.
    // URLs copied straight from Switch implementation.
    const char *manualURL = "";
    switch (curSKU.language) {
        default:
        case LANGUAGE_EN: manualURL = "https://www.sonicthehedgehog.com/mania/manual/en/"; break;
        case LANGUAGE_FR: manualURL = "https://www.sonicthehedgehog.com/mania/manual/fr/"; break;
        case LANGUAGE_IT: manualURL = "https://www.sonicthehedgehog.com/mania/manual/it/"; break;
        case LANGUAGE_GE: manualURL = "https://www.sonicthehedgehog.com/mania/manual/de/"; break;
        case LANGUAGE_SP: manualURL = "https://www.sonicthehedgehog.com/mania/manual/es/"; break;
        case LANGUAGE_JP: manualURL = "https://manual.sega.jp/sonicmania/"; break;
        case LANGUAGE_KO: manualURL = "https://manual.sega.jp/sonicmania/kr/"; break;
        case LANGUAGE_SC: manualURL = "https://manual.sega.jp/sonicmania/ct/"; break;
        case LANGUAGE_TC: manualURL = "https://www.sonicthehedgehog.com/mania/manual/en/"; break;
    }

    launch_browser(manualURL);
}

bool32 CTRCore::CheckDLCCIAInstalled()
{
    u64 tid          = dlc_title_id(0x3082);
    bool isInstalled = title_installed_on(MEDIATYPE_NAND, tid) || title_installed_on(MEDIATYPE_SD, tid);

    PrintLog(PRINT_NORMAL, "DLC check for title ID %016llX: %s", tid, isInstalled ? "installed" : "not installed");

    return isInstalled;
}

bool32 CTRCore::ShowExtensionOverlay(int32 overlay)
{
    if(overlay != 0) {
        error_box_format(false, false, "No 3DS extension overlay is available: %d", overlay);
    }

    const char *message = "";
    switch (curSKU.language) {
        default:
        case LANGUAGE_EN: message = "To enjoy Sonic Mania Plus, install the DLC package. More information is available on the download page."; break;
        case LANGUAGE_JP: message = "Sonic Mania Plus をお楽しみになるには、DLC パッケージをインストールしてください。詳しくはダウンロードページをご覧ください。"; break;
        case LANGUAGE_FR: message = "Pour profiter de Sonic Mania Plus, installez le paquet DLC. Plus d'informations sur la page de téléchargement."; break;
        case LANGUAGE_IT: message = "Per usufruire di Sonic Mania Plus, installa il pacchetto DLC. Ulteriori informazioni sono disponibili nella pagina di download."; break;
        case LANGUAGE_GE: message = "Um Sonic Mania Plus genießen zu können, installiere das DLC-Paket. Weitere Informationen findest du auf der Downloadseite."; break;
        case LANGUAGE_SP: message = "Para disfrutar de Sonic Mania Plus, instala el paquete DLC. Más información en la página de descarga."; break;
        case LANGUAGE_KO: message = "소닉 마니아 플러스의 기능을 이용하려면 DLC 패키지를 설치해야 합니다. 자세한 내용은 다운로드 페이지를 확인해 주세요."; break;
        case LANGUAGE_SC: message = "要享受 Sonic Mania Plus，请安装 DLC 包。更多信息请见下载页面。"; break;
        case LANGUAGE_TC: message = "要享受 Sonic Mania Plus，請安裝 DLC 套件。更多資訊請見下載頁面。"; break;
    }

    error_box(true, false, message);
    return false;
}

void CTRCore::ExitGame() { RenderDevice::isRunning = false; }

int32 CTRCore::GetDefaultGamepadType() { return (DEVICE_API_NONE << 16) | (DEVICE_TYPE_CONTROLLER << 8) | (0 << 0); }
#endif
