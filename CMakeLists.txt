cmake_minimum_required(VERSION 3.10)
project(adc_sensor_processor C)

# Target the C99 standard as required by the coursework specification.
# C99 introduced several features used in this project:
#   - stdint.h (uint8_t, uint16_t, uint32_t, int16_t)
#   - declarations mixed with statements inside blocks
#   - //  single-line comments
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable all warnings and treat them as errors so bad code is caught early.
# -Wall  : all common warnings
# -Wextra: additional warnings (unused parameters, missing return values, etc.)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

# Link the math library (required for sqrt() in stats.c on Linux/macOS).
# On Windows with MSVC the math library is linked automatically.
if (UNIX)
    set(MATH_LIB m)
else()
    set(MATH_LIB "")
endif()

# Define the executable target and list every .c source file.
# All four source files must be included so the linker can resolve
# function references across translation units.
add_executable(adc_sensor_processor
    main.c
    adc.c
    io.c
    stats.c
)

# Link the math library if needed (UNIX only).
target_link_libraries(adc_sensor_processor ${MATH_LIB})

# Include the project root so that #include "adc.h" etc. resolve correctly.
target_include_directories(adc_sensor_processor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
