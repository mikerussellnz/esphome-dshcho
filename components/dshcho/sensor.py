import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart

DEPENDENCIES = ["uart"]

dshcho_ns = cg.esphome_ns.namespace("dshcho")
DSHCHOSensor = dshcho_ns.class_(
    "DSHCHOSensor", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        DSHCHOSensor,
        unit_of_measurement="mg/m³",
        icon="mdi:flask",
        accuracy_decimals=3,
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)