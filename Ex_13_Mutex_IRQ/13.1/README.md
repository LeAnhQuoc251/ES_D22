# NHÓM 3 – HỆ THỐNG NHÚNG  
## Nhiệm vụ
Viết chương trình có 2 task: 1 Task Blink LED, 1 Task xử lý khi có ngắt ngoài thì bật đèn hoặc còi cảnh báo. Yêu cầu xử lý Task bật đèn ngay sau khi hàm xử lý ngắt thực thi. Khi chưa nhấn nút Task bật đèn/ còi ở chế độ Block

## Phân tích hoạt động

#### 1. Khai báo

SemaphoreHandle_t semAlert; — biến semaphore nhị phân dùng để thông báo giữa ISR và task.

#### 2. Cấu hình GPIO (GPIO_Config)

Bật clock cho GPIOA, GPIOC và AFIO.

PC13 cấu hình GPIO_Mode_Out_PP dùng làm LED chính (nháy).

PA6 cấu hình tương tự làm LED thông báo.

PA0 cấu hình GPIO_Mode_IPU (input pull-up) làm nút nhấn — khi nhấn sẽ kéo xuống mức 0.

#### 3. Cấu hình ngắt ngoài (EXTI_Config)

Map chân PA0 sang EXTI_Line0 bằng GPIO_EXTILineConfig.

Khởi tạo EXTI_Line0 với:

Mode = Interrupt

Trigger = Falling (kích khi nút nhấn kéo xuống 0)

LineCmd = ENABLE

Cấu hình NVIC cho EXTI0_IRQn:

Gọi NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4) (thiết lập cách phân chia preemption/subpriority).

Gán NVIC_IRQChannelPreemptionPriority = 6, SubPriority = 0, và bật IRQ.

#### 4. Task 1 — vTaskMainLED

Task chạy vô hạn:

Xor PC13 để đảo trạng thái LED (nháy).

vTaskDelay(pdMS_TO_TICKS(400)) — delay 400 ms giữa các lần nháy.

Priority: tạo ở main với priority 1.

#### 5. Task 2 — vTaskNotifyLED

Task chờ semaphore: xSemaphoreTake(semAlert, portMAX_DELAY) — block vô hạn cho đến khi semaphore được cho.

Khi nhận semaphore:

Bật PA6 (LED báo).

vTaskDelay(pdMS_TO_TICKS(2000)) — giữ sáng 2s.

Tắt PA6.

Priority: tạo với priority 2 (cao hơn task nháy LED).

#### 6. ISR (EXTI0_IRQHandler)

Kiểm tra flag EXTI_GetITStatus(EXTI_Line0).

Tạo biến BaseType_t xHigherPriorityTaskWoken = pdFALSE;

Gọi xSemaphoreGiveFromISR(semAlert, &xHigherPriorityTaskWoken); — giải phóng semaphore từ ISR để unblock task đợi.

Gọi portYIELD_FROM_ISR(xHigherPriorityTaskWoken); — nếu task được unblock có priority cao hơn, request context switch ngay sau ISR.

Clear pending bit EXTI_ClearITPendingBit(EXTI_Line0).

#### 7. main

Gọi GPIO_Config() và EXTI_Config().

Tạo semaphore nhị phân: semAlert = xSemaphoreCreateBinary();

Tạo hai task (vTaskMainLED priority 1, vTaskNotifyLED priority 2).

vTaskStartScheduler() — bắt đầu scheduler.

while(1); ở cuối (không bao giờ đến được nếu scheduler chạy thành công).
