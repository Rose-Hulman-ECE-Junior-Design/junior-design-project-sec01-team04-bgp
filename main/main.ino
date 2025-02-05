#include <ESP32Servo.h>
#include <QuickPID.h>

#include "telemetry.h"
#include "state.h"
#include "api.h"
#include "camera.h"

#define PRINTLN(arg) Serial.println(arg)

Servo motor_servo;
Servo steering_servo;
// PID objects. Variables are placeholders
float Input_angle, Output_angle, Setpoint_angle = 0;
float Kp_angle = 1, Ki_angle = 0.1, Kd_angle = 0;
QuickPID AnglePID(&Input_angle, &Output_angle, &Setpoint_angle);
Telemetry tl;
VehicleState state = VehicleState::stopped;
Camera camera;
Api api(&tl, &state);

const int steering_servo_pin = 32;
const int motor_servo_pin = 33;

void setup() {
  Serial.println("Starting");
    Serial.begin(115200);
    delay(2000);

    tl.init();
    Serial.println("Initialized telemetry");
    
    api.init();
    Serial.println("Initialized API");

    // Init PID controllers
    // Setpoint_angle = 90;
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

// double map_double(double value, double from_low, double from_high, double to_low, double to_high) {
//   value -= from_low;
//   value *= (to_high - to_low) / (from_high - from_low);
//   value += to_low;

//   if (value > to_high) {
//     return to_high;
//   } else if (value < to_low) {
//     return to_low;
//   } else {
//     return value;
//   }
// }

void old_update() {
  if (!camera.old_read()) return;

  Serial.println("Updated");
  Input_angle = map_double(camera.offset, -160, 160, -100, 100);
  AnglePID.Compute();

  steering_servo.write(map_double(Output_angle, -100, 100, 10, 170));
  motor_servo.write(30);

  delay(100);
}

// Update motor speed and steering angle
void update() {
    // Update PID loop based on camera values
    if (!camera.read()) return;
    // Input_angle = mapFloat(0.4 * camera.offset + 1.6 * mapFloat(camera.angle, 0, 180, -160, 160), -320, 320, -70, 80);
    // Input_angle = mapFloat(camera.offset, -160, 160, -100, 100);
    // Input_angle = mapFloat(camera.angle, 0, 180, -100, 100);
    // AnglePID.Compute();
    // Serial.print("output angle: ");
    // Serial.print(Output_angle);
    // Serial.print(", input angle: ");
    // Serial.print(Input_angle);
    Serial.print("camera angle: ");
    Serial.print(camera.angle);
    Serial.print(", steering angle: ");
    Serial.println(camera.steering_angle);
    // Serial.print(", camera offset: ");
    // Serial.println(camera.offset);

    delay(5000);

    // uint8_t speed_setpoint = speed_pid.step(0, camera.angle);
    // uint8_t new_angle = angle_pid.step(0, camera.angle + camera.offset);
    // steering_servo.write(camera.get_servo_angle());

    motor_servo.write(30); // TODO: Setup PID loop. For now, just using min speed

    delay(100);
}

void loop() {
    switch (state) {
    case VehicleState::started:
        // update();
        Serial.println("About to update");
        old_update();
        break;
    case VehicleState::stopped:
        motor_servo.write(0);
        break;
    }
}
