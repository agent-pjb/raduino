# Changelog

All notable changes to this project are listed here.

## [1.0.0] - 2026-11-18

### Added
- Initial release of raduino: touchscreen FM radio with 7 preset stations, `<<` / `>>` station buttons and PLAY/PAUSE (mute).
- README with parts list, wiring tables, setup steps and troubleshooting.
- Wiring diagram for the Arduino Mega 2560, the Adafruit 2.8" TFT Touch Shield (capacitive) and the TEA5767N (`docs/wiring.png`, `docs/wiring.svg`).

### Changed (compared with the original sketch)
- The project and the on-screen title are now called "Raduino".
- The I2C comment now gives the Mega 2560 pins (SDA 20, SCL 21) instead of the Uno's A4/A5.
- `maxstation` is now calculated from the size of the `stations` array. It was hard-coded to 8 while the list has 7 entries, so `>>` and `<<` could select a preset that does not exist.
- `gettouch()` returns `false` when the screen is not touched. It previously used a bare `return;` in a function that returns a value.
