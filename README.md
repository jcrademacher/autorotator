# Autorotator


## Prerequisites

[Brew](brew.sh) is strongly encouraged for use of this software on MacOSX. 

Install the required libraries and packages: 
<br>
**Homebrew (Ubuntu, WSL, MacOS X):** `brew install boost cmake` <br>
**Linux Native (Ubuntu, WSL)**: `sudo apt install cmake libboost-dev libboost-program-options-dev libboost-filesystem-dev`

## Installation

This project uses the CMake build system. To create the executable, in the project root, run <br>

`mkdir build && cd build && cmake ../` <br>
`make`

The `autorotator` executable will be made in the build folder.

## Usage

First, your system LAN must be set to manual with a subnet mask of 255.255.0.0 and any IP address **EXCEPT** 192.168.1.20. This is the IP address of the stepper motor. 

Use `./autorotator --help` for more information. There are 3 modes: 

1. **Interactive**: The program will connect to the motor and give the option to set any angle between -180 and 180 degrees. You may also send any eSCL *query* command but you are not allowed to send any eSCL *set* commands, except for `SP`. The full set of host reference commands is [here](https://appliedmotion.s3.amazonaws.com/Host-Command-Reference_920-0002W_0.pdf)
2. **Target angle**: With the `--angle` flag the program will position the autorotator to the desired angle and exit gracefully
3. **Angle sweep**: With the `--start-angle`, `--end-angle`, and `--step` flags, the program will position the autorotator to each of the angles between `start-angle` (inclusive) and `end-angle` (exclusive), with a step of `step`

### Running an executable at each angle

With the `--exec` flag, you can pass an executable that the program will system-call at each angle, in any of the modes listed above. If `--exec` is not given, nothing is done at each angle. 

Additionally, if the argument to the `--exec` flag contains a **question mark** ("?"), then the program will string-replace the question mark with the angle that the executable was run at, with the angle rounded to the nearest **tenth of a degree** and the decimal point replaced with a comma, to prevent file extension collisions. This is particularly useful for running data-acquisition commands where you would like to specify in the executable what angle the executable was run at.

### Zeroing

Zeroing must first be manually performed, either with the motor powered off or the motor powered on and disabled. Then, run the program with the `--zero` flag to set the zero angle at the current position. This can also be performed by running `SP0` as an eSCL *set* command in the interactive mode. 