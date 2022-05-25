#include <stdio.h>
#include "pico/stdlib.h"
#include "common/pimoroni_common.hpp"
#include "hardware/uart.h"

#include "pms5003.hpp"

using namespace pimoroni;

PMS5003 pms5003(uart1, 8, 9, 2, 3);

int main() {
  stdio_init_all();

  while(true){
    bool result = pms5003.read();
    if(result){
      printf("%d\n", pms5003.data.pm_0_3_1l);

    }
    sleep_ms(100);
  };

  return 0;
}
