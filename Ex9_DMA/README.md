# NHÓM 3 - HỆ THỐNG NHÚNG - BÁO CÁO BÀI TẬP TUẦN 4

## A. Nội dung công việc 

Hiểu cơ chế DMA và giải phóng cpu khỏi các tác vụ truyền dữ 
- Cấu hình DMA để truyền dữ liệu ADC vào bộ nhớ RAM
- Khi quá trình truyền hoàn tất, một ngắt được kích hoạt để xử lý tín hiệu
- Hiển thị dữ liệu đã được truyền thành công qua terminal

#### Demo:https://drive.google.com/file/d/1J_W-UuVczr_eYJK2PCy3BxoWNYEuTQLu/view

## B. Báo cáo chi tiết

## 1️ Ý tưởng chính

- **Đọc dữ liệu ADC từ một kênh (PA0 / ADC0) liên tục** mà không block CPU.
- **Sử dụng DMA** để tự động chuyển dữ liệu ADC vào một buffer trong RAM.
- Khi buffer đầy, **DMA kích hoạt ngắt** → tính giá trị trung bình của các mẫu ADC.
- **Gửi kết quả trung bình qua UART** tới máy tính.
- Sau đó, DMA được **khởi động lại** để tiếp tục thu thập dữ liệu.

## 2️ Khởi tạo UART – `USART1_Init()`

```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
````

* Bật clock cho GPIOA và USART1 trước khi cấu hình.

```c
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

* PA9 → **TX**, Alternate Function Push-Pull → UART điều khiển chân truyền dữ liệu.

```c
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

* PA10 → **RX**, Input Floating → nhận dữ liệu từ bên ngoài.

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
* Thiết lập baud rate, data bits, stop bit, parity và bật UART.


## 3️ Hàm gửi dữ liệu qua UART

#### `USART1_SendChar()`

```c
while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
USART_SendData(USART1, c);
```

* Đợi cho TX buffer trống → gửi 1 ký tự.

#### `USART1_SendString()`

```c
while (*s) {
    USART1_SendChar(*s++);
}
```

* Gửi từng ký tự trong chuỗi cho đến khi gặp `'\0'`.

## 4️ Cấu hình ADC với DMA – `ADC1_DMA_Config()`

### 4.1 Khởi tạo GPIO

```c
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOA, &GPIO_InitStructure);
```

* PA0 ở chế độ **Analog Input**, tắt digital buffer để giảm nhiễu.

### 4.2 Khởi tạo DMA

```c
DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcBuffer;
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
DMA_InitStructure.DMA_Priority = DMA_Priority_High;
DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
DMA_Init(DMA1_Channel1, &DMA_InitStructure);
DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
```

* **DMA** tự động lấy dữ liệu từ thanh ghi ADC → ghi vào buffer.
* `PeripheralInc = Disable`: thanh ghi ADC cố định.
* `MemoryInc = Enable`: buffer RAM tăng dần.
* `Mode = Normal`: DMA dừng sau khi đầy buffer.
* `IT_TC`: bật ngắt khi hoàn tất.

### 4.3 Cấu hình ngắt DMA

```c
NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);
```

* Cho phép **DMA Channel1 interrupt** → xử lý dữ liệu khi buffer đầy.

### 4.4 Cấu hình ADC

```c
ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
ADC_InitStructure.ADC_ScanConvMode = DISABLE;
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
ADC_InitStructure.ADC_NbrOfChannel = 1;
ADC_Init(ADC1, &ADC_InitStructure);
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
ADC_DMACmd(ADC1, ENABLE);
ADC_Cmd(ADC1, ENABLE);
ADC_ResetCalibration(ADC1);
while (ADC_GetResetCalibrationStatus(ADC1));
ADC_StartCalibration(ADC1);
while (ADC_GetCalibrationStatus(ADC1));
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
```

* ADC liên tục chuyển đổi (Continuous Mode) và **gửi dữ liệu qua DMA**.
* Calibration vẫn được thực hiện để đảm bảo độ chính xác.

## 5️ Xử lý ngắt DMA – `DMA1_Channel1_IRQHandler()`

```c
if (DMA_GetITStatus(DMA1_IT_TC1)) {
    DMA_ClearITPendingBit(DMA1_IT_TC1);
    DMA_Cmd(DMA1_Channel1, DISABLE);
    dmaDoneFlag = 1;
}
```

* Khi DMA hoàn tất buffer:

  * Xóa cờ ngắt.
  * Tạm dừng DMA.
  * Đặt **cờ `dmaDoneFlag = 1`** → main loop xử lý dữ liệu.
    
## 6️ Vòng lặp chính – `main()`

```c
if (dmaDoneFlag) {
    dmaDoneFlag = 0;
    uint32_t sum = 0;
    for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
        sum += adcBuffer[i];
    }
    uint16_t avg = sum / ADC_BUFFER_SIZE;
    uint32_t voltage_mV = ((uint32_t)avg * 3300UL) / 4095UL;
    sprintf(buffer, "DMA done | AVG ADC=%u | Voltage=%lu mV\r\n", (unsigned)avg, (unsigned long)voltage_mV);
    USART1_SendString(buffer);
    DMA_SetCurrDataCounter(DMA1_Channel1, ADC_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
```

* Khi **DMA hoàn tất**:

  * Tính **trung bình ADC** từ buffer.
  * Quy đổi ra **mV**.
  * Gửi kết quả qua UART.
  * Reset DMA để **tiếp tục thu thập dữ liệu mới**.

