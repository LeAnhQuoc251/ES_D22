#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f10x_exti.h"

// ==================== KHAI BAO SEMAPHORE ====================
SemaphoreHandle_t semAlert;

// ==================== CAU HINH GPIO ====================
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Cap xung clock cho GPIOA, GPIOC va AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    // LED chinh: PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // LED thong bao: PA6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Nut nhan: PA0 (keo len noi)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// ==================== CAU HINH NGAT NGOAI ====================
void EXTI_Config(void)
{
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  // nhan nut xuong muc 0
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// ==================== TASK 1: LED CHINH ====================
void vTaskMainLED(void *pvParameters)
{
    while (1)
    {
        GPIOC->ODR ^= GPIO_Pin_13; // dao trang thai LED
        vTaskDelay(pdMS_TO_TICKS(400)); // thay doi chu ky nhay
    }
}

// ==================== TASK 2: LED THONG BAO ====================
void vTaskNotifyLED(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(semAlert, portMAX_DELAY) == pdTRUE)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_6);
            vTaskDelay(pdMS_TO_TICKS(2000)); // giam thoi gian sang
            GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        }
    }
}

// ==================== HAM NGAT NGOAI ====================
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xSemaphoreGiveFromISR(semAlert, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

// ==================== HAM MAIN ====================
int main(void)
{
    GPIO_Config();
    EXTI_Config();

    semAlert = xSemaphoreCreateBinary();

    xTaskCreate(vTaskMainLED, "MainLED", 128, NULL, 1, NULL);
    xTaskCreate(vTaskNotifyLED, "NotifyLED", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1);
}
