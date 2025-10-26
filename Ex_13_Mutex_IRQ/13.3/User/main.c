#include "stm32f10x_usart.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t uartMutex;

void UART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;      
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void UART1_SendString(char *str)
{
    while (*str)
    {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}

void Task1(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(uartMutex, portMAX_DELAY) == pdTRUE)
        {
            UART1_SendString("Task 1 dang gui du lieu...\r\n");
            xSemaphoreGive(uartMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task2(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(uartMutex, portMAX_DELAY) == pdTRUE)
        {
            UART1_SendString("Task 2 gui du lieu khac...\r\n");
            xSemaphoreGive(uartMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

int main(void)
{
    SystemInit();
    UART1_Init();

    uartMutex = xSemaphoreCreateMutex();

    xTaskCreate(Task1, "Task1", 128, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}
