if (TARGET tinyusb_device)
    add_executable(displayDriver main.cpp)
    pico_generate_pio_header(displayDriver ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR})

    # pull in common dependencies
    target_link_libraries(displayDriver pico_stdlib hardware_dma hardware_pio hardware_spi)

    pico_enable_stdio_usb(displayDriver 1)
    pico_enable_stdio_uart(displayDriver 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(displayDriver)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building displayDriver because TinyUSB submodule is not initialized in the SDK")
endif()