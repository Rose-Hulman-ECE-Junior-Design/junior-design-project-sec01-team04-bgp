#include "state.h"

void VehicleState::read() {
  File config = this->fs.open(config_path);
  if (!config) {
    Serial.println("Config file doesn't exist, reading default values");
    this->write();
    return;
  }

  if (config.isDirectory()) {
    Serial.println("Config file path is directory, reading default values");
    return;
  }

  config.read((uint8_t*)&this->data, sizeof(this->data));
  Serial.printf("Read config file: ld = %f, fo = %f, speed = %d\n", this->data.lookahead_distance, this->data.forward_offset, this->data.speed);
}

void VehicleState::write() {
  File config = this->fs.open(config_path, FILE_WRITE, true);
  if (!config) {
    Serial.println("Coudldn't create config file");
    return;
  }

  config.write((uint8_t*)&this->data, sizeof(this->data));
  Serial.println("Wrote config file");
}