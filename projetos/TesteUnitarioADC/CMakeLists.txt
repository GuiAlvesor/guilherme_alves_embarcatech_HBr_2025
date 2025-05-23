# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TesteUnitarioADC C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(TesteUnitarioADC 
app/main.c
drivers/adc_to_celsius.c
Unity/unity.c )

pico_set_program_name(TesteUnitarioADC "TesteUnitario")
pico_set_program_version(TesteUnitarioADC "0.1")

set(UNITY_DIR${CMAKE_SOURCE_DIR}/Unity)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(TesteUnitarioADC 0)
pico_enable_stdio_usb(TesteUnitarioADC 1)

# Add the standard library to the build
target_link_libraries(TesteUnitarioADC
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(TesteUnitarioADC PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        Unity
        drivers
)

# Add any user requested libraries
target_link_libraries(TesteUnitarioADC
        hardware_adc
        )

pico_add_extra_outputs(TesteUnitarioADC)

set(UNITY_DIR ${CMAKE_SOURCE_DIR}/Unity)

# Adiciona um executável para os testes unitários
add_executable(test_adc_to_celsius
    test/test_adc_to_celsius.c
    drivers/adc_to_celsius.c
    Unity/unity.c
)

pico_enable_stdio_usb(test_adc_to_celsius 1)
pico_enable_stdio_uart(test_adc_to_celsius 0)


# Inclui headers do projeto e da Unity
target_include_directories(test_adc_to_celsius PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Unity
    ${UNITY_DIR}/drivers
)

# Liga as bibliotecas necessárias
target_link_libraries(test_adc_to_celsius
    pico_stdlib
    hardware_adc
)

pico_add_extra_outputs(test_adc_to_celsius)

