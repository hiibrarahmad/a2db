#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/i2s.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_bt_device.h"

#define TAG "A2DP_I2S"

#include "esp_avrc_api.h"

// I2S GPIO configuration (match your hardware)
#define I2S_BCLK 27
#define I2S_LRCK 26
#define I2S_DOUT 25

// Global variable to track volume state.
static uint8_t last_volume = 0;

void bt_avrc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    ESP_LOGI(TAG, "AVRCP Callback event: %d", event);

    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT:
            ESP_LOGI(TAG, "📶 AVRCP connected");
            if (param->conn_stat.connected) {
                // Immediately request metadata.
                esp_avrc_ct_send_metadata_cmd(0,
                    ESP_AVRC_MD_ATTR_TITLE |
                    ESP_AVRC_MD_ATTR_ARTIST |
                    ESP_AVRC_MD_ATTR_ALBUM);
                // Register for notifications:
                esp_avrc_ct_send_register_notification_cmd(0, ESP_AVRC_RN_PLAY_STATUS_CHANGE, 0);
                esp_avrc_ct_send_register_notification_cmd(0, ESP_AVRC_RN_VOLUME_CHANGE, 0);
            }
            break;

        case ESP_AVRC_CT_METADATA_RSP_EVT:
            ESP_LOGI(TAG, "Metadata Response, attr: %d", param->meta_rsp.attr_id);
            switch (param->meta_rsp.attr_id) {
                case ESP_AVRC_MD_ATTR_TITLE:
                    ESP_LOGI(TAG, "🎵 Title: %.*s", param->meta_rsp.attr_length, param->meta_rsp.attr_text);
                    break;
                case ESP_AVRC_MD_ATTR_ARTIST:
                    ESP_LOGI(TAG, "🎤 Artist: %.*s", param->meta_rsp.attr_length, param->meta_rsp.attr_text);
                    break;
                case ESP_AVRC_MD_ATTR_ALBUM:
                    ESP_LOGI(TAG, "💿 Album: %.*s", param->meta_rsp.attr_length, param->meta_rsp.attr_text);
                    break;
                default:
                    ESP_LOGI(TAG, "Unknown metadata attribute: %d", param->meta_rsp.attr_id);
                    break;
            }
            break;

        case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
            ESP_LOGI(TAG, "AVRCP Change Notify event, ID: %d", param->change_ntf.event_id);
            switch (param->change_ntf.event_id) {
                case ESP_AVRC_RN_PLAY_STATUS_CHANGE: {
                    uint8_t play_status = param->change_ntf.event_parameter.playback;
                    if (play_status == ESP_AVRC_PLAYBACK_PLAYING)
                        ESP_LOGI(TAG, "▶️ Playback: Playing");
                    else if (play_status == ESP_AVRC_PLAYBACK_PAUSED)
                        ESP_LOGI(TAG, "⏸️ Playback: Paused");
                    else if (play_status == ESP_AVRC_PLAYBACK_STOPPED)
                        ESP_LOGI(TAG, "⏹️ Playback: Stopped");
                    else
                        ESP_LOGI(TAG, "Playback status: %d", play_status);
                    // Re-register to continue receiving updates.
                    esp_avrc_ct_send_register_notification_cmd(0, ESP_AVRC_RN_PLAY_STATUS_CHANGE, 0);
                    break;
                }
                case ESP_AVRC_RN_VOLUME_CHANGE: {
                    uint8_t new_volume = param->change_ntf.event_parameter.volume;
                    int diff = (int)new_volume - (int)last_volume;
                    if (diff > 0)
                        ESP_LOGI(TAG, "🔊 Volume increased by %d (from %d to %d)", diff, last_volume, new_volume);
                    else if (diff < 0)
                        ESP_LOGI(TAG, "🔊 Volume decreased by %d (from %d to %d)", -diff, last_volume, new_volume);
                    else
                        ESP_LOGI(TAG, "🔊 Volume unchanged: %d", new_volume);
                    last_volume = new_volume;
                    // Re-register for volume notifications.
                    esp_avrc_ct_send_register_notification_cmd(0, ESP_AVRC_RN_VOLUME_CHANGE, 0);
                    break;
                }
                default:
                    ESP_LOGI(TAG, "Unhandled notify event: %d", param->change_ntf.event_id);
                    break;
            }
            break;

        default:
            ESP_LOGI(TAG, "Unhandled AVRCP event: %d", event);
            break;
    }
}

void i2s_init()
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRCK,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(I2S_NUM_0));
}

void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len)
{
    size_t bytes_written;
    i2s_write(I2S_NUM_0, data, len, &bytes_written, portMAX_DELAY);
}

void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT:
            ESP_LOGI(TAG, "A2DP connection state: %d", param->conn_stat.state);
            break;
        case ESP_A2D_AUDIO_STATE_EVT:
            ESP_LOGI(TAG, "A2DP audio state: %d", param->audio_stat.state);
            break;
        default:
            ESP_LOGI(TAG, "Unhandled A2DP event: %d", event);
            break;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_bt_gap_set_device_name("ESP32-SPEAKER"));

    // Initialize AVRCP before A2DP (as in the default example).
    ESP_ERROR_CHECK(esp_avrc_ct_init());
    ESP_ERROR_CHECK(esp_avrc_ct_register_callback(bt_avrc_ct_cb));

    i2s_init();

    ESP_ERROR_CHECK(esp_a2d_register_callback(&bt_app_a2d_cb));
    ESP_ERROR_CHECK(esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb));
    ESP_ERROR_CHECK(esp_a2d_sink_init());

    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    ESP_LOGI(TAG, "✅ Ready to pair and stream.");
}
