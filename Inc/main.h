#pragma once
#include <stm32f0xx_hal.h>
#include <stdint.h>

void SystemClock_Config(void);

int lab1_main(void);
int lab2_main(void);
int lab3_main(void);
int lab4_main(void);
int lab5_main(void);
int lab6_main(void);

void HAL_RCC_GPIOC_CLK_ENABLE(void);
void HAL_RCC_GPIOA_CLK_ENABLE(void);
void HAL_RCC_GPIOB_CLK_ENABLE(void);
void HAL_RCC_SYSCFG_CLK_ENABLE(void);
void HAL_RCC_TIM2_CLK_ENABLE(void);
void HAL_RCC_TIM3_CLK_ENABLE(void);
void HAL_RCC_USART3_CLK_ENABLE(void);
void HAL_RCC_I2C2_CLK_ENABLE(void);
void HAL_RCC_ADC1_CLK_ENABLE(void);

void HAL_GPIO_AFSelect(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t AFnum);

void HAL_config_EXTI(uint32_t line_num, uint32_t control);

void configure_ADC_lab6(ADC_TypeDef* ADCx);
