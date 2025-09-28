# NHÓM 3 - HỆ THỐNG NHÚNG - BÁO CÁO BÀI TẬP TUẦN 4

## A. Nội dung công việc

Đọc và chuyển đổi tín hiệu analog sang số
#### Demo
- Phần I:https://drive.google.com/file/d/19HwSKFK7r0cVCuKWObuxcm1OHEPfmuL_/view
- Phần II:https://drive.google.com/file/d/1Lg5vnwO2wJkpHVWyYhMBrzkeEk6K_54W/view
## B. Báo cáo chi tiết

## I. Cấu hình 1 kênh ADC để đọc giá trị từ 1 biến trở 
## Các bước thực thi 
### 1️ Khởi tạo USART1 – `USART1_Init()`

```c
// Bat clock cho USART1 va GPIOA
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
```
- Bắt buộc phải bật clock trước khi cấu hình ngoại vi, nếu không các lệnh sau sẽ **không hoạt động**.


```c
// Cau hinh chan PA9 lam TX (Output, AF Push-Pull)
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

- PA9 cấu hình **Alternate Function Push-Pull** → cho phép ngoại vi (UART) điều khiển chân này để **truyền dữ liệu ra ngoài ở tốc độ cao (50MHz)**.


```c
// Cau hinh chan PA10 lam RX (Input Floating)
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

- PA10 ở **Input Floating** → không kéo lên/kéo xuống → UART có thể nhận tín hiệu từ bên ngoài.

```c
// Cau hinh tham so truyen UART
USART_InitStructure.USART_BaudRate = 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
USART_InitStructure.USART_Parity = USART_Parity_No;
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
```

| Thông số          | Ý nghĩa              | Lý do chọn          |
| ----------------- | -------------------- | ------------------- |
| BaudRate = 115200 | Tốc độ truyền        | Chuẩn PC hay dùng   |
| 8-bit data        | Dữ liệu mỗi lần gửi  | Chuẩn phổ biến      |
| 1 Stop Bit        | Kết thúc dữ liệu     | Tiết kiệm thời gian |
| No Parity         | Không kiểm tra lỗi   | Đơn giản hơn        |
| Tx + Rx           | Cho phép gửi và nhận | Đầy đủ chức năng    |

```c
USART_Init(USART1, &USART_InitStructure);
USART_Cmd(USART1, ENABLE);
```

- Ghi cấu hình vào thanh ghi USART — **bắt đầu hoạt động UART**.

### 2️ Hàm gửi dữ liệu UART

#### `USART1_SendChar()`

```c
while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
USART_SendData(USART1, c);
```

- Đợi cho tới khi **TXE = 1** (buffer rỗng) → sau đó gửi ký tự → tránh ghi đè.

#### `USART1_SendString()`

```c
while (*s) {
    USART1_SendChar(*s++);
}
```

- Gửi từng ký tự trong chuỗi cho đến khi gặp **ký tự '\0' (null terminator)**.

### 3️ Khởi tạo ADC – `ADC1_Init_Config()`

```c
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

- PA0 ở chế độ **Analog Input**, tắt Schmitt-Trigger và Digital buffer để giảm nhiễu.

```c
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
```

- **Liên tục đo mà không cần gọi lệnh Start mỗi lần** → giá trị luôn sẵn sàng.

```c
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
```

| Tham số         | Ý nghĩa                              |
| --------------- | ------------------------------------ |
| `ADC_Channel_0` | Đọc chân PA0                         |
| `Rank 1`        | Vị trí ưu tiên trong chuỗi đo        |
| `55.5 cycles`   | Thời gian lấy mẫu (chậm → chính xác) |

```c
ADC_ResetCalibration(ADC1);
while (ADC_GetResetCalibrationStatus(ADC1));
ADC_StartCalibration(ADC1);
while (ADC_GetCalibrationStatus(ADC1));
```

- Calibration giúp **giảm sai số ADC**, nên luôn chạy **một lần sau reset**.

```c
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
```

- Bắt đầu quá trình đo **vô hạn** (do Continuous Mode).

### 4️ Đọc ADC

```c
return ADC_GetConversionValue(ADC1);
```

- Trả về giá trị **0 → 4095 (12-bit)** tương ứng với **0V → 3.3V**.

### 5️ Vòng lặp chính – `main()`

```c
voltage_mV = ((uint32_t)adcValue * 3300UL) / 4095UL;
```

- Quy đổi sang **mV** 

```c
sprintf(buffer, "ADC=%u | Voltage=%lu mV\r\n", ...);
USART1_SendString(buffer);
```

- Format chuỗi rồi **in ra máy tính qua UART** → có thể xem bằng **Serial Monitor (115200 baud)**.

```c
for (volatile int i = 0; i < 50000; i++);
```



- **Delay thủ công** → tránh spam UART quá nhanh.

Demo:https://drive.google.com/file/d/19HwSKFK7r0cVCuKWObuxcm1OHEPfmuL_/view

## II. Chuyển giá trị đã đọc thành (mV) gửi lên terminal
### 1. Khởi tạo USART1 – USART1_Init()
```c

RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

```

Bật clock cho ngoại vi USART1 và GPIOA trước khi cấu hình.

Nếu không bật clock, các lệnh cấu hình GPIO hay USART sẽ không hoạt động.
```c

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_Init(GPIOA, &GPIO_InitStructure);

```

PA9 cấu hình Alternate Function Push-Pull → cho phép USART1 điều khiển chân để truyền dữ liệu ra ngoài.

Tốc độ 50MHz giúp truyền dữ liệu nhanh.
```c

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);

```

PA10 cấu hình Input Floating → nhận tín hiệu UART từ bên ngoài, không kéo lên hoặc kéo xuống.
```c

USART_InitStructure.USART_BaudRate = 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
USART_InitStructure.USART_Parity = USART_Parity_No;
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

USART_Init(USART1, &USART_InitStructure);
USART_Cmd(USART1, ENABLE);
```

Cấu hình tốc độ truyền (115200 baud), 8 bit dữ liệu, 1 stop bit, không parity, không flow control.

Cho phép gửi và nhận dữ liệu qua UART.

Lệnh USART_Cmd(USART1, ENABLE) bật USART1 để hoạt động.

### 2. Hàm gửi dữ liệu UART
- USART1_SendChar()
```c

while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
USART_SendData(USART1, c);
```

Đợi Transmit Data Register Empty (TXE = 1) → buffer rỗng.

Gửi 1 ký tự c qua USART.

- USART1_SendString()
```c

while (*s) {
    USART1_SendChar(*s++);
}

```

Lặp gửi từng ký tự trong chuỗi đến khi gặp null terminator ('\0').

### 3. Khởi tạo ADC – ADC1_Init_Config()
```c

RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

```

Bật clock cho GPIOA và ADC1.
```c

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```
PA0 cấu hình Analog Input → tắt Digital Buffer/Schmitt-Trigger để đo tín hiệu analog.
```c

ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode = DISABLE;
ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
ADC_InitStructure.ADC_NbrOfChannel = 1;
ADC_Init(ADC1, &ADC_InitStructure);

```

ADC độc lập, không scan nhiều kênh.

Single Conversion Mode (không liên tục) → phải gọi Start Conversion mỗi lần đọc.

DataAlign Right → kết quả 12-bit nằm ở bit thấp của thanh ghi.

Chỉ đọc 1 kênh (PA0).
```c


ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
ADC_Cmd(ADC1, ENABLE);
```


Chọn kênh 0, Rank 1 trong chuỗi conversion, thời gian lấy mẫu 55.5 cycles (tăng độ chính xác).

Bật ADC.

Hiệu chuẩn ADC
```c

ADC_ResetCalibration(ADC1);
while (ADC_GetResetCalibrationStatus(ADC1));
ADC_StartCalibration(ADC1);
while (ADC_GetCalibrationStatus(ADC1));
```

Reset và calibration để giảm sai số đo, luôn thực hiện sau khi bật ADC.

### 4. Đọc giá trị ADC – ADC1_Read()
```c

ADC_SoftwareStartConvCmd(ADC1, ENABLE);
while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
return ADC_GetConversionValue(ADC1);
```


Khởi động conversion bằng phần mềm mỗi lần đọc.

Chờ cờ EOC để đảm bảo dữ liệu sẵn sàng.

Trả về giá trị 12-bit: 0 → 4095 (tương ứng 0V → 3.3V).

### 5. Vòng lặp chính – main()
```c

adcValue = ADC1_Read();
voltage_mV = ((uint32_t)adcValue * 3300UL) / 4095UL;
```

Đọc giá trị ADC.

Chuyển đổi sang mV dựa trên Vref = 3.3V.
```c

sprintf(buffer, "ADC=%u | Voltage=%lu mV\r\n", (unsigned)adcValue, (unsigned long)voltage_mV);
USART1_SendString(buffer);

```

Format chuỗi và gửi qua UART.

Có thể hiển thị trên Serial Monitor.
```c

for (volatile int i = 0; i < 50000; i++);
```


Delay thủ công để không spam UART quá nhanh.

F. Các điểm giống với code Continuous ADC (Phần I)

- Cấu hình USART1 và GPIO giống nhau

- Cấu hình ADC cơ bản (kênh, Rank, SampleTime, calibration) giống.

- Format chuỗi và gửi UART giống.

- Chuyển giá trị ADC sang mV và delay bằng vòng lặp.

G. Các điểm khác biệt so với Continuous Mode (Phần I)

- ADC_ContinuousConvMode = DISABLE → Single Conversion Mode.

- ADC_SoftwareStartConvCmd() gọi mỗi lần đọc trong hàm ADC1_Read().

- Phải chờ cờ EOC mỗi lần đọc để đảm bảo dữ liệu hợp lệ.

- CPU bị block trong khi chờ đọc → không tự động cập nhật như Continuous Mode.
  
Demo:https://drive.google.com/file/d/1Lg5vnwO2wJkpHVWyYhMBrzkeEk6K_54W/view
