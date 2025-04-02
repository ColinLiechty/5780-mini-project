#include <assert.h>
#include "main.h"
#include "hal_usart.h"

void control_LED(char led_sel);
void led_repl_iteration(void);

#define ACTIVITY_NUM 1
#define BUF_SIZE     8

char buf[BUF_SIZE];
uint8_t head_idx;
uint8_t tail_idx;

void inc_tail_idx(void)
{
    ++tail_idx;
    if (tail_idx == BUF_SIZE) {
        tail_idx = 0;
    }
}

int lab4_main(void) {
    HAL_Init();
    SystemClock_Config();

    // Enable clocks
    HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_RCC_USART3_CLK_ENABLE();

    // USART3
#if (ACTIVITY_NUM == 2)
    configure_TTL_RXint(USART3, HAL_RCC_GetHCLKFreq()/115200);
#else
    configure_TTL(USART3, HAL_RCC_GetHCLKFreq()/115200);
#endif

    // TX Pin
    GPIO_InitTypeDef init_pb10 = {GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_LOW, GPIO_NOPULL, 4};
    HAL_GPIO_Init(GPIOB, &init_pb10);

    // RX Pin
    GPIO_InitTypeDef init_pb11 = {GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_LOW, GPIO_NOPULL, 4};
    HAL_GPIO_Init(GPIOB, &init_pb11);

    GPIO_InitTypeDef init_pc = {GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
                                GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_LOW, GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &init_pc);
    // Orange LED
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    // Green LED
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
    // Red LED
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    // Blue LED
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

#if (ACTIVITY_NUM == 0)
    // Push button
    GPIO_InitTypeDef init_pa0 = {GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_SPEED_FREQ_LOW, GPIO_PULLDOWN};
    HAL_GPIO_Init(GPIOA, &init_pa0);
    uint32_t debouncer = 0;
    char* data = "Remember to collect the screenshot for the post lab.\r\n";
#elif (ACTIVITY_NUM == 1)
    char led_select = 0;
#elif (ACTIVITY_NUM == 2)
    // Set up NVIC
    NVIC_EnableIRQ(USART3_4_IRQn);
    NVIC_SetPriority(USART3_4_IRQn, 1);

    head_idx = 0;
    tail_idx = 0;
#endif

    while (1) {
#if (ACTIVITY_NUM == 0)
        HAL_Delay(10);
        
        debouncer = (debouncer << 1);

        if (HAL_GPIO_ReadPin(GPIOA, 0) == 1) {
            debouncer |= 0x00000001;
        }
        if (debouncer == 0xFFFFFFFF) {
            USART_send_string(USART3, data);
        }
#elif (ACTIVITY_NUM == 1)
        led_select = USART_recv_byte(USART3);
        control_LED(led_select);
#elif (ACTIVITY_NUM == 2)
        led_repl_iteration();
#endif
    } 

    return 1;
}

void control_LED(char led_sel)
{
    char* error_msg = "ERROR: Invalid character.\r\n";
    switch (led_sel) {
        case 'r':
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
            break;
        case 'b':
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
            break;
        case 'g':
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
            break;
        case 'o':
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
            break;
        default:
            // Send error
            USART_send_string(USART3, error_msg);
            break;
    }
}

void led_repl_iteration(void)
{
    // Single iteration of a repl within the while(1)
    char* prompt = "CMD?\r\n";
    char* error_msg = "ERROR: Invalid character.\r\n";
    char recv_cmd[4] = {'x', 'x', '\n', '\0'};
    USART_send_string(USART3, prompt);
    // Wait for color
    while (head_idx == tail_idx);
    inc_tail_idx();
    recv_cmd[0] = buf[tail_idx];
    uint32_t pin_num = 0;
    switch (recv_cmd[0]) {
        case 'r':
            pin_num = GPIO_PIN_6;
            break;
        case 'b':
            pin_num = GPIO_PIN_7;
            break;
        case 'g':
            pin_num = GPIO_PIN_9;
            break;
        case 'o':
            pin_num = GPIO_PIN_8;
            break;
        default:
            // Send error
            USART_send_string(USART3, error_msg);
            return;
    }
    // Wait for op
    while (head_idx == tail_idx);
    inc_tail_idx();
    recv_cmd[1] = buf[tail_idx];
    switch (recv_cmd[1]) {
        case '0':
            HAL_GPIO_WritePin(GPIOC, pin_num, GPIO_PIN_RESET);
            break;
        case '1':
            HAL_GPIO_WritePin(GPIOC, pin_num, GPIO_PIN_SET);
            break;
        case '2':
            HAL_GPIO_TogglePin(GPIOC, pin_num);
            break;
        default:
            USART_send_string(USART3, error_msg);
            return;
    }
    // Print cmd
    USART_send_string(USART3, recv_cmd);
}

void USART3_4_IRQHandler(void)
{
    ++head_idx;
    if (head_idx == BUF_SIZE) {
        head_idx = 0;
    }
    buf[head_idx] = (char) USART3->RDR;
}
