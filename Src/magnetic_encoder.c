#include <stm32f0xx_hal.h>
#include <assert.h>
#include <stdio.h>
#include <hal_usart.h>


uint16_t I2C2_Read_ISR(uint32_t bit);
void I2C_Write(uint8_t data);
uint8_t I2C_Read();
void setup_USART(void);
void printR(char* comment, u_int32_t reg);
void printD(char* comment, int32_t numb);
void My_HAL_GPIO_AF(GPIO_TypeDef  *GPIOx, uint16_t pin, uint16_t mode);


int magnetic_encoder_main(void)
{
    HAL_Init();
    SystemClock_Config();

    HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_RCC_I2C2_CLK_ENABLE();
    HAL_RCC_USART3_CLK_ENABLE();

    //initialize the GPIO pins for the LEDs
    GPIO_InitTypeDef initStrLED = {GPIO_PIN_6 | GPIO_PIN_7 |GPIO_PIN_8 | GPIO_PIN_9,
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};

    HAL_GPIO_Init(GPIOC, &initStrLED);

    //initalize I2C SCL and SDA
    GPIO_InitTypeDef initStrSDA = {GPIO_PIN_11, GPIO_MODE_AF_OD};
    HAL_GPIO_Init(GPIOB, &initStrSDA);

    GPIO_InitTypeDef initStrSCL = {GPIO_PIN_13, GPIO_MODE_AF_OD};
    HAL_GPIO_Init(GPIOB, &initStrSCL);

    GPIO_InitTypeDef initStr1 = {GPIO_PIN_14, GPIO_MODE_OUTPUT_PP};
    HAL_GPIO_Init(GPIOB, &initStr1);

    GPIO_InitTypeDef initStr2 = {GPIO_PIN_0, GPIO_MODE_OUTPUT_PP};
    HAL_GPIO_Init(GPIOC, &initStr2);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);

   My_HAL_GPIO_AF(GPIOB, 11, 1);
   My_HAL_GPIO_AF(GPIOB, 13, 5);

    //setup USART for debugging
    setup_USART();

    USART_send_string(USART3,"START");


    I2C2->TIMINGR = I2C2->TIMINGR & ((~I2C_TIMINGR_PRESC_Msk) | (1 << I2C_TIMINGR_PRESC_Pos));
    I2C2->TIMINGR = I2C2->TIMINGR & ((~I2C_TIMINGR_SCLL_Msk) | (0x13 << I2C_TIMINGR_SCLL_Pos));
    I2C2->TIMINGR = I2C2->TIMINGR & ((~I2C_TIMINGR_SCLH_Msk) | (0xF << I2C_TIMINGR_SCLH_Pos));
    I2C2->TIMINGR = I2C2->TIMINGR & ((~I2C_TIMINGR_SDADEL_Msk) | (0x2 << I2C_TIMINGR_SDADEL_Pos));
    I2C2->TIMINGR = I2C2->TIMINGR & ((~I2C_TIMINGR_SCLDEL_Msk) | (0x4 << I2C_TIMINGR_SCLDEL_Pos));

    I2C2->CR1 |= 0x1;

    I2C2->CR2 &= ~(0x3FF); //clear SADD
    I2C2->CR2 &= ~(0xFF << 16); //clear NBYTES

    I2C2->CR2 |= (0x36 << 1); //set slave address
    I2C2->CR2 |= (1 << 16); //set NBYTES 
    I2C2->CR2 &= ~(1 << 10); //set RD_WRN = 0

    printR("CR2", I2C2->CR2); //print cr2 to verify setup

    USART_send_byte(USART3, '\n');
    USART_send_byte(USART3, '\r');

    I2C2->CR2 |= (1 << 13); //set start

    printR("ISR", I2C2->ISR); //print isr 

    USART_send_byte(USART3, '\n');
    USART_send_byte(USART3, '\r');

    USART_send_string(USART3,"checkpoint 1");

    while ((I2C2->ISR & I2C_ISR_TXIS) == 0) {
        if (I2C2->ISR & I2C_ISR_NACKF) {
            USART_send_string(USART3, "NACK on TXIS\n");
            return;
        }
    }
    

    USART_send_string(USART3,"checkpoint 2");

    I2C_Write(0x0B);

    printR("ISR", I2C2->ISR); //print isr 

    USART_send_byte(USART3, '\n');
    USART_send_byte(USART3, '\r');

    // //If NACK end program
    // if (I2C2_Read_ISR(1) == 0)
    // {
    //     USART_send_string(USART3, "ERROR NACK 1");
    //     return;
    // }

    
    while((I2C2->ISR & (1 << 6)) == 0);

    I2C2->CR2 &= ~(0x3FF); //clear SADD
    I2C2->CR2 &= ~(0xFF << 16); //clear NBYTES

    I2C2->CR2 |= (0x36 << 1); //set slave address
    I2C2->CR2 |= (1 << 16); //set NBYTES 
    I2C2->CR2 |= (1 << 10); //set RD_WRN = 1

    if (I2C2_Read_ISR(2) == 0)
    {
        USART_send_string(USART3, "ERROR NACK 1");
        
        return;
    }

    //while((I2C2->ISR & (1 << 6)) == 0);

    printR("status", I2C2->RXDR);


}

//Function to help read the ISR register
uint16_t I2C2_Read_ISR(uint32_t bit)
{

    while(((I2C2->ISR & (1 << 4)) == 0) && ((I2C2->ISR & (1 << bit)) == 0)); 

    printR("read ISR", I2C2->ISR); //print isr
    if (I2C2->ISR & (1 << 4))
        return 0;

    return 1;
}

//fucntion to help wrtire data through the TXDR
void I2C_Write(uint8_t data)
{
    //transmit_String("write");

    I2C2->TXDR = data;

    //printR("write data", I2C2->TXDR);
}

//functino to help read the RXDR
uint8_t I2C_Read()
{
    //transmit_String("read");
    //printR("read data", I2C2->RXDR);

    return I2C2->RXDR;
}

//set up USART for debugging
void setup_USART(void)
{

    //set up GPIO pins for USART
    //pin 10 = Tx pin 11 = Rx
    GPIO_InitTypeDef initStrTXRX = {GPIO_PIN_10 | GPIO_PIN_11, 
        GPIO_MODE_AF_PP,  
        GPIO_SPEED_FREQ_LOW, 
        GPIO_NOPULL,
        GPIO_AF1_USART3};


    HAL_GPIO_Init(GPIOC, &initStrTXRX);

    configure_TTL(USART3, HAL_RCC_GetHCLKFreq()/115200);
    

}

//helper function to print what is in registers to help debugging
void printR(char* comment, uint32_t reg)
{
    char r[50];
    sprintf(r, "%s: 0x%08X", comment, reg);
    USART_send_string(USART3, r);
}

//helper function to print actaul decimal numbers to help debugging
void printD(char* comment, int32_t numb)
{
    char str[20];
    snprintf(str, sizeof(str), "%d", numb);  // Convert hex to a decimal string
    USART_send_string(USART3, ("%s %s", comment, str));
}

void My_HAL_GPIO_AF(GPIO_TypeDef  *GPIOx, uint16_t pin, uint16_t mode)
{
    //make sure teh mode is 0-7
    assert(mode >= 0 && mode <= 7);

    //make sure the pin is in the range of 0-15
    assert(pin >= 0 && pin <= 15);

    //make sure the GPIO passed
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));

    if (pin <= 7)
    {
        GPIOx->AFR[0] &= ~(0xF << (pin * 4));  // Clear bits
        GPIOx->AFR[0] |= (mode << (pin * 4));  //set bits
    }
    else
    {
        pin -= 8;
        GPIOx->AFR[0] &= ~(0xF << (pin * 4));  
        GPIOx->AFR[1] |= (mode << (pin * 4));
    }
}