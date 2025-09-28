#include "stm32f10x.h"
#include <stdio.h>

// Khoi tao USART1 de gui du lieu ra may tinh (PA9 = TX, PA10 = RX)
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Bat clock cho USART1 va GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA9 lam chan TX (truyen du lieu ra)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // Alternate Function Push-Pull
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10 lam chan RX (nhan du lieu vao)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh thong so UART
    USART_InitStructure.USART_BaudRate = 115200;                    // Toc do baud
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // 8 bit du lieu
    USART_InitStructure.USART_StopBits = USART_StopBits_1;          // 1 stop bit
    USART_InitStructure.USART_Parity = USART_Parity_No;             // Khong parity
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // Cho phep TX + RX

    USART_Init(USART1, &USART_InitStructure);  // Ap dung cau hinh
    USART_Cmd(USART1, ENABLE);                 // Bat USART1
}

// Gui 1 ky tu qua UART
void USART1_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Cho TX san sang
    USART_SendData(USART1, c);
}

// Gui chuoi ky tu null-terminated (\0) qua UART
void USART1_SendString(char *s) {
    while (*s) {
        USART1_SendChar(*s++);
    }
}

// Khoi tao ADC1 doc dien ap tai chan PA0 (ADC Channel 0)
void ADC1_Init_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // Bat clock cho ADC1 va GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // PA0 lam Analog Input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh ADC o che do doc lien tuc
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;              // Chi doc 1 kenh
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;         // Doc lien tuc
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Khong kich tu outside
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;     // Can phai
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Chon kenh ADC0, rank 1, thoi gian lay mau 55.5 chu ky
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    ADC_Cmd(ADC1, ENABLE);  // Bat ADC

    // Calibration (bat buoc tren STM32F1)
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // Bat qua trinh chuyen doi
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// Doc gia tri ADC (tra ve so 0..4095)
uint16_t ADC1_Read(void) {
    return ADC_GetConversionValue(ADC1);
}



int main(void) {
    uint16_t adcValue;      // Gia tri ADC doc duoc
    uint32_t voltage_mV;    // Dien ap tinh ra mV
    char buffer[64];        // Chuoi in UART

    USART1_Init();          // Khoi dong UART
    ADC1_Init_Config();     // Khoi dong ADC

    while (1) {
        adcValue = ADC1_Read();                           // Doc ADC
        voltage_mV = ((uint32_t)adcValue * 3300UL) / 4095UL; // Tinh ra mV (Vref = 3.3V)

        sprintf(buffer, "ADC=%u | Voltage=%lu mV\r\n", adcValue, voltage_mV);
        USART1_SendString(buffer);                       // Gui ra UART

        for (volatile int i = 0; i < 50000; i++);        // Delay don gian
    }
}
