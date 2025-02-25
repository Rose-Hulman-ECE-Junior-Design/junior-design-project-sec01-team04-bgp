#pragma once

#include "FS.h"

enum State {
  stopped,
  started,
};

typedef struct {
  double x_min;
  double x_start;
  double x_end;
  double x_max;
  double y_min;
  double y_start;
  double y_end;
  double y_max;

  void update(double x_start, double x_end, double y_start, double y_end);

  double apply(double x);
} CurveData;

typedef struct {
  CurveData speed = { 0, 0, 90, 90, 0, 30, 30, 120 };              // Default motor speed
  CurveData lookahead_distance = { 0, 0, 90, 90, 0, 10, 10, 30 };  // Default lookahead distance
  CurveData forward_offset = { 0, 0, 90, 90, 0, 4, 4, 30 };        // Default forward offset
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
  Data data = {};

  void read();
  void write();
};
