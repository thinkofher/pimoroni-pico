#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <cstring>

constexpr char PMS5003_SOF[] = "\x42\x4d";
constexpr uint16_t PMS5003_SOFU = 0x424d;
constexpr char PMS5003_CMD_MODE_PASSIVE[] = "\xe1\x00\x00";
constexpr char PMS5003_CMD_MODE_ACTIVE[] = "\xe1\x00\x01";
constexpr char PMS5003_CMD_READ[] = "\xe2\x00\x00";
constexpr char PMS5003_CMD_SLEEP[] = "\xe4\x00\x00";
constexpr char PMS5003_CMD_WAKEUP[] = "\xe4\x00\x01";

constexpr uint PMS5003_MAX_RESET_TIME = 20000;
constexpr uint PMS5003_MAX_RESP_TIME = 5000;
constexpr uint PMS5003_MIN_CMD_INTERVAL = 100;


class PMS5003 {
    public:
        enum PMS5003Mode {
            PMS5003ModePassive,
            PMS5003ModeActive
        };

#pragma pack(push, 1)
        struct alignas(1) cmd_response_data {
            uint8_t a;
            uint8_t b;
            uint16_t crc;
        };

        struct alignas(1) cmd_data {
            uint16_t sof;
            uint8_t cmd[3];
            uint16_t crc;  
        };

        struct alignas(1) response_data {
            uint16_t pm_1_0;
            uint16_t pm_2_5;
            uint16_t pm_10;
            uint16_t pm_1_0_ao;
            uint16_t pm_2_5_ao;
            uint16_t pm_uhhh;
            uint16_t pm_0_3_1l;
            uint16_t pm_0_5_1l;
            uint16_t pm_1_0_1l;
            uint16_t pm_2_5_1l;
            uint16_t pm_5_1l;
            uint16_t pm_10_1l;
            uint16_t _; // ????
            uint16_t crc;
        };
#pragma pack(pop)

        response_data data;

        PMS5003(uart_inst_t *uart, uint pin_tx, uint pin_rx,
                uint pin_reset, uint pin_enable,
                PMS5003Mode mode=PMS5003ModeActive, uint retries=5)
                : uart(uart),
                pin_tx(pin_tx),
                pin_rx(pin_rx),
                pin_reset(pin_reset),
                pin_enable(pin_enable) {
                    uart_init(uart, 9600);
                    gpio_init(pin_tx);gpio_set_function(pin_tx, GPIO_FUNC_UART);
                    gpio_init(pin_rx);gpio_set_function(pin_rx, GPIO_FUNC_UART);
                    gpio_init(pin_reset);gpio_set_function(pin_reset, GPIO_FUNC_SIO);gpio_set_dir(pin_reset, true);gpio_put(pin_reset, false);
                    gpio_init(pin_enable);gpio_set_function(pin_enable, GPIO_FUNC_SIO);gpio_set_dir(pin_enable, true);gpio_put(pin_enable, true);

                    reset();
                };
        ~PMS5003() {};

        void cmd_mode_passive() {
            sleep_ms(PMS5003_MIN_CMD_INTERVAL);
            reset_input_buffer();
        };
        void cmd_mode_active() {
            reset_input_buffer();
        }
        void reset() {
            sleep_ms(100);
            gpio_put(pin_reset, true);
            reset_input_buffer();
            sleep_ms(100);
            gpio_put(pin_reset, false);

            // Reset will return to active mode
            if (mode == PMS5003ModePassive) {
                cmd_mode_passive();
            }
        };
        void data_available();
        bool read() {
            return read_data();
        };

    private:
        uart_inst_t *uart;
        uint pin_tx;
        uint pin_rx;
        uint pin_reset;
        uint pin_enable;
        PMS5003Mode mode;

        uint8_t buf[64];

        bool process_response(uint8_t *data, size_t length, void *result) {
            uint16_t data_crc = (data[length - 2] << 8) | data[length - 1];
            uint16_t calc_crc = calculate_checksum(data, length - 2);
    
            // Copy the data into the result struct
            memcpy(result, data, length - 2);

            return data_crc == calc_crc;
        };

        static uint16_t calculate_checksum(uint8_t *data, size_t length) {
            uint16_t crc = PMS5003_SOF[0] + PMS5003_SOF[1];
            for(auto i = 0u; i < length; i++) {
                crc += data[i];
            }
            return crc;
        };

        static void build_cmd_frame(uint8_t *command, uint8_t *result) {
            cmd_data *cmd = (cmd_data *)result;
            cmd->sof = PMS5003_SOFU;
            cmd->crc = calculate_checksum(command, 3);
            memcpy(cmd->cmd, command, sizeof(cmd->cmd));
        }
        void reset_input_buffer() {
            while(uart_is_readable_within_us(uart, 100)) {
                uart_getc(uart);
            }
        };
        bool read_data() {
            uint sof_index = 0;
            
            while (true) {
                wait_for_bytes(1);
                if (buf[0] == PMS5003_SOF[sof_index]) {
                    if(sof_index == 0) {
                        sof_index = 1;
                        continue;
                    } else if (sof_index == 1) {
                        break;
                    }
                } else {
                    sof_index = 0;
                }
            }

            size_t buf_length = wait_for_bytes(2);
            uint16_t *buf16 = (uint16_t *)buf;
            uint frame_length = buf16[0];
            if (frame_length == 32) {
                buf_length = wait_for_bytes(frame_length);

                return process_response(buf, buf_length, (void *)&data);
            }
    
            return false;
        }
        size_t wait_for_bytes(uint num_bytes, uint timeout=PMS5003_MAX_RESP_TIME) {
            uint i = 0;
            while(uart_is_readable_within_us(uart, 100) && i < num_bytes && i < 256) {
                buf[i++] = uart_getc(uart);
            }
            return i;
        };
        void cmd_passive_read();
};