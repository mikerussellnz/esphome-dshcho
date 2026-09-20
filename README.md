# ESPHome component for the DS-HCHO formaldehyde sensor.

```yaml
# Example ESPHome configuration:

external_components:
  - source: github://mikerussellnz/esphome-dshcho
    refresh: 30s

sensor:
  - platform: dshcho
    name: DS-HCHO
    uart_id: uart_hcho

uart:
  - id: uart_hcho
    tx_pin: GPIO10
    rx_pin: GPIO11
    baud_rate: 9600
```