# WeatherPanel 3.49 reconciliation

'WeatherPanel349/' is the canonical source implementation on the
'weatherpanel/source-reconciliation' branch.

## Decisions

- The source implementation retains the upstream WeatherPanel HKO photo,
  radar, and satellite workflows.
- The V2 hardware map from the deployment investigation is now the only board
  map in this source:
  - display QSPI: CS 45, SCK 47, D0 39, D1 48, D2 40, D3 21
  - display reset and backlight: GPIO 46
  - touch I2C: SDA 4, SCL 8, address 0x38, interrupt 1
  - SD_MMC 1-bit: clock 41, command 39, data0 40
- The standalone NMC satellite-clock implementation in 'opt/WeatherPanel'
  was not merged. It is a separate experiment with different image paths
  and should remain reference material until the board is tested.

## Fixes included

- Removed self-referential SD pin macros.
- Removed duplicate 'initTouch()' and 'listDir()' definitions.
- Standardised the JPEG decoder include and declared its PlatformIO dependency.
- Centralised the V2 pins in 'Config349.h' and recorded them in
  'board_config.json'.

## Validation status

Static source checks are complete. PlatformIO and Arduino CLI are not
installed on 'compute01', so compilation and flashing still need to be run
