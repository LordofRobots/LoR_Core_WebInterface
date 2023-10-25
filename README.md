# LoR_Core_WebInterface

Contains the configurations for controlling a MiniBot via Web Interface.

**Arduino Board selection:** ESP32 dev module 

**User Guide to Web Interface Control :** [Download User Guide](https://tr.ee/w6C960lNGT)

**LoR_Core Module :** [Download Data Sheet](https://tr.ee/klJ5_gKmH-)

**Beginners Guide to minibots:** [Download beginner's Guide](https://tr.ee/QhCzRJD69O)

**Link Tree :** [Useful Links to Web Pages and Documentation](https://linktr.ee/LordofRobots)



This Arduino sketch serves as a comprehensive framework for remotely controlling a MiniBot via a web interface. Utilizing libraries for WiFi, HTTP server, and Adafruit's NeoPixel, the code establishes the MiniBot as a WiFi access point with the SSID 'MiniBot'. Users can connect to this network and access the web interface by navigating to the MiniBot's local IP address (10.0.0.1) in a web browser or by using the mDNS address 'robot.local'.

Once connected, the web interface provides buttons for directional control, speed settings, and custom functions. The code is structured to be modular and easily customizable. It includes adjustable parameters for setting high and low speeds for the robot's motors, which are controlled using PWM (Pulse Width Modulation). Additionally, the sketch features NeoPixel LED control, providing visual feedback based on the robot's actions.

The HTTP server is configured to listen on port 80 and includes URI handlers for serving the web interface and processing incoming commands. All actions are event-driven, making the system highly responsive. The code is well-commented, facilitating easy modifications and feature additions.


# Code content:

### Libraries and Configurations
- Imports necessary libraries for WiFi, HTTP server, and NeoPixel functionalities.
- Sets the version and WiFi credentials for the robot.

### Customizable Parameters
- Allows users to set high and low speeds for the MiniBot's movement.

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
