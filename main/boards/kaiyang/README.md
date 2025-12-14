# KAIYANG 开发板

## 硬件规格

### 主控芯片
- **ESP32-S3** (双核 Xtensa LX7, 240MHz)

### 音频系统
- **麦克风**: SPM1423 PDM 数字 MEMS 麦克风
- **功放**: MAX98357A I2S 数字功放
- **采样率**: 16kHz (输入/输出)

### 显示屏
- **LCD**: ILI9486 (320x480)
- **接口**: SPI
- **颜色**: 16位 RGB565

### 网络
- **Wi-Fi**: 2.4GHz 802.11 b/g/n

## 引脚定义

### 音频引脚
| 功能 | GPIO | 说明 |
|------|------|------|
| 麦克风 CLK | GPIO39 | SPM1423 PDM 时钟 |
| 麦克风 DATA | GPIO38 | SPM1423 PDM 数据 |
| 扬声器 BCLK | GPIO18 | MAX98357A 位时钟 |
| 扬声器 LRCLK | GPIO8 | MAX98357A 左右时钟 |
| 扬声器 DIN | GPIO17 | MAX98357A 数据输入 |
| 扬声器 EN | GPIO16 | MAX98357A 使能控制 |

### 显示引脚
| 功能 | GPIO | 说明 |
|------|------|------|
| SPI SCK | GPIO10 | SPI 时钟 |
| SPI MOSI | GPIO9 | SPI 数据输出 |
| SPI MISO | GPIO46 | SPI 数据输入 |
| LCD DC | GPIO12 | 数据/命令选择 |
| LCD CS | GPIO11 | 片选 |
| LCD RST | GPIO13 | 复位 |
| LCD BL | - | 背光控制（可选） |

### 其他引脚
| 功能 | GPIO | 说明 |
|------|------|------|
| BOOT 按钮 | GPIO0 | 启动按钮 |
| LED | GPIO2 | 内置 LED |

## 硬件连接

### SPM1423 麦克风连接
```
SPM1423          ESP32-S3
--------         --------
VDD      ----    3.3V
GND      ----    GND
CLK      ----    GPIO39
DATA     ----    GPIO38
L/R      ----    GND (左声道) 或 VDD (右声道)
```

### MAX98357A 功放连接
```
MAX98357A        ESP32-S3
---------        --------
VIN      ----    5V (或 3.3V)
GND      ----    GND
BCLK     ----    GPIO18
LRCLK    ----    GPIO8
DIN      ----    GPIO17
SD       ----    GPIO16
GAIN     ----    GND (9dB) 或 VDD (15dB) 或 悬空 (12dB)
```

### ILI9486 LCD 连接
```
ILI9486          ESP32-S3
-------          --------
VCC      ----    3.3V
GND      ----    GND
SCK      ----    GPIO10
MOSI     ----    GPIO9
MISO     ----    GPIO46
DC       ----    GPIO12
CS       ----    GPIO11
RST      ----    GPIO13
BL       ----    (可选)
```

## 编译和烧录

### 方法一：使用 release.py 脚本（推荐）

```bash
python scripts/release.py kaiyang
```

### 方法二：手动编译

1. 设置目标芯片：
```bash
idf.py set-target esp32s3
```

2. 配置项目：
```bash
idf.py menuconfig
```
在菜单中选择：`Xiaozhi Assistant` -> `Board Type` -> `KAIYANG Board`

3. 编译：
```bash
idf.py build
```

4. 烧录：
```bash
idf.py flash monitor
```

## 功能特性

- ? PDM 数字麦克风输入（SPM1423）
- ? I2S 数字功放输出（MAX98357A）
- ? 320x480 彩色 LCD 显示
- ? Wi-Fi 连接
- ? 离线语音唤醒
- ? 实时语音交互
- ? OTA 固件升级
- ? 多语言支持

## 注意事项

1. **音频质量**：
   - SPM1423 是数字麦克风，音质优于模拟麦克风
   - MAX98357A 支持最高 384kHz 采样率，本配置使用 16kHz
   - 可以通过调整 `AUDIO_SPKR_ENABLE` 引脚控制功放开关

2. **显示屏**：
   - ILI9486 支持 320x480 分辨率
   - 使用 SPI 接口，速度适中
   - 背光可通过 PWM 调节亮度

3. **电源**：
   - MAX98357A 建议使用 5V 供电以获得更大功率
   - 如果使用 3.3V，输出功率会降低

4. **引脚冲突**：
   - 确保引脚配置与实际硬件一致
   - 避免与 Flash/PSRAM 引脚冲突

## 故障排除

### 麦克风无声音
- 检查 SPM1423 的 L/R 引脚是否正确连接
- 确认 PDM 时钟和数据引脚连接正确
- 查看串口日志确认麦克风是否初始化成功

### 扬声器无声音
- 检查 MAX98357A 的 SD 引脚是否拉高
- 确认 I2S 引脚连接正确
- 检查扬声器是否正确连接到 MAX98357A 输出

### 显示屏不显示
- 检查 SPI 引脚连接
- 确认背光引脚是否正常工作
- 查看串口日志确认显示屏初始化状态

## 技术支持

如有问题，请在项目 GitHub 提交 Issue：
https://github.com/78/xiaozhi-esp32/issues

QQ 群：1011329060
