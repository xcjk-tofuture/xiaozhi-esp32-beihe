# HEIHE ��������������˵��

## ��Ҫ��ʾ
?? **��������ʵ��Ӳ�������޸� `config.h` �е����Ŷ��壡**

## ��ǰ���õ�����

### ��Ƶϵͳ

#### SPM1423 PDM ��˷�
- **CLK**: GPIO4
- **DATA**: GPIO5

#### MAX98357A I2S ����
- **BCLK**: GPIO6
- **LRCLK**: GPIO7
- **DIN**: GPIO8
- **SD (Enable)**: GPIO9

### ��ʾϵͳ

#### ILI9486 LCD (SPI)
- **SCK**: GPIO18
- **MOSI**: GPIO23
- **MISO**: GPIO19
- **DC**: GPIO21
- **CS**: GPIO22
- **RST**: δʹ�� (GPIO_NUM_NC)
- **BL (����)**: GPIO27

### ����

#### ��ť
- **BOOT**: GPIO0

#### LED
- **���� LED**: GPIO2

## ����޸���������

1. �� `main/boards/beihe/config.h`
2. �ҵ���Ӧ�ĺ궨��
3. �޸�Ϊ���ʵ�����ź�
4. ���±���̼�

���磬��������˷� CLK ���ӵ� GPIO10��
```c
#define AUDIO_MIC_I2S_GPIO_CLK   GPIO_NUM_10  // ԭ���� GPIO_NUM_4
```

## ���ų�ͻ���

��ȷ����������û�б�ռ�ã�
- GPIO6-11: ͨ���� Flash ռ�ã����ʹ�� PSRAM��
- GPIO26-32: ESP32-S3 û����Щ����
- Strapping ����: GPIO0, GPIO3, GPIO45, GPIO46

## �Ƽ����ŷ���

�������Ҫ���·������ţ����飺
- **I2S/PDM**: ʹ�� GPIO4-GPIO21 ��Χ�ڵ�����
- **SPI**: ʹ�� GPIO18, GPIO19, GPIO23 (��׼ SPI ����)
- **��ť**: GPIO0 (BOOT ��ť)
- **LED**: GPIO2 ���������� GPIO
