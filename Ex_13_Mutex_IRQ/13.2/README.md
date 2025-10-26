# NHÓM 3 – HỆ THỐNG NHÚNG  
## Nhiệm vụ
Viết chương trình có 2 Task cùng in dữ liệu qua UART, không sử dụng Mutex. Hiển thị được sự xung đột

##### Code: https:https://github.com/LeAnhQuoc251/ES_D22/blob/master/Ex_13_Mutex_IRQ/13.2/User/main.c

##### Video demo: https://drive.google.com/file/d/1oMQrgssujZu0xrbzo_VFEOVD8jzisEBl/view

## Phân tích hoạt động
#### 1️ Khởi tạo UART – UART_Config()
```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
```

- Bật clock cho GPIOA và USART1 → cần thiết (USART1 nằm trên APB2).

GPIO:

- PA9 (TX): GPIO_Mode_AF_PP, speed 50MHz → đúng (alternate function push-pull).

- PA10 (RX): GPIO_Mode_IN_FLOATING → đúng nếu muốn RX; trong mã hiện tại chỉ cấu hình TX mode cho USART, nhưng PA10 vẫn được cấu hình sẵn.

USART:

- Cấu hình 9600, 8N1, không flow control.

- USART_Mode = USART_Mode_Tx; → chỉ bật TX (không bật RX). Nếu muốn nhận phải bật Rx.

- USART_Cmd(USART1, ENABLE); → bật peripheral sau cấu hình.

#### 2️ Hàm gửi chuỗi – UART_SendString()
```c
while (*str)
{
    USART_SendData(USART1, *str++);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
````

- Gửi từng ký tự, chờ TXE trước khi gửi ký tự tiếp theo → đảm bảo dữ liệu đưa vào DR khi TX buffer trống.

- Hàm này blocking: trong khi chờ TXE nó busy-wait (thực ra chờ cờ, nhưng vì gọi trong task, không gọi taskYIELD(); nhưng vì cờ sẽ set nhanh so với vTaskDelay, blocking ở mức micro/mili giây là chấp nhận được).

#### 3️ Các Task

- vTaskA và vTaskB:

- Gửi chuỗi mỗi 500 ms.

- Được tạo với cùng priority = 1 → nếu preemption bật và tick cho phép time-slice, hai task sẽ chia CPU; nhưng vì gửi blocking trên UART, scheduling có thể khiến xen kẽ chuỗi (see above).

- Stack size 128: khá nhỏ nhưng đủ cho hàm đơn giản; vẫn nên kiểm tra stack overflow runtime.

#### 4️ main() và scheduler

- Gọi SystemInit(); UART_Config(); tạo 2 task rồi vTaskStartScheduler();.

- Không kiểm tra giá trị trả về của xTaskCreate → nên kiểm tra để phát hiện lỗi tạo task (heap thiếu, v.v).
