#include "kaiyang_audio_codec.h"

#include <esp_log.h>
#include <driver/i2s_pdm.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>

#include "config.h"

static const char TAG[] = "KaiyangAudioCodec";

KaiyangAudioCodec::KaiyangAudioCodec(int input_sample_rate, int output_sample_rate,
    gpio_num_t mic_clk, gpio_num_t mic_data,
    gpio_num_t spkr_bclk, gpio_num_t spkr_lrclk, gpio_num_t spkr_data,
    gpio_num_t spkr_enable) {
    
    duplex_ = true;  // ʹ�ö�������������?��
    input_reference_ = false;
    input_channels_ = 1;
    input_sample_rate_ = input_sample_rate;
    output_sample_rate_ = output_sample_rate;
    spkr_enable_pin_ = spkr_enable;

    // ���� PDM ��˷�����?
    CreatePdmMicrophone(mic_clk, mic_data);
    
    // ���� I2S ���������?
    CreateI2sSpeaker(spkr_bclk, spkr_lrclk, spkr_data);

    SetOutputVolume(volume_);

    // ����������ʹ������
    if (spkr_enable_pin_ != GPIO_NUM_NC) {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << spkr_enable_pin_);
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
        gpio_set_level(spkr_enable_pin_, 1);  // ��ʼ�ر�
    }

    ESP_LOGI(TAG, "KaiyangAudioCodec initialized");
}

KaiyangAudioCodec::~KaiyangAudioCodec() {
    audio_codec_delete_codec_if(in_codec_if_);
    audio_codec_delete_ctrl_if(in_ctrl_if_);
    audio_codec_delete_codec_if(out_codec_if_);
    audio_codec_delete_ctrl_if(out_ctrl_if_);
    audio_codec_delete_gpio_if(gpio_if_);
    audio_codec_delete_data_if(data_if_);
}

void KaiyangAudioCodec::CreatePdmMicrophone(gpio_num_t clk, gpio_num_t data) {
    // ���� I2S RX ͨ������ PDM ��˷�?
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle_));

    // ���� PDM RX ģʽ
    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG((uint32_t)input_sample_rate_),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = clk,
            .din = data,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_handle_, &pdm_rx_cfg));

    // // ������Ƶ���������?
    // audio_codec_i2s_cfg_t i2s_cfg = {
    //     .port = I2S_NUM_0,
    //     .rx_handle = rx_handle_,
    //     .tx_handle = NULL,
    // };
    // const audio_codec_data_if_t* data_if = audio_codec_new_i2s_data(&i2s_cfg);

    // esp_codec_dev_cfg_t codec_dev_cfg = {
    //     .dev_type = ESP_CODEC_DEV_TYPE_IN,
    //     .codec_if = NULL,
    //     .data_if = data_if,
    // };
    // input_dev_ = esp_codec_dev_new(&codec_dev_cfg);
    
    ESP_LOGI(TAG, "PDM microphone created");
}

void KaiyangAudioCodec::CreateI2sSpeaker(gpio_num_t bclk, gpio_num_t lrclk, gpio_num_t data) {
    // ���� I2S TX ͨ������ MAX98357A
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle_, NULL));

    // ���ñ�׼ I2S ģʽ
    i2s_std_slot_config_t slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = (uint32_t)output_sample_rate_,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = slot_cfg,
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = bclk,
            .ws = lrclk,
            .dout = data,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle_, &std_cfg));

    // // ������Ƶ���������?
    // audio_codec_i2s_cfg_t i2s_cfg = {
    //     .port = I2S_NUM_1,
    //     .rx_handle = NULL,
    //     .tx_handle = tx_handle_,
    // };
    // const audio_codec_data_if_t* data_if = audio_codec_new_i2s_data(&i2s_cfg);

    // esp_codec_dev_cfg_t codec_dev_cfg = {
    //     .dev_type = ESP_CODEC_DEV_TYPE_OUT,
    //     .codec_if = NULL,
    //     .data_if = data_if,
    // };
    // output_dev_ = esp_codec_dev_new(&codec_dev_cfg);
    
    ESP_LOGI(TAG, "I2S speaker created");
}

void KaiyangAudioCodec::SetOutputVolume(int volume) {
    volume_ = volume;
    AudioCodec::SetOutputVolume(volume);
}

void KaiyangAudioCodec::EnableInput(bool enable) {
    AudioCodec::EnableInput(enable);
}

void KaiyangAudioCodec::EnableOutput(bool enable) {
    gpio_set_level(AUDIO_SPKR_ENABLE, enable);
        AudioCodec::EnableOutput(enable);
}

int KaiyangAudioCodec::Read(int16_t* dest, int samples) {
    if (input_enabled_){
        size_t bytes_read;
        i2s_channel_read(rx_handle_, dest, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);

        // ESP_LOGI(TAG, "Left: %d\n", dest[0]);
        // ESP_LOGI(TAG,"Right: %d\n", dest[1]);
    }
    return samples;
}

int KaiyangAudioCodec::Write(const int16_t* data, int samples) {
    if (output_enabled_){
        size_t bytes_read;
        auto output_data = (int16_t *)malloc(samples * sizeof(int16_t));
        for (size_t i = 0; i < samples; i++){
            output_data[i] = (float)data[i] * (float)(volume_ / 80.0);
        }
        i2s_channel_write(tx_handle_, output_data, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);
        free(output_data);
    }
    return samples;
}
