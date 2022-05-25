#pragma once
#include <stdint.h>
#include <climits>
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "hardware/regs/uart.h"
#include "pico/mutex.h"
#include "hardware/gpio.h"
#include "ringbuf.hpp"

namespace pimoroni {
    class UART {
      private:
        uart_inst_t *uart;
        uint pin_tx;
        uint pin_rx;
        uint baudrate;
        RingBuffer read_buffer(256);
        RingBuffer write_buffer(256);

      public:
        UART(uart_inst_t *uart, uint pin_tx, uint pin_rx, uint baudrate=9600)
        : uart(uart), pin_tx(pin_tx), pin_rx(pin_rx), baudrate(baudrate) {
            uart_init(uart, baudrate);
            gpio_init(pin_tx);gpio_set_function(pin_tx, GPIO_FUNC_UART);
            gpio_init(pin_rx);gpio_set_function(pin_rx, GPIO_FUNC_UART);
        }
    
        ~UART() {
            uart_deinit(uart);
            gpio_set_function(pin_tx, GPIO_FUNC_NULL);
            gpio_set_function(pin_rx, GPIO_FUNC_NULL);
        }

        void uart_drain_rx_fifo() {
            while(uart_is_readable(uart) && read_buffer.free() > 0) {
                read_buffer.put(uart_get_hw(uart)->dr);
            }
        }

        int available() {
            uart_drain_rx_fifo();
            return read_buffer.avail();
        }
    };
}