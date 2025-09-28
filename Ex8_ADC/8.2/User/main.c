#include "stm32f10x.h"
#include <stdio.h>

// Khoi tao USART1 de truyen nhan du lieu qua cong Serial
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Bat clock cho USART1 va GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA9 cau hinh lam TX (AF Push Pull, 50MHz)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10 cau hinh lam RX (Input Floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh USART1: 115200 baud, 8 bit, 1 stop, khong parity
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE); // Bat USART1
}

// Gui 1 ky tu qua USART1
void USART1_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Cho TX san sang
    USART_SendData(USART1, c); // Gui du lieu
}

// Gui 1 chuoi ky tu qua USART1
void USART1_SendString(char *s) {
    while (*s) {
        USART1_SendChar(*s++); // Gui tung ky tu
    }
}

// Khoi tao ADC1 doc gia tri tu PA0 (kenh ADC0)
void ADC1_Init_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // Bat clock cho GPIOA va ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // PA0 cau hinh lam analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;              // ADC doc lap
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                   // Khong scan nhieu kenh
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;             // Doc 1 lan moi lan goi
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Kich bang phan mem
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;          // Can phai
    ADC_InitStructure.ADC_NbrOfChannel = 1;                         // 1 kenh
    ADC_Init(ADC1, &ADC_InitStructure);

    // Chon kenh 0 (PA0), rank 1, thoi gian lay mau 55.5 chu ky
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    ADC_Cmd(ADC1, ENABLE); // Bat ADC1

    // Hieu chuan ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

// Doc 1 gia tri ADC (single conversion)
uint16_t ADC1_Read(void) {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // Bat dau chuyen doi
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); // Cho hoan thanh
    return ADC_GetConversionValue(ADC1); // Tra ve gia tri ADC
}

int main(void) {
    uint16_t adcValue;      // Gia tri ADC (0–4095)
    uint32_t voltage_mV;    // Dien ap tinh bang mV
    char buffer[64];        // Chuoi gui qua UART

    USART1_Init();          // Khoi tao USART1
    ADC1_Init_Config();     // Khoi tao ADC1

    while (1) {
        adcValue = ADC1_Read();  // Doc gia tri ADC tu PA0
        voltage_mV = ((uint32_t)adcValue * 3300UL) / 4095UL; // Quy doi sang mV

        // Gui chuoi ket qua qua UART
        sprintf(buffer, "ADC=%u | Voltage=%lu mV\r\n", 
                (unsigned)adcValue, (unsigned long)voltage_mV);
        USART1_SendString(buffer);

        // Tao delay de khong gui qua nhanh
        for (volatile int i = 0; i < 50000; i++);
    }
}
