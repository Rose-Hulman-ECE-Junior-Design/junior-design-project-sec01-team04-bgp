#include "BluetoothSerial.h"
#include "telemetry.h"

class Bluetooth {
public:
    struct Response {
        enum { OK, ERR } code;
        void* payload;
    };

    enum Cmd {
        stop,
        start,
        telemetry,
    };

    void init(char* name);

    void handle_cmd();

private:
    BluetoothSerial bt_serial;
    Telemetry tl;

    Cmd parse_cmd(void);

    void serialize(void* payload);

    void send(char* data);

    char* recv(void);
};
