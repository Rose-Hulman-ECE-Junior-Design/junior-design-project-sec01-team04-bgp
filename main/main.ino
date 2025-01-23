#include <ESP32Servo.h>
#include <FastPID.h>

#include "telemetry.h"
#include "state.h"
#include "api.h"
#include "camera.h"

#define PRINTLN(arg) Serial.println(arg)

Servo motor_servo;
Servo steering_servo;
// PID objects. Variables are placeholders
uint16_t Kp1, Kp2, Ki1, Ki2, Kd1, Kd2, Hz, output_bits, output_signed;
FastPID angle_pid(Kp1, Ki1, Kd1, Hz, output_bits, output_signed);
FastPID speed_pid(Kp2, Ki2, Kd2, Hz, output_bits, output_signed);

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

    api.init();
    Serial.println("Initialized API");

    // Init PID controllers
    angle_pid.setOutputRange(10, 170);
    speed_pid.setOutputRange(50, 120);
    Serial.println("Initialized PID controllers");

    // Init motors
    motor_servo.attach(motor_servo_pin, 800, 2000);
    steering_servo.attach(steering_servo_pin, 800, 2000);
    delay(1000);
    motor_servo.write(0);
    steering_servo.write(0);
    delay(3000);
    Serial.println("Initialized motors");

    // Init camera
    camera.init();
    Serial.println("Initialized camera");
}

// Update motor speed and steering angle
void update() {
    // Update PID loop based on camera values
    camera.read();

    uint8_t speed_setpoint = speed_pid.step(0, camera.angle);
    uint8_t new_angle = angle_pid.step(0, camera.angle + camera.offset);
    steering_servo.write(new_angle);

    motor_servo.write(50); // TODO: Setup PID loop. For now, just using min speed
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
