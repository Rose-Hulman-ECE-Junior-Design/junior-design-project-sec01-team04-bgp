#pragma once

// Represents the possible states of the vehicle
enum VehicleState {
    stopped,
    started,
};

VehicleState state = VehicleState::stopped; // Current state of the vehicle
int speed = 30; // Default motor speed
double lookahead_distance = 14; // Default lookahead distance
double forward_offset = 8; // Default forward offset
