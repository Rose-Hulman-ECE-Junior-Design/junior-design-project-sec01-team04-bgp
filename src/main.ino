#include <ESP32Servo.h>
#include <FastPID.h>

#include "main.h"
#include "bt.h"

#define PRINTLN(arg) Serial.println(arg); SerialBT.println(arg)

Servo motorServo;
Servo steeringServo;
// PID objects. Variables are placeholders
FastPid angle_pid(Kp1, Ki1, Hz, output_bits, output_signed);
FastPid speed_pid(Kp2, Ki2, Hz, output_bits, output_signed);

Camera camera;
Bluetooth bt;

const int steeringServoPin = 32;
const int motorServoPin = 33;

void setup() {
    Serial.begin(115200);
    bt.init("ECE362CarTeam04");
    delay(2000);

    // Init motors
    motorServo.attach(motorServoPin, 800, 2000);
    steeringServo.attach(steeringServoPin, 800, 2000);
    delay(1000);
    motorServo.write(0);
    steeringServo.write(0);
    delay(3000);

    // Init camera
    camera.init();
}

void loop() {
    // Handle API command
    bt.handle_cmd();

    if (stop) return;

    // Update PID loop based on camera values
    camera.read();
    uint8_t speed_setpoint = angle_pid.step(0, camera.angle);
    uint8_t new_speed = speed_pid.step(speed_setpoint, camera.offset);
    motorServo.write(new_speed);
}

// Leftover testing functions
void testServos(void) {
    motorServo.write(120);
    steeringServo.write(10);
    delay(1000);
    motorServo.write(85);
    steeringServo.write(90);
    delay(1000);
    motorServo.write(50);
    steeringServo.write(170);
    delay(1000);
    motorServo.write(0);
    steeringServo.write(90);
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
