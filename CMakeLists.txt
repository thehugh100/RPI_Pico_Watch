add_executable(smartwatch
        font_manager.cpp
        font_manager.h
        smartwatch.cpp
        SSH1106_SPI_Lite.h
        SSH1106_SPI_Lite.cpp
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(smartwatch pico_stdlib hardware_spi hardware_rtc hardware_dma hardware_pio hardware_timer hardware_clocks)

pico_enable_stdio_usb(smartwatch 1)
pico_enable_stdio_uart(smartwatch 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(smartwatch)

# add url via pico_set_program_url
example_auto_set_url(smartwatch)
