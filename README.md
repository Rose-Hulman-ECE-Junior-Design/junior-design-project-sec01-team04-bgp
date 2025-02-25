# Junior Design Project

## Where to find the code

The most up to date code includes everything in the [`main`](main) directory.
The [`telemetry`](main/telemetry.h) files are for gathering telemetry data to
be sent from the device, like the current use or battery voltage. The
[`camera`](main/camera.h) files are for reading from the HuskyLens camera and
calculating the new steering angle for the vehicle based on the line. The
[`api`](main/api.h) files are for setting up the webserver that both allows
devices to access the dashboard webpage and provides an API for clients to
control the vehicle. The webpage accesses this API and presents the data on a
user-friendly GUI. The webpage and other web files are stored in the
[`www`](main/data/www) directory. The [`main`](main/main.ino) file holds the
setup and loop functions. Finally, the [`state`](main/state.h) files have an
enum which represents the possible vehicle states and a class which holds the
vehicle's state and handles saving and loading it to the filesystem.

## Uploading New Code

Prerequisites

1. Install Arduino IDE 2.0 or higher
2. Install the following libraries:
    - "Adafruit INA219" by Adafruit
    - "ESP Async Web Server" by ESP32Async
    - "ESP32Servo" by Kevin Harrington, John K. Bennett
    - "EspSoftwareSerial" by Dirk Kaar, Peter Lerup
    - "mjson" by Cesanta Software Limited
3. Install the ESP32 Dev Module board
4. Install the [LittleFS Filesystem Uploader](https://github.com/earlephilhower/arduino-littlefs-upload) plugin
5. Obtain a USB A male to Micro USB male cable

Steps to upload new code

1. Open the sketch in Arduino IDE
2. Connect your computer to the vehicle using the USB cable
3. Press the "Upload" button in the Arduino IDE
4. Once the command displays the line "Connecting...", press and hold the boot button on the vehicle's ESP32 until the upload completes

Steps to update the dashboard files
5. Ensure the serial monitor is closed
6. Press Ctrl+Shift+P and search for "Upload LittleFS to Pico/ESP8266/ESP32" then press enter
7. Once the command displays the line "Connecting...", press and hold the boot button on the vehicle's ESP32 until the upload completes


## State Diagram

![state diagram](docs/state_diagram.png)


## UML Diagram

![uml diagram](docs/uml_diagram.png)


## Software Documentation Plan

The device device provides an API for use over HTTP. By connecting to the
vehicle's WiFi network, users can control the vehicle and view data about it.
This API allows language-agnostic programming support, and easy integration
into a GUI, such as a web or mobile app.

This API will provide detailed information about the status of the vehicle and
its power supply, which will be educational for the students because it will
introduce them to concepts such as power, voltage, and current. Being able to
manipulate the vehicle and observe how its power supply reacts will help them
develop an intuition for these concepts. This also ties into the requirement
that user will be able to see the instantaneous power draw over time during the
competition.


## Using the Dashboard

To access the vehicle dashboard, first turn on the vehicle and connect to its
WiFi network (The SSID is "ECE362TeamBGP"). Next, open a web browser and
navigate to the IP of the vehicle. This can be found by checking the serial
output as the vehicle starts up (It defaults to 192.168.4.1). For connecting to
serial, attach a USB cable from your device to the vehicle and use an
application like screen or the Arduino IDE to view the output. Set the baud
rate to 115,200.

You should be presented with the following screen:
![dashboard controls](docs/dashboard_controls.png)

You can start and stop the vehicle using the "Start" and "Stop" buttons as
shown above. To stop the vehicle based on a timer, set the value of the "Timer
Value (s)" input to the desired value in seconds, then click the "Timed Stop"
button. On the right, there is a box which displays the line detected by the
vehicle's camera. This is updated while the vehicle is driving. Below that,
there are 3 boxes each with two points connected by lines. These lines
represent how each of the 3 constants (motor speed, lookahead distance, and
forward offset) scale based on the steering angle of the vehicle. To change how
these constants scale, adjust each control point by clicking and dragging. The
values update in real time and the vehicle will save whatever changes you make.
For an explanation of how each of these values work, hover over the circle with
the question marks.  If you scroll down, you should see the graph which
displays telemetry data.  This is shown below.
![dashboard telemetry](docs/dashboard_telemetry.png)

You can enable and disable updating the telemetry graph by clicking the
"Enable" and "Disable" buttons as shown above. To clear the data on the graph,
click the "Clear" button. To change how quickly the graph refreshes with new
data, change the value in the box labeled "Telemetry Refresh (ms)". To download
the data as a csv file (for use in Excel or Matlab) click the "Download csv"
button.


## API Usage

The vehicle hosts a webserver on port 80 that both serves the webpage and
provides an API for programmatic usage. The API uses the [JSON-RPC](https://www.jsonrpc.org/)
protocol [over HTTP](https://www.simple-is-better.org/json-rpc/transport_http.html)
and can be accessed on the `/api` route.

An [example python script](examples/test.py) is included to demonstrate how the
API can be used programmatically. Any language can be used, so long as it is
able to make HTTP requests and serialize/parse JSON objects.


### API Functions

| Method Name  | Parameters                                                                               | Return                                                                                                                                           | Description                                                                                                                                                                                                                                                                                                                                              |
|--------------|------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| start        | none                                                                                     | none                                                                                                                                             | Starts the vehicle                                                                                                                                                                                                                                                                                                                                       |
| stop         | none                                                                                     | none                                                                                                                                             | Stops the vehicle                                                                                                                                                                                                                                                                                                                                        |
| set_curve    | `[curve_number: number, x_start: number, x_end: number, y_start: number, y_end: number]` | none                                                                                                                                             | Sets parameters for one of the "curves" used by the software to scale constants according to the steering angle. For curve_number, pass 0 to change the speed curve, 1 to change the lookahead distance, and 2 for the forward offset. The remaining parameters are for two points which represent the values that the vehicle will interpolate between. |
| get_defaults | `[curve_number: number]`                                                                 | `{ "x_min": number, "x_start": number, "x_end": number, "x_max": number, "y_min": number, "y_start": number, "y_end": number, "y_max": number }` | Gets the default parameters for a given curve                                                                                                                                                                                                                                                                                                            |
| telemetry    | none                                                                                     | `{ "battery_v": number, "current_ma": number, "power_mw": number }`                                                                              | Requests telemetry data from the vehicle. This includes battery voltage, current, and power                                                                                                                                                                                                                                                              |
| camera_view  | none                                                                                     | `{"target": { "x": number, "y": number }, "origin": { "x": number, "y": number }`                                                                | Requests camera data from the vehicle. This includes the target and origin points of the line that the camera detects.                                                                                                                                                                                                                                   |
