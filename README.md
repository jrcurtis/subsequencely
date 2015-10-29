# Subsequencely

Welcome to the Github page about Subsequencely, an 8 track performance sequencer
for the Launchpad Pro built on the Launchpad Pro open firmware, which you can
read about [here](https://github.com/dvhdr/launchpad-pro).

If you'd like to see what the program can do, check out the user manual in the
resources directory, or check out [this video](https://youtu.be/_G3-Z9f-Xr0), or
just upload resources/subsequencely.syx to your Launchpad Pro and try it out.

![Customizable faders](http://i.imgur.com/JIs1cOp.jpg) ![Drum pad layout](http://i.imgur.com/VusxP8s.jpg)
![Session mode](http://i.imgur.com/FSUyeEX.jpg) ![Step sequencer](http://i.imgur.com/qDviuut.jpg)

# Be aware

This software hasn't been thoroughly tested, and may be buggy, or not perform up
to the standards of dedicated hardware sequencers. If you encounter any
problems, or have a suggestion, let me know by creating an issue.

# VirtualLpp

This repo also houses a wrapper program called VirtualLpp in the tools directory
that can be used for debugging Launchpad Pro firmware code. It's a simple
wrapper around the Launchpad open firmware API that uses OpenGL to draw a
software Launchpad onscreen that you can interact with using a mouse or with a
real Launchpad as a controller.

Currently there's only an XCode project file in there, but if you'd like to use
it on another platform, you should be able to use the
[Cinder](https://libcinder.org) project generator and add the source files to it
without too many problems.

# Uploading to a Launchpad Pro
Now you've got some nice new code to run! To upload it to your Launchpad Pro, you'll need a sysex tool for your host platform (I'd love to get it working from the virtual machine, but that's for later).  I recommend [Sysex Librarian](http://www.snoize.com/SysExLibrarian/) on OS X, and [MIDI OX](http://www.midiox.com/) on Windows.  On Linux, I'll bet you already have a tool in mind.

I won't describe how to use these tools, I'm sure you already know - and if you don't, their documentation is superior to mine!  Here's what you need to do:

1. Unplug your Launchpad Pro
2. Hold the "Setup" button down while connecting it to your host via USB (ensure it's connected to the host, and not to a virtual machine!)
3. The unit will start up in "bootloader" mode
4. Send your launchpad_pro.syx file to the device MIDI port - it will briefly scroll "upgrading..." across the grid.
5. Wait for the update to complete, and for the device to reboot!

Tip - set the delay between sysex messages to as low a value as possible, so you're not waiting about for ages while the firmware uploads!
