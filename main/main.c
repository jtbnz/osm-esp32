/**
 * ESP32 Hybrid Crypto Miner
 * Main application entry point
 *
 * Dual-mode cryptocurrency miner:
 * - Bitcoin (SHA-256) lottery mining
 * - Duino-Coin (DUCO-S1) practical mining
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "ESP32 Hybrid Crypto Miner v1.0.0");
    ESP_LOGI(TAG, "Bitcoin + Duino-Coin Dual Mining");
    ESP_LOGI(TAG, "===========================================");

    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    // TODO: Load configuration
    // TODO: Initialize display
    // TODO: Show splash screen
    // TODO: Initialize WiFi
    // TODO: Start web server
    // TODO: Initialize stats
    // TODO: Start mining coordinator

    ESP_LOGI(TAG, "Initialization complete - entering main loop");

    // Main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "System running...");
    }
}
