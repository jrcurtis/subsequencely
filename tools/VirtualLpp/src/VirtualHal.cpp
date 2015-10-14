
#include "app.h"
#include "VirtualLpp.h"

void hal_plot_led(u8 type, u8 index, u8 red, u8 green, u8 blue)
{
    VirtualLpp::getInstance()->plotLed(type, index, red, green, blue);
}

void hal_send_midi(u8 port, u8 status, u8 data1, u8 data2)
{
    VirtualLpp::getInstance()->sendMidi(port, status, data1, data2);
}

void hal_send_sysex(u8 port, const u8* data, u16 length)
{
    VirtualLpp::getInstance()->sendSysex(port, data, length);
}
