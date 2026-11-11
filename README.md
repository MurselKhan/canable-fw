# CANable Firmware

This repository contains sources for the slcan CANable firmware, based off of the CANtact firwmare. This firmware may still compile and run on the CANtact.

## Supported Commands

- `O` - Open channel 
- `C` - Close channel 
- `S0` - Set bitrate to 10k
- `S1` - Set bitrate to 20k
- `S2` - Set bitrate to 50k
- `S3` - Set bitrate to 100k
- `S4` - Set bitrate to 125k
- `S5` - Set bitrate to 250k
- `S6` - Set bitrate to 500k
- `S7` - Set bitrate to 750k
- `S8` - Set bitrate to 1M
- `M0` - Set mode to normal mode (default)
- `M1` - Set mode to silent mode
- `A0` - Disable automatic retransmission 
- `A1` - Enable automatic retransmission (default)
- `TIIIIIIIILDD...` - Transmit data frame (Extended ID) [ID, length, data]
- `tIIILDD...` - Transmit data frame (Standard ID) [ID, length, data]
- `RIIIIIIIIL` - Transmit remote frame (Extended ID) [ID, length]
- `rIIIL` - Transmit remote frame (Standard ID) [ID, length]
- `V` - Returns firmware version and remote path as a string
- `kNIIIMMM` - Let marker N mark frames with a matching standard ID [marker, ID, mask]
- `KNIIIIIIIIMMMMMMMM` - Let marker N mark frames with a matching extended ID [marker, ID, mask]
- `kN` - Switch marker N off

Note: Channel configuration commands must be sent before opening the channel. The channel must be opened before transmitting frames.

This firmware currently does not provide any ACK/NACK feedback for serial commands.

## Frame Markers

Two pins of the SWD header change level once per frame, so a logic analyzer can time bus events without decoding CAN.

- Marker 1 is SWCLK (PA14). It marks every frame by default.
- Marker 2 is SWDIO (PA13). It is off by default.

A marker toggles its pin for every matching frame that completes on the bus. That covers frames this adapter received and frames it sent and got acknowledged. Every edge is one frame, and it follows the end of that frame on the bus.

A frame matches when it has the ID type of the marker and `(frame ID XOR marker ID) AND mask` is zero. A zero mask matches every frame of either ID type. For example, `k10107FF` lets marker 1 mark the standard ID 0x010 only, and `k20107FE` lets marker 2 mark 0x010 and 0x011. The markers can be changed at any time.

The firmware drives both pins as outputs, so SWD is not available while it runs. Keep debug probes disconnected and flash through the bootloader.

## Building

Firmware builds with GCC. Specifically, you will need gcc-arm-none-eabi, which
is packaged for Windows, OS X, and Linux on
[Launchpad](https://launchpad.net/gcc-arm-embedded/+download). Download for your
system and add the `bin` folder to your PATH.

Your Linux distribution may also have a prebuilt package for `arm-none-eabi-gcc`, check your distro's repositories to see if a build exists.

- If you have a CANable device, you can compile using `make`. 
- If you have a CANtact or other device with external oscillator, you can compile using `make INTERNAL_OSCILLATOR=1`
- If you have an MKS CANable Pro V1.0, compile using `make MKS_PRO=1`. Its TX and RX LEDs are on PA0 and PA1 and light up on a low pin. The image is named `canable-mks-pro-<version>.bin`.

## Flashing with the Bootloader

Simply plug in your CANable with the BOOT jumper enabled (or depress the boot button on the CANable Pro while plugging in). Next, type `make flash` and your CANable will be updated to the latest firwmare. Unplug/replug the device after moving the boot jumper back, and your CANable will be up and running.

## Debugging

Debugging and flashing can be done with any STM32 Discovery board as a
programmer, or an st-link. You can also use other tools that support SWD.

To use an STM32 Discovery, run [OpenOCD](http://openocd.sourceforge.net/) using
the stm32f0x.cfg file: `openocd -f fw/stm32f0x.cfg`.

With OpenOCD running, arm-none-eabi-gdb can be used to load code and debug.

## Contributors

- [Ethan Zonca](https://github.com/normaldotcom) - New features, HAL updates, Makefile fixes and code size optimization, updates for CANable
- [onejope](https://github.com/onejope) - Fixes to extended ID handling
- Phil Wise - Added dfu-util compatibility to Makefile

## License

See LICENSE.md
