#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "dshcho_sensor.h"

namespace esphome {
namespace dshcho {

static const char *TAG = "dshcho.sensor";

static constexpr uint8_t HCHO_READ_COMMAND[] = {0x42, 0x4D, 0x01, 0x00, 0x00, 0x00, 0x90};
static constexpr uint8_t HCHO_COMMAND_LENGTH = sizeof(HCHO_READ_COMMAND);
static constexpr uint8_t HCHO_RESPONSE_LENGTH = 10;
static constexpr uint8_t HCHO_RESPONSE_COMMAND = 0x08;
static constexpr uint8_t HCHO_GAS_TYPE = 0x14;
static constexpr uint8_t HCHO_UNIT = 0x05;
static constexpr uint32_t HCHO_READ_TIMEOUT_MS = 1000;

void DSHCHOSensor::setup() {
  ESP_LOGCONFIG(TAG, "DS-HCHO initialized");
}

void DSHCHOSensor::update() {
  while (this->available()) {
    uint8_t discarded_byte;
    this->read_byte(&discarded_byte);
  }

  this->flush();
  for (uint8_t index = 0; index < HCHO_COMMAND_LENGTH; index++) {
    this->write_byte(HCHO_READ_COMMAND[index]);
  }
  this->flush();

  uint8_t response[HCHO_RESPONSE_LENGTH];
  uint8_t bytes_read = 0;
  const uint32_t start = millis();

  while ((millis() - start) < HCHO_READ_TIMEOUT_MS) {
    if (!this->available()) {
      delay(10);
      continue;
    }

    uint8_t byte;
    if (!this->read_byte(&byte)) {
      continue;
    }

    if (bytes_read == 0) {
      if (byte == 0x42) {
        response[bytes_read++] = byte;
      }
      continue;
    }

    if (bytes_read == 1 && byte != 0x4D) {
      bytes_read = byte == 0x42 ? 1 : 0;
      if (bytes_read == 1) {
        response[0] = byte;
      }
      continue;
    }

    response[bytes_read++] = byte;
    if (bytes_read == HCHO_RESPONSE_LENGTH) {
      break;
    }
  }

  if (bytes_read != HCHO_RESPONSE_LENGTH) {
    ESP_LOGW(TAG, "Failed to read a complete HCHO response (%u/%u bytes)", bytes_read,
             HCHO_RESPONSE_LENGTH);
    this->status_set_warning();
    return;
  }

  if (response[2] != HCHO_RESPONSE_COMMAND) {
    ESP_LOGW(TAG, "Unexpected HCHO response command: 0x%02X", response[2]);
    this->status_set_warning();
    return;
  }

  if (response[3] != HCHO_GAS_TYPE || response[4] != HCHO_UNIT) {
    ESP_LOGW(TAG, "Unexpected HCHO response type/unit: 0x%02X/0x%02X", response[3], response[4]);
    this->status_set_warning();
    return;
  }

  const uint16_t checksum = (static_cast<uint16_t>(response[8]) << 8) | response[9];
  uint16_t calculated_checksum = 0;
  for (uint8_t index = 0; index < 8; index++) {
    calculated_checksum += response[index];
  }
  if (checksum != calculated_checksum) {
    ESP_LOGW(TAG,
             "Invalid HCHO response checksum: received 0x%04X, calculated 0x%04X; frame "
             "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
             checksum, calculated_checksum, response[0], response[1], response[2], response[3],
             response[4], response[5], response[6], response[7], response[8], response[9]);
    this->status_set_warning();
    return;
  }

  float data_quantity;
  switch (response[5]) {
    case 0x01:
      data_quantity = 1.0f;
      break;
    case 0x02:
      data_quantity = 10.0f;
      break;
    case 0x03:
      data_quantity = 100.0f;
      break;
    case 0x04:
      data_quantity = 1000.0f;
      break;
    default:
      ESP_LOGW(TAG, "Unexpected HCHO data quantity code: 0x%02X", response[5]);
      this->status_set_warning();
      return;
  }

  const uint16_t raw_value = (static_cast<uint16_t>(response[6]) << 8) | response[7];
  const float hcho = raw_value / data_quantity;
  ESP_LOGD(TAG, "HCHO: %.3f mg/m³", hcho);
  this->publish_state(hcho);
  this->status_clear_warning();
}

void DSHCHOSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "DS-HCHO sensor");
}

}  // namespace dshcho
}  // namespace esphome