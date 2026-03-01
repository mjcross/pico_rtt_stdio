# Pico *stdout* over SWD with Segger RTT

Using a spare Pico as a [PicoProbe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html) to upload and debug your application via the SWD pins is considerably more convenient and powerful than connecting and disconnecting the USB port.

Typically with this setup you configure your application to send *stdout* to the UART on pins 1 and 2, which the PicoProbe then relays to the host via a USB serial device; but it's actually quite straightforward to send it directly over the SWD with no need for a separate connection.

To send application output over SWD we can use [Segger RTT]((https://kb.segger.com/RTT)): an open real-time transfer interface supported by the C/C++ SDK, PicoProbe and OpenOCD.  

During a debug session OpenOCD exposes each RTT channel as a local TCP socket that we can view in a VSCode terminal or with an external tool like netcat `nc`.

## configuring RTT in the application

Simply include `pico_enable_stdio_rtt(test_stdio_rtt 1)` in your CMakeLists.txt, or explicitly link `pico_stdio_rtt` which does the same thing.

You can then just call `sdtio_init_all()`, `printf()` and so on as normal.

    Note that SWD will only work in a debug build.

## RTT in VSCode

To enable RTT in a VSCode debug configuration you can add a section to the `launch.json` file in your project's `.vscode` folder.
Put the following lines in the relevant configuration, normally *"Pico Debug (Cortex-Debug)"*:

    "rttConfig": {
        "enabled": true,
        "address": "auto",
        "decoders": [
            {
                "label": "",
                "port": 0,
                "type": "console"
            }
        ]
    }

*(see [cortex-debug launch attributes](https://github.com/Marus/cortex-debug/blob/master/debug_attributes.md))*

You should now be able to launch a debug session for an RTT enabled application, open the TERMINAL tab and select the RTT Console on the the right.

## RTT in a standalone debug session

Use OpenOCD to connect to the Debug Probe and select the correct target (rp2040 or rp2350):

    $ openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"

Open a second terminal and connect to OpenOCD via gdb:

    $ arm-none-eabi-gdb
    (gdb) target extended-remote localhost:3333

Flash the executable to the target: *('monitor' commands are passed to OpenOCD)*

    (gdb) monitor program <your_program>.elf verify reset

Find the RTT control block on the target and forward channel 0 to a local TCP socket :

    (gdb) monitor rtt setup 0x20000000 2048 "SEGGER RTT"
    (gdb) monitor rtt start
    (gdb) monitor rtt server start 60000 0

*(see [section 15.6 of the OpenOCD documentation](https://openocd.org/doc/html/General-Commands.html))*

You can now open a third terminal and connect to the local socket to see the RTT console:

    $ nc localhost 60000

Finally return to the second terminal and start the application:

    (gdb) continue