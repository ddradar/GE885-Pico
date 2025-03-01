# GE885-Pico

[![last commit](https://img.shields.io/github/last-commit/ddradar/GE885-Pico "last commit")](https://github.com/ddradar/ddradar/commits/main)
[![release version](https://img.shields.io/github/v/release/ddradar/GE885-Pico?sort=semver "release version")](https://github.com/ddradar/ddradar/releases)
[![License](https://img.shields.io/github/license/ddradar/GE885-Pico)](LICENSE)

> [!WARNING]
> :construction: This project is still under development. :construction:

RP2040 based GE885-PWB(A) (a.k.a. DanceDanceRevolution Link Kit, PS1 Memory Card reader for System 573) emulator

## Features

### Supported

- None...

### Planed

- [ ] Read/Write PS1 Memory Card
- [ ] Read controller input (used by Guitar Freaks)

### Not Planed (but PR is welcome)

- [ ] Support other boards (e.g. Raspberry Pi Pico)
- [ ] Custom PCB layout
- [ ] Store memory card images on other storage (e.g. SD card)
- [ ] Authenticate and download/upload memory card images (like Brightwhite)
- [ ] JVS daisy chain

## References

- [Controllers and Memory Cards - PlayStation Specifications - psx-spx](https://psx-spx.consoledev.net/controllersandmemorycards/)
- JAMMA VIDEO STANDARD (THE THIRD EDITION)
- [MAME k573mcr implements](https://github.com/mamedev/mame/blob/master/src/mame/konami/k573mcr.cpp)
