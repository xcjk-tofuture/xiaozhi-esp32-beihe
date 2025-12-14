#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ��Ƶ����������
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 16000

// SPM1423 PDM ������˷�����?
#define AUDIO_MIC_I2S_GPIO_CLK   GPIO_NUM_1  // PDM CLK
#define AUDIO_MIC_I2S_GPIO_DATA  GPIO_NUM_2  // PDM DATA

// MAX98357A I2S ��������
#define AUDIO_SPKR_I2S_GPIO_BCLK  GPIO_NUM_18  // BCLK
#define AUDIO_SPKR_I2S_GPIO_LRCLK GPIO_NUM_8   // LRCLK (WS)
#define AUDIO_SPKR_I2S_GPIO_DATA  GPIO_NUM_17  // DIN
#define AUDIO_SPKR_ENABLE         GPIO_NUM_16  // SD_MODE (Enable)

// �Ƿ�ʹ�òο����루����������
#define AUDIO_INPUT_REFERENCE false

// ��ť����
#define BOOT_BUTTON_GPIO GPIO_NUM_0

// ILI9486 LCD ��ʾ������ (SPI) - ʹ���Զ�������
#define DISPLAY_SPI_SCK_PIN  GPIO_NUM_10  // SPI CLK
#define DISPLAY_SPI_MOSI_PIN GPIO_NUM_9   // SPI MOSI
#define DISPLAY_SPI_MISO_PIN GPIO_NUM_46  // SPI MISO
#define DISPLAY_DC_PIN       GPIO_NUM_12  // DC
#define DISPLAY_CS_PIN       GPIO_NUM_11  // CS
#define DISPLAY_RST_PIN      GPIO_NUM_13  // RST

// ILI9486 ��ʾ����
#define DISPLAY_WIDTH   480
#define DISPLAY_HEIGHT  320
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY  false

#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0

// ������ƣ�����ж����������ţ�
#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_3  // ����б������ţ����ö�Ӧ��? GPIO
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

// LED ���ã���ѡ��
#define BUILTIN_LED_GPIO GPIO_NUM_NC

#endif // _BOARD_CONFIG_H_
