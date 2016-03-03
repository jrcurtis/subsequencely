
#include "app.h"
#include "VirtualLpp.h"

void hal_plot_led(uint8_t type, uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    VirtualLpp::getInstance()->plotLed(type, index, red, green, blue);
}

void hal_send_midi(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2)
{
    VirtualLpp::getInstance()->sendMidi(port, status, data1, data2);
}

void hal_send_sysex(uint8_t port, const uint8_t* data, uint16_t length)
{
    VirtualLpp::getInstance()->sendSysex(port, data, length);
}

void hal_read_flash(uint32_t offset, uint8_t *data, uint32_t length)
{
    VirtualLpp::getInstance()->readFlash(offset, data, length);
}

void hal_write_flash(uint32_t offset,const uint8_t *data, uint32_t length)
{
    VirtualLpp::getInstance()->writeFlash(offset, data, length);
}
