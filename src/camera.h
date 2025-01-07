#include "HUSKYLENS.h"

class Camera {
public:
    int angle;
    int offset;

    void init();

    void read();

private:
    HUSKYLENS camera;
};
