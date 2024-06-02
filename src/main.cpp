#include "main.hpp"

/*---------- Main ----------*/

extern "C" void app_main(void)
{
    static const char* TAG = "MAIN";
    esp_err_t status;

    status = setupSensors();
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensors: %s", esp_err_to_name(status));
    }

    status = setupWolfClient(SENSOR_WATER);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WolfClient: %s", esp_err_to_name(status));
    }

    status = startReporting(gatewayAddress);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start reporting: %s", esp_err_to_name(status));
    }

    ESP_LOGI(TAG, "Setup complete.");


}
