# Junior Design Project

## Where to find the code

The most up to date code includes everything in the `src` directory.
The `telemetry` files are for gathering telemetry to be sent from the device,
like the current use or battery voltage.
The `camera` files are for reading from the HuskyLens camera and calculating
the angle/offset of the vehicle relative to the line.
The `bt` files are for providing an API over Bluetooth to control the vehicle.
The `main` files hold the setup and loop functions.


## State Diagram

![state diagram](state_diagram.png)


## UML Diagram

![uml diagram](uml_diagram.png)


## Software Documentation Plan

The device device provides an API for use over Bluetooth. The specifications
are still in the works, but it will allow language-agnostic programming
support, and easy integration into a GUI, such as a web or mobile app.


This API will provide detailed information about the status of the vehicle and
its power supply, which will be educational for the students because it will
introduce them to concepts such as power, voltage, and current. Being able to
manipulate the vehicle and observe how its power supply reacts will help them
develop an intuition for these concepts.
