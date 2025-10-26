# NHÓM 3 – HỆ THỐNG NHÚNG  
## Nhiệm vụ
Viết chương trình có 2 task: 1 Task Blink LED, 1 Task xử lý khi có ngắt ngoài thì bật đèn hoặc còi cảnh báo. Yêu cầu xử lý Task bật đèn ngay sau khi hàm xử lý ngắt thực thi. Khi chưa nhấn nút Task bật đèn/ còi ở chế độ Block

##### Code: https://github.com/LeAnhQuoc251/ES_D22/blob/master/Ex_13_Mutex_IRQ/13.1/User/main.c

##### Video demo: https://drive.google.com/file/d/13GJW9g9hvRyYVsuRDxuUbez6cEr9TVVI/view

## Phân tích hoạt động

#### 1️ Khởi tạo clock & GPIO – GPIO_Config()
```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
```

- Bật clock cho GPIOA, GPIOC và AFIO trước khi cấu hình ngoại vi.

GPIOs:

- PC13 (LED chính): cấu hình GPIO_Mode_Out_PP, tốc độ 2MHz. Dùng để nháy LED chính.
- PA6 (LED thông báo): dùng cùng GPIO_InitStructure (Out_PP), là LED báo khi có sự kiện.

- PA0 (nút nhấn): cấu hình GPIO_Mode_IPU (Input Pull-Up) — nghĩa là chân thường kéo lên VCC nội và khi nhấn sẽ kéo xuống 0 (mạch nút nhấn nối mass khi nhấn).

#### 2️ Cấu hình ngắt ngoài (EXTI) – EXTI_Config()
```c
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
```

- Map PA0 → EXTI_Line0.

EXTI cấu hình:

- EXTI_Line = EXTI_Line0

- EXTI_Mode = EXTI_Mode_Interrupt

- EXTI_Trigger = EXTI_Trigger_Falling → ngắt khi chân chuyển từ 1 → 0 (nhấn nút).

- NVIC: enable EXTI0_IRQn, PreemptionPriority = 6, SubPriority = 0.

- NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); — thiết lập grouping (4 => 4 bits preemption, 0 subpriority trên Cortex-M3).


#### 3️ Semaphore – khai báo và ý nghĩa
```c
SemaphoreHandle_t semAlert;
semAlert = xSemaphoreCreateBinary();
```

- Dùng binary semaphore để đồng bộ giữa ISR và task. Ban đầu semaphore rỗng (0) → vTaskNotifyLED sẽ block chờ tới khi ISR give.

- Sử dụng xSemaphoreTake(semAlert, portMAX_DELAY) trong task để chờ vĩnh viễn (blocking, không tiêu tốn CPU).


#### 4️ Task 1 — vTaskMainLED (LED chính)
```c
xTaskCreate(vTaskMainLED, "MainLED", 128, NULL, 1, NULL);
```

- Mục đích: đảo trạng thái PC13 mỗi 400 ms (vTaskDelay(pdMS_TO_TICKS(400))).

- Priority = 1 (thấp hơn task NotifyLED).

- Sử dụng vTaskDelay để yield và cho phép các task khác chạy (tiết kiệm CPU).


#### 5️ Task 2 — vTaskNotifyLED (LED thông báo)
```c
xTaskCreate(vTaskNotifyLED, "NotifyLED", 128, NULL, 2, NULL);
```

- Chờ semaphore xSemaphoreTake(semAlert, portMAX_DELAY) → khi nhận, bật PA6, vTaskDelay(2000 ms), sau đó tắt.

- Priority = 2 cao hơn MainLED, vì cần phản hồi nhanh khi có sự kiện.

#### 6️ ISR EXTI0 — EXTI0_IRQHandler()
```c
if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(semAlert, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    EXTI_ClearITPendingBit(EXTI_Line0);
}
```


- Kiểm tra cờ EXTI, dùng xSemaphoreGiveFromISR để give semaphore từ ISR (đúng API cho ISR context).

- Dùng portYIELD_FROM_ISR để request context switch nếu semaphore unblocked một task có priority cao hơn current — phù hợp và tối ưu.

- Xóa cờ ngắt sau khi thao tác.

#### 7️ main() và scheduler

- Gọi GPIO_Config(); EXTI_Config(); rồi tạo semaphore, tạo 2 task, start scheduler.

- Sau vTaskStartScheduler(), control thuộc về FreeRTOS; while(1); là dự phòng nếu scheduler trả về (không xảy ra trong điều kiện bình thường).


