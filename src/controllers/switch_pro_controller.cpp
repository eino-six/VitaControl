#include <cstdlib>
#include <cstring>
#include <psp2kern/ctrl.h>
#include <psp2kern/kernel/iofilemgr.h>

#include "switch_pro_controller.h"

namespace
{
struct ButtonName
{
    const char *name;
    uint32_t mask;
};

const ButtonName buttonNames[] =
{
    { "CROSS",    SCE_CTRL_CROSS },
    { "CIRCLE",   SCE_CTRL_CIRCLE },
    { "SQUARE",   SCE_CTRL_SQUARE },
    { "TRIANGLE", SCE_CTRL_TRIANGLE },
    { "L1",       SCE_CTRL_L1 },
    { "R1",       SCE_CTRL_R1 },
    { "L2",       SCE_CTRL_LTRIGGER },
    { "R2",       SCE_CTRL_RTRIGGER },
    { "L3",       SCE_CTRL_L3 },
    { "R3",       SCE_CTRL_R3 },
    { "SELECT",   SCE_CTRL_SELECT },
    { "START",    SCE_CTRL_START },
    { "PS",       SCE_CTRL_PSBUTTON },
    { "UP",       SCE_CTRL_UP },
    { "DOWN",     SCE_CTRL_DOWN },
    { "LEFT",     SCE_CTRL_LEFT },
    { "RIGHT",    SCE_CTRL_RIGHT },
    { "EXT1",     SCE_CTRL_EXT1 },
};

bool tokenEquals(const char *token, size_t length, const char *name)
{
    size_t i = 0;
    for (; i < length && name[i]; i++)
    {
        char a = token[i];
        char b = name[i];
        if (a >= 'a' && a <= 'z') a -= 'a' - 'A';
        if (b >= 'a' && b <= 'z') b -= 'a' - 'A';
        if (a != b)
            return false;
    }

    return i == length && name[i] == '\0';
}

uint32_t buttonMaskFromName(const char *token, size_t length)
{
    for (unsigned int i = 0; i < sizeof(buttonNames) / sizeof(buttonNames[0]); i++)
    {
        if (tokenEquals(token, length, buttonNames[i].name))
            return buttonNames[i].mask;
    }

    return 0;
}
}

SwitchProController::SwitchProController(uint32_t mac0, uint32_t mac1, int port, bool switchMode)
    : Controller(mac0, mac1, port), switchMode(switchMode)
{
}

void SwitchProController::start(uint8_t *buffer, size_t length)
{
    // Optional per-controller button remapping. The file is loaded when the
    // controller connects, so reconnect it after editing the config.
    loadRemapConfig();

    if (switchMode)
    {
        // Prepare a write request to switch to standard mode
        static uint8_t report[12] = {};
        report[0]  = 0x01;
        report[1]  = 0x01;
        report[10] = 0x03;
        report[11] = 0x30;

        // Send the write request
        requestReport(HID_REQUEST_WRITE, report, sizeof(report));
    }
    else
    {
        // 8BitDo DirectInput mode (including the Ultimate 2.4G over Bluetooth)
        // streams input reports without an init command. Start the read chain.
        requestReport(HID_REQUEST_READ, buffer, length);
    }
}

void SwitchProController::processReport(uint8_t *buffer, size_t length)
{
    if (buffer[0] == 0x30 || buffer[0] == 0x21)
        processStandardReport(buffer, length);
    else if (buffer[0] == 0x3F)
        processSimpleReport(buffer, length);
    else if (!switchMode)
        processDirectInputReport(buffer, length);

    applyRemap();

    // The DirectInput parser logs its own reports. Also log Switch reports so
    // a "pairs but no input" case on the Switch path produces useful data.
    if (buffer[0] == 0x30 || buffer[0] == 0x21 || buffer[0] == 0x3F)
        logReport(buffer);
    else if (switchMode)
        logReport(buffer);
}

void SwitchProController::processStandardReport(uint8_t *buffer, size_t length)
{
    // Interpret the data as a standard input report
    SwitchProReport0x30 *report = (SwitchProReport0x30*)buffer;

    // Clear the old control data
    controlData.buttons = 0;

    // Map the face buttons
    if (report->b) controlData.buttons |= SCE_CTRL_CROSS;
    if (report->a) controlData.buttons |= SCE_CTRL_CIRCLE;
    if (report->x) controlData.buttons |= SCE_CTRL_TRIANGLE;
    if (report->y) controlData.buttons |= SCE_CTRL_SQUARE;

    // Map the D-pad
    if (report->up)    controlData.buttons |= SCE_CTRL_UP;
    if (report->right) controlData.buttons |= SCE_CTRL_RIGHT;
    if (report->down)  controlData.buttons |= SCE_CTRL_DOWN;
    if (report->left)  controlData.buttons |= SCE_CTRL_LEFT;

    // Map the triggers
    if (report->l)      controlData.buttons |= SCE_CTRL_L1;
    if (report->r)      controlData.buttons |= SCE_CTRL_R1;
    if (report->zl)     controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (report->zr)     controlData.buttons |= SCE_CTRL_RTRIGGER;
    if (report->stickL) controlData.buttons |= SCE_CTRL_L3;
    if (report->stickR) controlData.buttons |= SCE_CTRL_R3;

    // Map the menu buttons
    if (report->plus)  controlData.buttons |= SCE_CTRL_START;
    if (report->minus) controlData.buttons |= SCE_CTRL_SELECT;
    if (report->home)  controlData.buttons |= SCE_CTRL_PSBUTTON;

    // Map the extra buttons
    if (report->capture) controlData.buttons |= SCE_CTRL_EXT1;

    // Map the sticks
    controlData.leftX  = report->leftX  >> 4;
    controlData.leftY  = report->leftY  >> 4;
    controlData.rightX = report->rightX >> 4;
    controlData.rightY = report->rightY >> 4;

    // Reverse up and down
    controlData.leftY  = 255 - controlData.leftY;
    controlData.rightY = 255 - controlData.rightY;

    // Map the motion controls
    motionState.accelerX  = report->accelerX;
    motionState.accelerY  = report->accelerY;
    motionState.accelerZ  = report->accelerZ;
    motionState.velocityX = report->velocityX;
    motionState.velocityY = report->velocityY;
    motionState.velocityZ = report->velocityZ;

    // TODO: implement battery level
}

void SwitchProController::processSimpleReport(uint8_t *buffer, size_t length)
{
    // Interpret the data as a simple input report (used by 8BitDo and other clones)
    SwitchProReport0x3F *report = (SwitchProReport0x3F*)buffer;

    // Clear the old control data
    controlData.buttons = 0;

    // Map the face buttons
    if (report->b) controlData.buttons |= SCE_CTRL_CROSS;
    if (report->a) controlData.buttons |= SCE_CTRL_CIRCLE;
    if (report->x) controlData.buttons |= SCE_CTRL_TRIANGLE;
    if (report->y) controlData.buttons |= SCE_CTRL_SQUARE;

    // Map the D-pad from hat switch value
    switch (report->dpad)
    {
        case 0: controlData.buttons |= SCE_CTRL_UP; break;
        case 1: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_RIGHT; break;
        case 2: controlData.buttons |= SCE_CTRL_RIGHT; break;
        case 3: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_RIGHT; break;
        case 4: controlData.buttons |= SCE_CTRL_DOWN; break;
        case 5: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_LEFT; break;
        case 6: controlData.buttons |= SCE_CTRL_LEFT; break;
        case 7: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_LEFT; break;
        default: break; // 8 = centered / released
    }

    // Map the triggers
    if (report->l)      controlData.buttons |= SCE_CTRL_L1;
    if (report->r)      controlData.buttons |= SCE_CTRL_R1;
    if (report->zl)     controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (report->zr)     controlData.buttons |= SCE_CTRL_RTRIGGER;
    if (report->stickL) controlData.buttons |= SCE_CTRL_L3;
    if (report->stickR) controlData.buttons |= SCE_CTRL_R3;

    // Map the menu buttons
    if (report->plus)  controlData.buttons |= SCE_CTRL_START;
    if (report->minus) controlData.buttons |= SCE_CTRL_SELECT;
    if (report->home)  controlData.buttons |= SCE_CTRL_PSBUTTON;

    // Map the extra buttons
    if (report->capture) controlData.buttons |= SCE_CTRL_EXT1;

    // Map the sticks (already 8-bit in simple mode)
    controlData.leftX  = report->leftX;
    controlData.leftY  = report->leftY;
    controlData.rightX = report->rightX;
    controlData.rightY = report->rightY;

    // No motion data available in simple mode
}

void SwitchProController::processDirectInputReport(uint8_t *buffer, size_t length)
{
    // 8BitDo DirectInput is used by the Ultimate 2.4G in Bluetooth mode.
    // Two layouts are common:
    //
    //   Enhanced Bluetooth report:
    //     [0x01, hat, LX, LY, RX, RY, RT, LT, buttons0, buttons1, ...]
    //
    //   Legacy DirectInput report (old firmware, 9 bytes, no report ID):
    //     [buttons0, buttons1, hat, LX, LY, RX, RY, ...]
    //
    // The report ID is the reliable discriminator. Enhanced reports start
    // with 0x01 over Bluetooth (0x03/0x04 over USB); the legacy report has no
    // report ID, so its first byte is the button byte. Lock the layout on the
    // first report instead of guessing from stick positions, which can move
    // far enough to look like the other layout.
    if (layout < 0)
    {
        if (buffer[0] == 0x01 || buffer[0] == 0x03 || buffer[0] == 0x04)
        {
            layout = 1;
        }
        else if ((buffer[0] <= 8 || buffer[0] == 0x0F) &&
                 buffer[1] >= 0x60 && buffer[1] <= 0xA0 &&
                 buffer[2] >= 0x60 && buffer[2] <= 0xA0 &&
                 buffer[3] >= 0x60 && buffer[3] <= 0xA0 &&
                 buffer[4] >= 0x60 && buffer[4] <= 0xA0)
        {
            // Enhanced layout with the report ID stripped by the host stack.
            layout = 2;
        }
        else
        {
            layout = 0;
        }

        const char *line = (layout == 1)
            ? "LAYOUT=enhanced (report ID 0x01/0x03/0x04)\n"
            : (layout == 2)
                ? "LAYOUT=enhanced (no report ID)\n"
                : "LAYOUT=legacy (no report ID)\n";
        SceUID fd = ksceIoOpen("ux0:data/vitacontrol-8bitdo.log",
            SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
        if (fd >= 0)
        {
            ksceIoWrite(fd, line, strlen(line));
            ksceIoClose(fd);
        }
    }

    bool enhanced = (layout == 1 || layout == 2);
    controlData.buttons = 0;

    uint8_t hat;
    uint8_t leftX, leftY, rightX, rightY;
    uint8_t leftTrigger = 0, rightTrigger = 0;

    if (enhanced)
    {
        int offset = (layout == 2) ? 0 : 1;
        hat = buffer[offset];
        leftX  = buffer[offset + 1];
        leftY  = buffer[offset + 2];
        rightX = buffer[offset + 3];
        rightY = buffer[offset + 4];
        rightTrigger = buffer[offset + 5];
        leftTrigger  = buffer[offset + 6];

        uint8_t b0 = buffer[offset + 7];
        uint8_t b1 = buffer[offset + 8];

        if (b0 & 0x01) controlData.buttons |= SCE_CTRL_CROSS;    // B
        if (b0 & 0x02) controlData.buttons |= SCE_CTRL_CIRCLE;   // A
        if (b0 & 0x08) controlData.buttons |= SCE_CTRL_SQUARE;   // Y
        if (b0 & 0x10) controlData.buttons |= SCE_CTRL_TRIANGLE; // X
        if (b0 & 0x40) controlData.buttons |= SCE_CTRL_L1;
        if (b0 & 0x80) controlData.buttons |= SCE_CTRL_R1;

        if (b1 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
        if (b1 & 0x08) controlData.buttons |= SCE_CTRL_START;
        if (b1 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
        if (b1 & 0x20) controlData.buttons |= SCE_CTRL_L3;
        if (b1 & 0x40) controlData.buttons |= SCE_CTRL_R3;
        if (b1 & 0x80) controlData.buttons |= SCE_CTRL_EXT1;

        if (leftTrigger  > 0x40) controlData.buttons |= SCE_CTRL_LTRIGGER;
        if (rightTrigger > 0x40) controlData.buttons |= SCE_CTRL_RTRIGGER;
    }
    else
    {
        uint8_t b0 = buffer[0];
        uint8_t b1 = buffer[1];
        hat = buffer[2];
        leftX  = buffer[3];
        leftY  = buffer[4];
        rightX = buffer[5];
        rightY = buffer[6];

        if (b0 & 0x01) controlData.buttons |= SCE_CTRL_CROSS;    // B
        if (b0 & 0x02) controlData.buttons |= SCE_CTRL_CIRCLE;   // A
        if (b0 & 0x08) controlData.buttons |= SCE_CTRL_SQUARE;   // Y
        if (b0 & 0x10) controlData.buttons |= SCE_CTRL_TRIANGLE; // X
        if (b0 & 0x40) controlData.buttons |= SCE_CTRL_L1;
        if (b0 & 0x80) controlData.buttons |= SCE_CTRL_R1;

        if (b1 & 0x01) controlData.buttons |= SCE_CTRL_LTRIGGER;
        if (b1 & 0x02) controlData.buttons |= SCE_CTRL_RTRIGGER;
        if (b1 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
        if (b1 & 0x08) controlData.buttons |= SCE_CTRL_START;
        if (b1 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
        if (b1 & 0x20) controlData.buttons |= SCE_CTRL_L3;
        if (b1 & 0x40) controlData.buttons |= SCE_CTRL_R3;
    }

    // Map the D-pad from the hat value (0-7 clockwise from up, 8 = centered)
    switch (hat)
    {
        case 0: controlData.buttons |= SCE_CTRL_UP; break;
        case 1: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_RIGHT; break;
        case 2: controlData.buttons |= SCE_CTRL_RIGHT; break;
        case 3: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_RIGHT; break;
        case 4: controlData.buttons |= SCE_CTRL_DOWN; break;
        case 5: controlData.buttons |= SCE_CTRL_DOWN | SCE_CTRL_LEFT; break;
        case 6: controlData.buttons |= SCE_CTRL_LEFT; break;
        case 7: controlData.buttons |= SCE_CTRL_UP | SCE_CTRL_LEFT; break;
        default: break;
    }

    // DirectInput reports already use 0x7F as center and 0 = up for Y.
    controlData.leftX  = leftX;
    controlData.leftY  = leftY;
    controlData.rightX = rightX;
    controlData.rightY = rightY;

    logReport(buffer);
}

void SwitchProController::loadRemapConfig()
{
    remapCount = 0;

    SceUID fd = ksceIoOpen("ux0:data/vitacontrol-8bitdo.cfg", SCE_O_RDONLY, 0);
    if (fd < 0)
        return;

    char buffer[512];
    int length = ksceIoRead(fd, buffer, sizeof(buffer) - 1);
    ksceIoClose(fd);

    if (length <= 0)
        return;

    buffer[length] = '\0';
    char *cursor = buffer;

    // Skip a UTF-8 byte-order mark written by some text editors
    if (length >= 3 &&
        (unsigned char)cursor[0] == 0xEF &&
        (unsigned char)cursor[1] == 0xBB &&
        (unsigned char)cursor[2] == 0xBF)
        cursor += 3;

    while (*cursor && remapCount < MAX_REMAPS)
    {
        // Skip whitespace and blank lines
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
            cursor++;

        if (!*cursor)
            break;

        // Comments start with '#'
        if (*cursor == '#')
        {
            while (*cursor && *cursor != '\n')
                cursor++;
            continue;
        }

        char *command = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n')
            cursor++;
        size_t commandLength = cursor - command;

        while (*cursor == ' ' || *cursor == '\t')
            cursor++;

        char *from = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n')
            cursor++;
        size_t fromLength = cursor - from;

        while (*cursor == ' ' || *cursor == '\t')
            cursor++;

        char *to = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n')
            cursor++;
        size_t toLength = cursor - to;

        // Ignore anything else on the line
        while (*cursor && *cursor != '\n')
            cursor++;

        if (tokenEquals(command, commandLength, "map"))
        {
            uint32_t fromMask = buttonMaskFromName(from, fromLength);
            uint32_t toMask = buttonMaskFromName(to, toLength);

            if (fromMask && toMask)
            {
                remaps[remapCount].from = fromMask;
                remaps[remapCount].to = toMask;
                remapCount++;
            }
        }
    }
}

void SwitchProController::applyRemap()
{
    if (remapCount == 0)
        return;

    // Evaluate every mapping against the original button state so two lines
    // can swap buttons without one mapping undoing the other.
    uint32_t original = controlData.buttons;
    uint32_t remapped = original;

    for (int i = 0; i < remapCount; i++)
    {
        if (original & remaps[i].from)
        {
            remapped &= ~remaps[i].from;
            remapped |= remaps[i].to;
        }
    }

    controlData.buttons = remapped;
}

void SwitchProController::logReport(uint8_t *buffer)
{
    // Log the first 200 distinct reports to ux0:data so the layout can be
    // verified on a real controller. Each line is 16 bytes of hex.
    if (logCount >= 200)
        return;

    if (logCount > 0 && memcmp(buffer, lastLogged, sizeof(lastLogged)) == 0)
        return;

    static const char hex[] = "0123456789ABCDEF";
    char line[3 * 16 + 16];
    int pos = 0;

    if (logCount == 0)
    {
        const char *header = "-- VitaControl 8BitDo report log --\n";
        SceUID fd = ksceIoOpen("ux0:data/vitacontrol-8bitdo.log",
            SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
        if (fd >= 0)
        {
            ksceIoWrite(fd, header, strlen(header));
            ksceIoClose(fd);
        }
    }

    line[pos++] = 'R';
    line[pos++] = ':';
    for (int i = 0; i < 16; i++)
    {
        line[pos++] = ' ';
        line[pos++] = hex[buffer[i] >> 4];
        line[pos++] = hex[buffer[i] & 0xF];
    }
    line[pos++] = '\n';

    SceUID fd = ksceIoOpen("ux0:data/vitacontrol-8bitdo.log",
        SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
    if (fd >= 0)
    {
        ksceIoWrite(fd, line, pos);
        ksceIoClose(fd);
    }

    memcpy(lastLogged, buffer, sizeof(lastLogged));
    logCount++;
}
