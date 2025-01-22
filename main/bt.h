#include "BluetoothSerial.h"
#include "telemetry.h"

// Class to provide the Bluetooth API
class Bluetooth {
public:
    // Response sent from vehicle
    struct Response {
        enum { OK, ERR } code;
        void* payload;
    };

    // Command received from applications
    enum Cmd {
        stop,
        start,
        telemetry,
    };

    // Start Bluetooth serial connection
    void init(char* name);

    // Read and interpret command sent to API
    void handle_cmd();

private:
    BluetoothSerial bt_serial;
    Telemetry tl;

    Cmd parse_cmd(void);

    void serialize(void* payload);

    void send(char* data);

    char* recv(void);
};
