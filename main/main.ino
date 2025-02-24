#include <ESP32Servo.h>
#include <QuickPID.h>
#include <LittleFS.h>

#include "telemetry.h"
#include "state.h"
#include "api.h"
#include "camera.h"

// Servos
Servo motor_servo;
Servo steering_servo;

const int steering_servo_pin = 32;
const int motor_servo_pin = 33;

// Main loop delay
const int loop_delay = 100;

// PID object and constants
float Input_angle, Output_angle, Setpoint_angle = 0;
float Kp_angle = 1, Ki_angle = 0.1, Kd_angle = 0;
QuickPID AnglePID(&Input_angle, &Output_angle, &Setpoint_angle);

// Telemetry, VehicleState, API, and camera objects
Telemetry tl;
VehicleState state(LittleFS);
Camera camera(&state);
Api api(&state, LittleFS, &tl, &camera);


void setup() {
  Serial.println("Starting");
  Serial.begin(115200);
  delay(2000);

  // Init telemetry
  tl.init();
  Serial.println("Initialized telemetry");

  // Init filesystem
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("Initialized filesystem");

  // Read state from config file
  state.read();

  api.init();
  Serial.println("Initialized API");

  // Init PID controllers
  AnglePID.SetTunings(Kp_angle, Ki_angle, Kd_angle);
  AnglePID.SetMode(AnglePID.Control::automatic);
  AnglePID.SetOutputLimits(-100, 100);
  Serial.println("Initialized PID controller");

  // Init motors
  motor_servo.attach(motor_servo_pin, 800, 2000);
  steering_servo.attach(steering_servo_pin, 800, 2000);
  delay(1000);
  motor_servo.write(0);
  steering_servo.write(90);
  delay(3000);
  Serial.println("Initialized motors");

  // Init camera
  camera.init();
  Serial.println("Initialized camera");
}

// Update motor speed and steering angle based on naive PID loop algorithm
void old_update() {
  if (!camera.old_read()) return;

  Serial.println("Updated");
  Input_angle = map_double(camera.offset, -160, 160, -100, 100);
  AnglePID.Compute();

  steering_servo.write(map_double(Output_angle, -100, 100, 10, 170));
  motor_servo.write(state.data.speed.apply(Output_angle));
}

// Update motor speed and steering angle based on pure pursuit algorithm
void update() {
  if (!camera.read()) return;

  double angle = camera.get_servo_angle();
  steering_servo.write(angle);
  motor_servo.write(state.data.speed.apply(angle));
}

void loop() {
  switch (state.state) {
    case State::started:
      update();
      delay(loop_delay);
      break;
    case State::stopped:
      motor_servo.write(0);
      steering_servo.write(90);
      delay(loop_delay);
      break;
  }
}