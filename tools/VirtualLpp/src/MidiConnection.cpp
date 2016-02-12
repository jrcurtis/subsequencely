
#include "MidiConnection.h"

vector<string> MidiConnection::portNames;

MidiConnection::MidiConnection(InOut inout, const string& virtualPortName)
    : inout(inout),
      portName(),
      virtualPortName(virtualPortName),
      portNumber(-1),
      isVirtual(false)
{
    if (portNames.size() == 0)
    {
        portNames.push_back("None");
        #ifndef __WINDOWS_MM__
        portNames.push_back(virtualPortName);
        #endif
    }

    if (inout == INPUT)
    {
        port.reset(new RtMidiIn());
    }
    else
    {
        port.reset(new RtMidiOut());
    }
}

void MidiConnection::connect(int id)
{
    port->closePort();

    if (id == 0) // None chosen
    {
        isVirtual = false;
        portNumber = -1;
        return;
    }
    else if (id == 1 && EXTRA_PORTS == 2)
    {
        isVirtual = true;
        portNumber = 0;
        port->openVirtualPort(virtualPortName);
        portName = virtualPortName;
    }
    else
    {
        isVirtual = false;
        portNumber = id - EXTRA_PORTS;
        port->openPort(portNumber);
        portName = port->getPortName(portNumber);
    }

}

const vector<string>& MidiConnection::getPortNames()
{
    portNames.resize(port->getPortCount() + EXTRA_PORTS);

    for (int i = 0; i < port->getPortCount(); i++)
    {
        portNames[i + EXTRA_PORTS] = port->getPortName(i);
    }

    return portNames;
}
