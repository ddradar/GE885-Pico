# GE885-Pico

[![last commit](https://img.shields.io/github/last-commit/ddradar/GE885-Pico "last commit")](https://github.com/ddradar/ddradar/commits/main)
[![release version](https://img.shields.io/github/v/release/ddradar/GE885-Pico?sort=semver "release version")](https://github.com/ddradar/ddradar/releases)
[![License](https://img.shields.io/github/license/ddradar/GE885-Pico)](LICENSE)

> [!WARNING]
> :construction: This project is still under development. :construction:

RP2040 based GE885-PWB(A) (a.k.a. DanceDanceRevolution Link Kit, PS1 Memory Card reader for System 573) emulator

## How to build

### Pinout

|Type|Pin: Name|RP2040|Note|
|----|----------|----------|----|
|JVS|1: Sense(VBUS)|GP7[^1]||
|JVS|2: DATA-|GP6[^1]||
|JVS|3: DATA+|GP5[^1]||
|JVS|4: GND|GND||
|PSX|1: Data|GP0||
|PSX|2: Command|GP3||
|PSX|3: 7V|||
|PSX|4: GND|GND||
|PSX|5: 3.3V|3V3||
|PSX|6-1: Attention(1P)|GP14||
|PSX|6-2: Attention(2P)|GP15||
|PSX|7: Clock|GP2||
|PSX|8: IRQ|-||
|PSX|9: Acknowledge|GP8||

[^1]: JVS logic level is 5V, so use level shifter.

## Features

### Supported

- None...

### Planed

- [ ] Read/Write PS1 Memory Card on System 573
- [ ] Read controller input (used by Guitar Freaks)

### Not Planed (but PR is welcome)

- [ ] Support other boards (e.g. Raspberry Pi Pico)
- [ ] Custom PCB layout
- [ ] Read/Write PS1 Memory Card via PC
- [ ] Store memory card images on other storage (e.g. SD card)
- [ ] Authenticate and download/upload memory card images (like Brightwhite)
- [ ] JVS daisy chain

## References

- [Controllers and Memory Cards - PlayStation Specifications - psx-spx](https://psx-spx.consoledev.net/controllersandmemorycards/)
- JAMMA VIDEO STANDARD (THE THIRD EDITION)
- [MAME k573mcr implements](https://github.com/mamedev/mame/blob/master/src/mame/konami/k573mcr.cpp)
