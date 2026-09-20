#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "dshcho_sensor.h"

namespace esphome {
namespace dshcho {

static const char *TAG = "dshcho.sensor";

static constexpr uint8_t HCHO_READ_COMMAND[] = {0x42, 0x4D, 0x01, 0x00, 0x00, 0x00, 0x90};
static constexpr uint8_t HCHO_FRAME_LENGTH = sizeof(HCHO_READ_COMMAND);
static constexpr uint32_t HCHO_READ_TIMEOUT_MS = 1000;
static constexpr uint16_t DSHCHO_BAUD_RATE = 9600;

void DSHCHOSensor::setup() {
  ESP_LOGCONFIG(TAG, "DS-HCHO initialized");
}

void DSHCHOSensor::update() {
  while (this->available()) {
    uint8_t discarded_byte;
    this->read_byte(&discarded_byte);
  }

  this->flush();
  for (uint8_t index = 0; index < HCHO_FRAME_LENGTH; index++) {
    this->write_byte(HCHO_READ_COMMAND[index]);
  }
  this->flush();

  uint8_t response[HCHO_FRAME_LENGTH];
  uint8_t bytes_read = 0;
  const uint32_t start = millis();

  while ((millis() - start) < HCHO_READ_TIMEOUT_MS) {
    if (this->available() >= HCHO_FRAME_LENGTH) {
      while (this->available() && bytes_read < HCHO_FRAME_LENGTH) {
        if (!this->read_byte(&response[bytes_read])) {
          break;
        }
        bytes_read++;
      }
      break;
    }
    delay(10);
  }

  if (bytes_read != HCHO_FRAME_LENGTH || response[0] != 0x42 || response[1] != 0x4D ||
      response[2] != 0x01) {
    ESP_LOGW(TAG, "Failed to read a valid HCHO measurement");
    this->status_set_warning();
    return;
  }

  const uint16_t checksum = (static_cast<uint16_t>(response[5]) << 8) | response[6];
  const uint16_t calculated_checksum = response[0] + response[1] + response[2] + response[3] + response[4];
  if (checksum != calculated_checksum) {
    ESP_LOGW(TAG, "Invalid HCHO response checksum");
    this->status_set_warning();
    return;
  }

  const uint16_t raw_value = (static_cast<uint16_t>(response[3]) << 8) | response[4];
  const float hcho = raw_value / 1000.0f;
  ESP_LOGD(TAG, "HCHO: %.3f mg/m³", hcho);
  this->publish_state(hcho);
  this->status_clear_warning();
}

void DSHCHOSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "DS-HCHO sensor");
  ESP_LOGCONFIG(TAG, "  Baud rate: %u", DSHCHO_BAUD_RATE);
}

}  // namespace dshcho
}  // namespace esphome