# PopnForwarder
Virtual keyboard driver with light effects for official Pop'n Music cabinet

## About

This small tool allows the official IO Board to be used like a keyboard for emulators or frontend usage on an official Pop'n Music cabinet. It also simulates AC lights (a quick and dirty port of the Arduino code from https://github.com/CrazyRedMachine/UltimatePopnController ).

## How to use

- Compile with XP compatibility using your favorite method (I personally tested this with CLion/MinGW).

- Retrieve `ezusb.dll` from any Pop'n Music data folder and place it in the same folder as the forwarder

- Launch the forwarder

- Upon initialization, your panel buttons should trigger virtual keypresses so you can map them in any program compatible with keyboard input.

## AC Lights Simulation

In addition to simulating keypresses, this tool simulates AC lighting so playing with an emulator still feels kinda like the authentic arcade experience.

Pressing a button will get it to light up, and will trigger blinking lights from the side pillars, blue most of the time, red with 1/7 probability, and purple with 1/20 probability.

The top neons follow a fill-empty pattern whose speed is dynamically adjusted with the rate at which you press buttons (might need more work tweaking values).

## Known issues

Simulated keypresses won't work if klock is active.

