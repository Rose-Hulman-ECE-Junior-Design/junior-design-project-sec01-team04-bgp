#pragma once

#include "FS.h"

enum State {
  stopped,
  started,
};

typedef struct {
  int speed = 30;                  // Default motor speed
  double lookahead_distance = 12;  // Default lookahead distance
  double forward_offset = 8;       // Default forward offset
} Data;

// Represents the possible states of the vehicle
// Also holds configurable constants related to the controller
class VehicleState {
public:
  VehicleState(fs::FS& fs)
    : fs(fs){};

  const char* config_path = "/config.txt";
  fs::FS& fs;
  State state = State::stopped;
  Data data = (Data){};

  void read();
  void write();
};

// enum VehicleState {
//   stopped,
//   started,
// };
// VehicleState state = VehicleState::stopped; // Current state of the vehicle
// int speed = 30; // Default motor speed
// double lookahead_distance = 12; // Default lookahead distance
// double forward_offset = 8; // Default forward offset
