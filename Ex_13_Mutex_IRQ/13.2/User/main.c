#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_usart.h"

// ==================== CAU HINH UART ====================
void UART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 -> TX (output push-pull)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10 -> RX (input floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cau hinh UART1: 9600 8N1
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

// ==================== HAM GUI CHUOI QUA UART ====================
void UART_SendString(char *str)
{
    while (*str)
    {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}

// ==================== TASK 1: GUI CHUOI A ====================
void vTaskA(void *pvParameters)
{
    while (1)
    {
        UART_SendString(">>> Task A dang gui du lieu...\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ==================== TASK 2: GUI CHUOI B ====================
void vTaskB(void *pvParameters)
{
    while (1)
    {
        UART_SendString("<<< Task B cung gui du lieu!!!\r\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ==================== HAM MAIN ====================
int main(void)
{
    SystemInit();
    UART_Config();

    xTaskCreate(vTaskA, "Task_A", 128, NULL, 1, NULL);
    xTaskCreate(vTaskB, "Task_B", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}
