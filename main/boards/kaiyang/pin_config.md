# KAIYANG 开发板引脚配置说明

## 重要提示
?? **请根据你的实际硬件连接修改 `config.h` 中的引脚定义！**

## 当前配置的引脚

### 音频系统

#### SPM1423 PDM 麦克风
- **CLK**: GPIO4
- **DATA**: GPIO5

#### MAX98357A I2S 功放
- **BCLK**: GPIO6
- **LRCLK**: GPIO7
- **DIN**: GPIO8
- **SD (Enable)**: GPIO9

### 显示系统

#### ILI9486 LCD (SPI)
- **SCK**: GPIO18
- **MOSI**: GPIO23
- **MISO**: GPIO19
- **DC**: GPIO21
- **CS**: GPIO22
- **RST**: 未使用 (GPIO_NUM_NC)
- **BL (背光)**: GPIO27

### 控制

#### 按钮
- **BOOT**: GPIO0

#### LED
- **内置 LED**: GPIO2

## 如何修改引脚配置

1. 打开 `main/boards/kaiyang/config.h`
2. 找到对应的宏定义
3. 修改为你的实际引脚号
4. 重新编译固件

例如，如果你的麦克风 CLK 连接到 GPIO10：
```c
#define AUDIO_MIC_I2S_GPIO_CLK   GPIO_NUM_10  // 原来是 GPIO_NUM_4
```

## 引脚冲突检查

请确保以下引脚没有被占用：
- GPIO6-11: 通常被 Flash 占用（如果使用 PSRAM）
- GPIO26-32: ESP32-S3 没有这些引脚
- Strapping 引脚: GPIO0, GPIO3, GPIO45, GPIO46

## 推荐引脚分配

如果你需要重新分配引脚，建议：
- **I2S/PDM**: 使用 GPIO4-GPIO21 范围内的引脚
- **SPI**: 使用 GPIO18, GPIO19, GPIO23 (标准 SPI 引脚)
- **按钮**: GPIO0 (BOOT 按钮)
- **LED**: GPIO2 或其他空闲 GPIO
