# ESPHome component for the DS-HCHO formaldehyde sensor.

The sensor uses a request/response UART protocol. The datasheet does not specify
the UART baud rate; this example uses 9600 baud, which is the common default for
this sensor. The sensor response contains ten bytes:

```text
42 4D 08 14 05 03 00 0A 00 BD
|  |  |  |  |  |     |     checksum
|  |  |  |  |  |     measurement (0x000A)
|  |  |  |  |  quantity (0x03 = divide by 100)
|  |  |  |  unit (0x05 = mg/m³)
|  |  |  gas type (0x14 = HCHO)
|  |  response command
|  |  fixed header
```

The example response represents `10 / 100 = 0.10 mg/m³`. The checksum is the
16-bit sum of the first eight bytes (`0x00BD`).

Connect the sensor as follows:

| Sensor pin | Function | ESPHome connection |
| --- | --- | --- |
| 1 | VCC | 3.3 V |
| 2 | GND | GND |
| 6 | TX | UART `rx_pin` |
| 7 | RX | UART `tx_pin` |

Pins 4 and 5 are not connected.

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