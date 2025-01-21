#include <ESP32Servo.h>
#include <FastPID.h>

#include "telemetry.h"
#include "state.h"
#include "api.h"

#define PRINTLN(arg) Serial.println(arg); SerialBT.println(arg)

Servo motor_servo;
Servo steering_servo;
// PID objects. Variables are placeholders
uint16_t Kp1, Kp2, Ki1, Ki2, Kd1, Kd2, Hz, output_bits, output_signed;
FastPid angle_pid(Kp1, Ki1, Kd1, Hz, output_bits, output_signed);
FastPid speed_pid(Kp2, Ki2, Kd2, Hz, output_bits, output_signed);

Telemetry tl;
VehicleState state;
Camera camera;
Api api(&tl, &state);

const int steering_servo_pin = 32;
const int motor_servo_pin = 33;

void setup() {
    Serial.begin(115200);
    api.init();
    delay(2000);

    // Init PID controllers
    angle_pid.setOutputRange(10, 170);
    speed_pid.setOutputRange(50, 120);

    // Init motors
    motor_servo.attach(motor_servo_pin, 800, 2000);
    steering_servo.attach(steering_servo_pin, 800, 2000);
    delay(1000);
    motor_servo.write(0);
    steering_servo.write(0);
    delay(3000);

    // Init camera
    camera.init();
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

// Leftover testing functions
void testServos(void) {
    motor_servo.write(120);
    steering_servo.write(10);
    delay(1000);
    motor_servo.write(85);
    steering_servo.write(90);
    delay(1000);
    motor_servo.write(50);
    steering_servo.write(170);
    delay(1000);
    motor_servo.write(0);
    steering_servo.write(90);
}

void testCamera(void) {
    if (!camera.request()) {
        PRINTLN(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    } else if (!camera.isLearned()) {
        PRINTLN(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    } else if (!camera.available()) {
        PRINTLN(F("No arrow appears on the screen!"));
    } else {
        while (camera.available()) {
            HUSKYLENSResult result = camera.read();
            PRINTLN(String() + F("Found arrow #") + result.ID + F(": from (") + result.xOrigin + F(", ") + result.yOrigin + F(") to (") + result.xTarget + F(", ") + result.yTarget + F(")"));
        }
    }
}

void testCurrentSensor(void) {
    float current_ma = ina219.getCurrent_mA();
    PRINTLN(String() + F("Current: ") + current_ma + F(" mA"));
}
