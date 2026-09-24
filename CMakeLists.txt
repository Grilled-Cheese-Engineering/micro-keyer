# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD seeed_xiao_rp2350 CACHE STRING "Board type")
#option(KEYBOARD_KEYER "Enable keyboard" ON)
# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
project(micro-keyer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(micro-keyer main.cpp usb_descriptors.cpp ssd1306.c display.cpp)

pico_set_program_name(micro-keyer "micro-keyer")
pico_set_program_version(micro-keyer "0.1")

# Generate PIO header

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(micro-keyer 1)
pico_enable_stdio_usb(micro-keyer 0)

# Add the standard library to the build
target_link_libraries(micro-keyer
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(micro-keyer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(micro-keyer 
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_pwm
        hardware_flash
        pico_unique_id
        tinyusb_board
        tinyusb_device
        )



pico_add_extra_outputs(micro-keyer)