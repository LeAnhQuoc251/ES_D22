#include "stm32f10x.h"
#include <stdio.h>

// Ham khoi tao USART1 (TX: PA9, RX: PA10)
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Bat clock cho USART1 va GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // Cau hinh chan PA9 lam TX (Output, AF Push-Pull)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh chan PA10 lam RX (Input Floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh tham so truyen UART
    USART_InitStructure.USART_BaudRate = 115200;                        // Toc do baud
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;         // 8 bit du lieu
    USART_InitStructure.USART_StopBits = USART_StopBits_1;              // 1 stop bit
    USART_InitStructure.USART_Parity = USART_Parity_No;                 // Khong su dung parity
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // Khong flow control
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;     // Cho phep truyen va nhan

    // Goi ham khoi tao va bat USART1
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

// Gui 1 ky tu qua USART1
void USART1_SendChar(char c) {
    // Cho den khi thanh ghi TXE (Transmit Data Register Empty) san sang
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

// Gui chuoi ky tu (null terminated string) qua USART1
void USART1_SendString(char *s) {
    while (*s) {
        USART1_SendChar(*s++);
    }
}


// Cau hinh ADC1 doc kenh 0 (PA0)
void ADC1_Init_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // Bat clock cho GPIOA va ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // Cau hinh PA0 lam analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  // Che do doc doc lap
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       // Khong scan nhieu kenh
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  // Chuyen doi lien tuc
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Khong dung trigger ngoai
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              // Can phai
    ADC_InitStructure.ADC_NbrOfChannel = 1;                             // Chi doc 1 kenh
    ADC_Init(ADC1, &ADC_InitStructure);

    // Chon kenh ADC0, rank 1, thoi gian lay mau 55.5 chu ky
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    // Bat ADC1
    ADC_Cmd(ADC1, ENABLE);

    // Reset va calibration ADC de dam bao do chinh xac
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // Bat qua trinh chuyen doi bang phan mem (vi che do continuous)
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// Doc gia tri ADC (12 bit: 0 .. 4095)
uint16_t ADC1_Read(void) {
    return ADC_GetConversionValue(ADC1);
}

int main(void) {
    uint16_t adcValue;        // Gia tri ADC doc duoc
    uint32_t voltage_mV;      // Dien ap tinh bang mV
    char buffer[64];          // Bo dem chuoi in ra UART

    USART1_Init();            // Khoi tao UART
    ADC1_Init_Config();       // Khoi tao ADC

    while (1) {
        adcValue = ADC1_Read();  // Doc gia tri ADC

        // Chuyen doi sang mV voi Vref = 3.3V
        voltage_mV = ((uint32_t)adcValue * 3300UL) / 4095UL;

        // In ket qua qua UART
        sprintf(buffer, "ADC=%u | Voltage=%lu mV\r\n",
                (unsigned)adcValue, (unsigned long)voltage_mV);
        USART1_SendString(buffer);

        // Tao delay nho bang vong lap (tam thoi)
        for (volatile int i = 0; i < 50000; i++);
    }
}
