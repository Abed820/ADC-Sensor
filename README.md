# ADC Sensor Log Processor

**Author**: Abdulsalam Almutairi (Student ID: 24061915)  
**Module**: UFMFGT-15-1 Programming for Engineers | Resit Coursework 2025-26  
**Repository**: [https://github.com/Abed820/ADC-Sensor.git](https://github.com/Abed820/ADC-Sensor.git)

## Project Overview
This project is an embedded sensor data processor written in C (C99 standard). It reads a binary log file (`adc_sensor_log.bin`) containing 16-byte packed structs, performs statistical analysis (mean, min, max, std_dev, RMS) on the voltage data across 4 channels, checks for sequence gaps (dropped samples), and detects faults (overvoltage, undervoltage, sensor faults, out of range).

## Build Instructions

### Option 1: Using GCC (Command Line)
You must link the math library (`-lm`) when compiling manually.
```bash
gcc -std=c99 -Wall -Wextra -o adc_sensor_processor.exe main.c adc.c io.c stats.c -lm
```

### Option 2: Using CMake
A `CMakeLists.txt` file is provided for automated builds.
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Option 3: Using CLion (Recommended)
1. Open CLion and select **File > Open**.
2. Select the folder containing this project.
3. CLion will automatically detect the `CMakeLists.txt` and configure the project.
4. Click the green **Run** button to build and execute.

## Usage
Run the executable and pass the binary file as the single argument:
```bash
.\adc_sensor_processor.exe adc_sensor_log.bin
```

## Expected Output
If successful, the terminal will print the exact following output:

```text
Successfully loaded 4000 records into memory.
Finished computing stats for all 4 channels.
Checked sequences: Found 2 gap(s) in the data.
All done! I've written the full report out to 'results.txt'.
```

A structured report will be saved to `results.txt` in the same directory.
