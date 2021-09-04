[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=WT735CX4UMZ9U)

# PopnForwarder
Virtual keyboard driver with light effects for official Pop'n Music cabinet

## About

This small tool allows the official IO Board to be used like a keyboard for emulators or frontend usage on an official Pop'n Music cabinet. It also simulates AC lights (a quick and dirty port of the Arduino code from https://github.com/CrazyRedMachine/UltimatePopnController ).

## How to use

- Compile with XP compatibility using your favorite method (I recommend creating a new Win32 Console app project in Visual C++ 2010 Express then pasting this code).

- Retrieve `ezusb.dll` and `libavs-win32.dll` from any Pop'n Music data folder and place it in the same folder as the forwarder

- Launch the forwarder

- Upon initialization, your panel buttons should trigger virtual keypresses so you can map them in any program compatible with keyboard input.

## AC Lights Simulation

In addition to simulating keypresses, this tool simulates AC lighting so playing with an emulator still feels kinda like the authentic arcade experience.

Pressing a button will get it to light up, and will trigger blinking lights from the side pillars, blue most of the time, red with 1/7 probability, and purple with 1/20 probability.

The top neons follow a fill-empty pattern whose speed is dynamically adjusted with the rate at which you press buttons (might need more work tweaking values).

## Known issues

Simulated keypresses won't work if klock is active.


## Donation

If this project helps you and you want to give back, you can help me with my future projects.

While not necessary, donations are much appreciated and will only go towards funding future github projects (arcade hardware ain't cheap :( ).

Of course you'll also receive my gratitude and I'll remember you if you post a feature request ;)

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate?hosted_button_id=WT735CX4UMZ9U)

