#include <cstring>
#include <psp2kern/bt.h>

#include "controller.h"
#include "mempool.h"
#include "controllers/dualshock3_controller.h"
#include "controllers/dualshock4_controller.h"
#include "controllers/dualsense_controller.h"
#include "controllers/xbox_one_controller.h"
#include "controllers/xbox_one_controller_2016.h"
#include "controllers/switch_pro_controller.h"

inline void* operator new(std::size_t, void* __p) throw() { return __p; }

#define DECL_CONTROLLER(vid, pid, name) \
    case ((vid) << 16) | (pid): return new(Mempool::alloc(sizeof(name))) name(mac0, mac1, port)

static bool nameContains(const char *name, size_t length, const char *needle)
{
    for (size_t i = 0; i < length && name[i]; i++)
    {
        size_t j = 0;
        while (needle[j] && i + j < length && name[i + j])
        {
            char c = name[i + j];
            if (c >= 'A' && c <= 'Z')
                c += 'a' - 'A';
            if (c != needle[j])
                break;
            j++;
        }
        if (!needle[j])
            return true;
    }
    return false;
}

Controller *Controller::makeController(uint32_t mac0, uint32_t mac1, int port)
{
    // Get the VID and PID of the device with the given MAC address
    uint16_t id[2];
    ksceBtGetVidPid(mac0, mac1, id);

    // Get the device name too. Some 8BitDo modes can spoof another vendor's
    // VID/PID, so the name is a useful fallback for choosing the driver.
    char deviceName[0x79] = {};
    ksceBtGetDeviceName(mac0, mac1, deviceName);

    // A genuine Switch Pro identity must always use the Switch driver, even
    // if the name happens to contain "8BitDo" or "Ultimate".
    if (id[0] == 0x057E && id[1] == 0x2009)
        return new(Mempool::alloc(sizeof(SwitchProController))) SwitchProController(mac0, mac1, port, true);

    // 8BitDo's own VID is used in DirectInput mode (including the Ultimate
    // 2.4G's Bluetooth mode). There is no fixed PID to match here because the
    // same controller can report different PIDs depending on firmware/mode.
    // Fall back to the name for units that spoof a different VID over BT.
    if (id[0] == 0x2DC8 ||
        nameContains(deviceName, sizeof(deviceName), "8bitdo") ||
        nameContains(deviceName, sizeof(deviceName), "ultimate"))
        return new(Mempool::alloc(sizeof(SwitchProController))) SwitchProController(mac0, mac1, port, false);

    // Match the VID and PID to a controller type, and create one if it exists
    switch ((id[0] << 16) | id[1])
    {
        DECL_CONTROLLER(0x054C, 0x0268, DualShock3Controller);
        DECL_CONTROLLER(0x054C, 0x05C4, DualShock4Controller);
        DECL_CONTROLLER(0x054C, 0x09CC, DualShock4Controller);
        DECL_CONTROLLER(0x054C, 0x0CE6, DualSenseController);
        DECL_CONTROLLER(0x054C, 0x0DF2, DualSenseController);
        DECL_CONTROLLER(0x045E, 0x02E0, XboxOneController2016);
        DECL_CONTROLLER(0x045E, 0x02FD, XboxOneController);
        DECL_CONTROLLER(0x045E, 0x0B00, XboxOneController);
        DECL_CONTROLLER(0x045E, 0x0B05, XboxOneController);
        DECL_CONTROLLER(0x045E, 0x0B0A, XboxOneController);
    }

    return nullptr;
}

void Controller::requestReport(uint8_t type, uint8_t *buffer, size_t length)
{
    static SceBtHidRequest request;
    memset(&request, 0, sizeof(SceBtHidRequest));

    // Clear the buffer for read requests
    if (type == HID_REQUEST_READ)
        memset(buffer, 0, length);

    // Build a report request
    request.type   = type;
    request.buffer = buffer;
    request.length = length;
    request.next   = &request;

    // Send the request to the controller
    ksceBtHidTransfer(mac0, mac1, &request);
}

uint32_t Controller::calculateCrc(uint8_t *buffer, size_t length)
{
    // Calculate the CRC of the given data (used in requests for some controllers)
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        uint8_t ch = buffer[i];
        for (size_t j = 0; j < 8; j++)
        {
            uint32_t b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b) crc = crc ^ 0xEDB88320;
            ch >>= 1;
        }
    }
    return ~crc;
}
