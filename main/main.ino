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

// Telemetry, VehicleState, API, and Camera objects
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

// Update motor speed and steering angle based on pure pursuit algorithm
void update() {
  if (!camera.read()) return;

  double angle = camera.get_servo_angle();
  double speed = state.data.speed.apply(abs(camera.steering_angle));
  steering_servo.write(angle);
  motor_servo.write(speed);
  Serial.printf("Angle = %f, Speed = %f\n", camera.steering_angle, speed);
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