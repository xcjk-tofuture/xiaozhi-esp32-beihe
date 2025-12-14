#ifndef _KAIYANG_AUDIO_CODEC_H
#define _KAIYANG_AUDIO_CODEC_H

#include "audio_codec.h"

#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>

/**
 * @brief KAIYANG 音频编解码器
 * 
 * 输入：SPM1423 PDM 数字麦克风
 * 输出：MAX98357A I2S 功放
 */
class KaiyangAudioCodec : public AudioCodec {
private:
    const audio_codec_data_if_t *data_if_ = nullptr;
    const audio_codec_ctrl_if_t *out_ctrl_if_ = nullptr;
    const audio_codec_if_t *out_codec_if_ = nullptr;
    const audio_codec_ctrl_if_t *in_ctrl_if_ = nullptr;
    const audio_codec_if_t *in_codec_if_ = nullptr;
    const audio_codec_gpio_if_t *gpio_if_ = nullptr;
    gpio_num_t spkr_enable_pin_ = GPIO_NUM_NC;
    
    uint32_t volume_ = 90;

    void CreatePdmMicrophone(gpio_num_t clk, gpio_num_t data);
    void CreateI2sSpeaker(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t data);

    virtual int Read(int16_t* dest, int samples) override;
    virtual int Write(const int16_t* data, int samples) override;

public:
    KaiyangAudioCodec(int input_sample_rate, int output_sample_rate,
        gpio_num_t mic_clk, gpio_num_t mic_data,
        gpio_num_t spkr_bclk, gpio_num_t spkr_lrclk, gpio_num_t spkr_data,
        gpio_num_t spkr_enable);
    virtual ~KaiyangAudioCodec();

    virtual void SetOutputVolume(int volume) override;
    virtual void EnableInput(bool enable) override;
    virtual void EnableOutput(bool enable) override;
};

#endif // _KAIYANG_AUDIO_CODEC_H
