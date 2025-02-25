#include "state.h"

void CurveData::update(double x_start, double x_end, double y_start, double y_end) {
  this->x_start = constrain(x_start, this->x_min, this->x_max);
  this->x_end = constrain(x_end, this->x_start, this->x_max);
  this->y_start = constrain(y_start, this->y_min, this->y_max);
  this->y_end = constrain(y_end, this->y_min, this->y_max);
}

double CurveData::apply(double x) {
  x = constrain(x, this->x_min, this->x_max);
  double scaled = this->y_start + (x - this->x_min) * (this->y_end - this->y_start) / (this->x_end - this->x_start);
  return constrain(scaled, min(this->y_start, this->y_end), max(this->y_start, this->y_end));
}

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
  // Serial.printf("Read config file: ld = %f, fo = %f, speed = %d\n", this->data.lookahead_distance, this->data.forward_offset, this->data.speed);
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