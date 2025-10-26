# NHÓM 3 – HỆ THỐNG NHÚNG  
## Nhiệm vụ
Viết chương trình có 2 Task cùng in dữ liệu qua UART có sử dụng MUTEX để chia sẻ tài nguyên

##### Code: 

##### Video demo: https://drive.google.com/file/d/1Vb9OGc5mnz75YTcHtl5c24iqKBA7C95E/view

## Phân tích hoạt động
#### 1️ Khởi tạo UART – UART1_Init()
```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
```

- Bật clock cho GPIOA và USART1 — đúng và cần thiết.

GPIO:

- PA9 (TX): GPIO_Mode_AF_PP, tốc độ 50MHz — đúng cho TX.

- PA10 (RX): GPIO_Mode_IN_FLOATING — được cấu hình, mặc dù USART_Mode chỉ bật Tx; không gây lỗi nhưng nếu cần nhận phải bật Rx.

USART:

- 9600, 8N1, no flow control — bình thường.

- USART_InitStructure.USART_Mode = USART_Mode_Tx; — chỉ bật truyền. Nếu muốn nhận dữ liệu hoặc dùng IRQ RX cần bật Rx.

- USART_Cmd(USART1, ENABLE); — bật USART sau cấu hình.

#### 2️ Hàm gửi chuỗi – UART1_SendString()
```c
while (*str)
{
    USART_SendData(USART1, *str++);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
```

- Gửi từng ký tự rồi chờ cờ TXE trước khi tiếp tục → hoạt động đúng.

- Hàm blocking (busy-wait) trên CPU/task cho tới khi cờ TXE được set. Trong hệ dùng mutex, busy-wait kéo dài có thể làm task chiếm mutex lâu, làm tăng độ trễ của task khác. Với chuỗi ngắn và tốc độ baud nhỏ, chặn vài chục đến vài trăm µs mỗi ký tự thường chấp nhận được; với chuỗi dài hoặc baud thấp, cân nhắc giải pháp non-blocking (queue/stream-buffer + ISR).

#### 3️ Mutex – uartMutex
uartMutex = xSemaphoreCreateMutex();


- Dùng mutex (binary semaphore kiểu mutex) để bảo vệ truy cập chung tới UART — đúng hướng để tránh việc các chuỗi bị xen kẽ.

- Tuy nhiên chương trình không kiểm tra uartMutex != NULL — nên kiểm tra để phát hiện lỗi cấp phát (heap không đủ).

#### 4️ Task1 & Task2

- Cả hai task lấy mutex với portMAX_DELAY — chờ vô hạn nếu mutex đang bận.

- Task1 gửi chuỗi rồi vTaskDelay(1000 ms); Task2 delay 1500 ms. Cả hai có cùng priority = 1.

- Vì dùng mutex, các chuỗi sẽ được gửi nguyên vẹn, không bị interleaving.

#### 5️ main() và scheduler

- SystemInit(); UART1_Init(); uartMutex = xSemaphoreCreateMutex(); xTaskCreate(...); vTaskStartScheduler();

- Nếu uartMutex == NULL hoặc xTaskCreate thất bại, chương trình nên xử lý (ví dụ vòng lặp vô hạn, báo lỗi qua LED, v.v).

- Sau vTaskStartScheduler() control do scheduler quản lý; while(1); là dự phòng
