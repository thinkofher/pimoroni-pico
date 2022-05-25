add_library(pimoroni_uart INTERFACE)

target_sources(pimoroni_uart INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pimoroni_uart.cpp
)

target_include_directories(pimoroni_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Pull in pico libraries that we need
target_link_libraries(pimoroni_uart INTERFACE pico_stdlib hardware_uart hardware_mutex)
