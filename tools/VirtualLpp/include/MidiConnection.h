
#pragma once

#include <vector>
#include <string>
#include <memory>

#include <RtMidi.h>

using namespace std;

class MidiConnection
{
public:
    enum InOut
    {
        INPUT,
        OUTPUT
    };

    static const int NONE = 0;
    static const int VIRTUAL = 1;

    MidiConnection(InOut inout, const string& virtualPortName);

    void connect(int id);

    const vector<string>& getPortNames();

    int getId()
    {
        if (portNumber == -1)
        {
            return 0;
        }
        else if (isVirtual)
        {
            return 1;
        }
        else
        {
            return portNumber + EXTRA_PORTS;
        }
    }

    shared_ptr<RtMidi> getPort() { return port; }

    shared_ptr<RtMidiIn> getInPort()
    {
        return dynamic_pointer_cast<RtMidiIn>(port);
    }

    shared_ptr<RtMidiOut> getOutPort()
    {
        return dynamic_pointer_cast<RtMidiOut>(port);
    }

private:
    InOut inout;
    shared_ptr<RtMidi> port;
    string portName;
    string virtualPortName;
    int portNumber;
    bool isVirtual;

    static vector<string> portNames;

#ifndef __WINDOWS_MM__
    static const int EXTRA_PORTS = 2; // +2 for "None" and "Virtual"
#else
    static const int EXTRA_PORTS = 1; // +1 for "None"
#endif

};
