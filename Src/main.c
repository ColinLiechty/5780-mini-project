#include "main.h"
#include <stm32f0xx_hal.h>

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  HAL_RCC_GPIOC_CLK_ENABLE();
  
  while (1) {
    // Main loop
  }

  return 1;
}
