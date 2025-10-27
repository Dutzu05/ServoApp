# ServoApp
A simple project I make in order to apply the OOP principles (the file ServoControllerV2) in Arduino that allows the user to use a servo motor using keyboard inputs. 
Also the other files are for an actual app. 
This app turns your ESP32 into a Wi-Fi-controlled servo controller. When it powers on, it connects to your Wi-Fi network and starts a small TCP server that listens on port 3333. From your computer or phone, you can connect to that IP and port (for example using PuTTY or a socket program) and send simple text commands like “SET base 120”.
The ESP32 reads those messages, prints them in the Serial Monitor, and moves the connected servo motor smoothly to the requested angle. You can keep sending commands such as “SET base 45” or “SET base 180”, and the servo will continuously adjust to follow them in real time. All the while, the ESP32 can send text responses back through the same connection to confirm actions.
