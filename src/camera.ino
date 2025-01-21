#include "camera.h"
#include <Wire.h>

void Camera::init() {
    Wire.begin();
    this.camera.begin(Wire);
}

void Camera::read() {
    if (this.camera.request() && this.camera.available()) {
        HUSKYLENSResult result = this.camera.read();
        this.offset = 320 - (result.xTarget - result.yTarget);
        this.angle = std::atan2(result.yTarget - result.yOrigin, result.xTarget - result.xOrigin);
    }
}
