```yaml
external_components:
  - source: github://mikerussellnz/esphome-dsco220
    refresh: 30s

sensor:
  - platform: dshcho
    name: DS-HCHO

uart:
  tx_pin: D0
  rx_pin: D1
  baud_rate: 9600
```