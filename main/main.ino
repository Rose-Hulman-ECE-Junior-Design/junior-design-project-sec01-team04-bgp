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
float Kp_angle = 1.0, Ki_angle = 0.0, Kd_angle = 0;
QuickPID AnglePID(&Input_angle, &Output_angle, &Setpoint_angle);


Telemetry tl;
VehicleState state = VehicleState::started;
Camera camera;
Api api(&tl, &state);

const int steering_servo_pin = 32;
const int motor_servo_pin = 33;

void setup() {
  Serial.println("Starting");
    Serial.begin(115200);
    delay(2000);

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


float mapfloat(float value, float fromlow, float fromhigh, float tolow, float tohigh) {
  value -= fromlow;
  value *= (tohigh - tolow) / (fromhigh-fromlow);
  value += tolow;

  return value;
}

// Update motor speed and steering angle
void update() {
    // Update PID loop based on camera values
    camera.read();
    // Input_angle = mapfloat(0.4 * camera.offset + 1.6 * mapfloat(camera.angle, 0, 180, -160, 160), -320, 320, -70, 80);
    Input_angle = mapfloat(camera.offset, -160, 160, -100, 100);
    AnglePID.Compute();
    Serial.print("output angle: ");
    Serial.print(Output_angle);
    Serial.print(", input angle: ");
    Serial.print(Input_angle);
    Serial.print(", camera angle: ");
    Serial.print(camera.angle);
    Serial.print(", camera offset: ");
    Serial.println(camera.offset);

    // delay(1000);




    // uint8_t speed_setpoint = speed_pid.step(0, camera.angle);
    // uint8_t new_angle = angle_pid.step(0, camera.angle + camera.offset);
    steering_servo.write(mapfloat(Output_angle, -100, 100, 10, 170));

    motor_servo.write(30); // TODO: Setup PID loop. For now, just using min speed
}

void loop() {
    switch (state) {
    case VehicleState::started:
        update();
        break;
    case VehicleState::stopped:
        break;
    }
}
