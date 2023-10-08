# LoR_Core_WebInterface

This Arduino sketch provides a complete solution for controlling a robot via a web interface. It includes WiFi setup, HTTP server initialization, and motor control using PWM. The web interface offers buttons for directional control and custom functions. Additionally, it features NeoPixel LED control for visual feedback. The code is modular and well-commented, making it easy to customize.

# Code content:

### Libraries and Configurations
- Imports necessary libraries for WiFi, HTTP server, and NeoPixel functionalities.
- Sets the version and WiFi credentials for the robot.

### Customizable Parameters
- Allows users to set high and low speeds for the robot's movement.

### Pin Definitions
- Defines the GPIO pins used for LEDs, motors, and other peripherals.

### Motor Control
- Includes functions for controlling the speed and direction of motors.
- Configures PWM channels for each motor.

### NeoPixel Control
- Contains functions to control the color and state of NeoPixel LEDs.

### Custom Button Functions
- Provides placeholder functions for custom buttons labeled A, B, C, and D on the web interface.

### Web Page
- Embeds the HTML, CSS, and JavaScript code required for the web interface.
- The interface includes buttons for directional control and custom functions.

### HTTP Server
- Initializes an HTTP server that listens on port 80.
- Registers URI handlers for serving the web page and processing commands.

### HTTP Handlers
- Includes handlers for serving the web interface and processing robot movement commands sent from the web interface.

### WiFi Setup
- Configures the robot as a WiFi access point with a specific IP, gateway, and subnet.

### Setup Function
- Initializes Serial communication, sets up GPIO pins, starts the HTTP server, and configures NeoPixels.

### Loop Function
- Empty, as all actions are event-driven and handled by the HTTP server and web interface.
