#include <3ds.h>
#include <string.h>
#include <stdio.h>

/*
 * Display an error box with a specific message.
 * @param u8Message The message to display (UTF-8 encoded).
 */
void error_box(bool wordWrap, bool fatal, const char *u8Message);
/*
 * Display an error box with a formatted message (like printf).
 * @param format The format string (printf-style).
 * @param ... The arguments for the format string.
 */
void error_box_format(bool wordWrap, bool fatal, const char *format, ...);

/*
 * Launch the 3DS Internet Browser with a specific URL.
 *
 * The browser is launched as a "Library Applet" via APT services.
 * The parameter buffer contains the URL to open.
 *
 * NOTE: The exact parameter format is based on reverse engineering
 * and may need adjustment depending on firmware version.
 * @param url The URL to open in the browser.
 * @return Result code from the APT service call.
 */
Result launch_browser(const char* url);
