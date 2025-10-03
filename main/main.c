/**
 * ESP32 Hybrid Crypto Miner
 * Main application entry point
 *
 * Dual-mode cryptocurrency miner:
 * - Bitcoin (SHA-256) lottery mining
 * - Duino-Coin (DUCO-S1) practical mining
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "miner_config.h"
#include "duinocoin_miner.h"

static const char *TAG = "MAIN";
static bool wifi_connected = false;

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

    // Load configuration
    ret = config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize configuration: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "System halted");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Print current configuration (for debugging)
    config_print();

    // Get current config
    const miner_config_t *config = config_get_current();
    if (!config) {
        ESP_LOGE(TAG, "Failed to get current configuration");
        return;
    }

    // Validate configuration
    if (!config_is_valid(config)) {
        ESP_LOGW(TAG, "Configuration is incomplete or invalid");
        ESP_LOGW(TAG, "Please update config.h and reflash");
    } else {
        ESP_LOGI(TAG, "Configuration is valid");
    }

    // Initialize WiFi (simple station mode for now)
    ESP_LOGI(TAG, "Initializing WiFi...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, config->wifi_ssid, sizeof(wifi_config.sta.ssid));
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    strncpy((char *)wifi_config.sta.password, config->wifi_password, sizeof(wifi_config.sta.password));
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s...", config->wifi_ssid);
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for WiFi connection (simple blocking wait for now)
    ESP_LOGI(TAG, "Waiting for WiFi connection (30 seconds)...");
    for (int i = 0; i < 30; i++) {
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            wifi_connected = true;
            ESP_LOGI(TAG, "WiFi connected! IP will be assigned soon");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "Waiting... %d/30", i + 1);
    }

    if (!wifi_connected) {
        ESP_LOGW(TAG, "WiFi connection failed, but continuing anyway...");
        ESP_LOGW(TAG, "Mining may not work without network!");
    }

    // Give DHCP time to assign IP
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Initialize Duino-Coin miner if in DUCO mode
    if (config->active_mode == MINING_MODE_DUINOCOIN) {
        ESP_LOGI(TAG, "Initializing Duino-Coin miner...");
        ret = duco_miner_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize Duino-Coin miner");
        } else {
            ESP_LOGI(TAG, "Starting Duino-Coin mining...");
            ret = duco_miner_start();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start mining");
            } else {
                ESP_LOGI(TAG, "Mining started successfully!");
            }
        }
    } else {
        ESP_LOGI(TAG, "Bitcoin mode not implemented yet");
    }

    ESP_LOGI(TAG, "Initialization complete - entering main loop");
    ESP_LOGI(TAG, "Current mode: %s",
             config->active_mode == MINING_MODE_BITCOIN ? "Bitcoin" : "Duino-Coin");

    // Main loop - print stats every 30 seconds
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));

        if (config->active_mode == MINING_MODE_DUINOCOIN && duco_miner_is_running()) {
            duco_stats_t stats;
            if (duco_miner_get_stats(&stats) == ESP_OK) {
                ESP_LOGI(TAG, "=== Duino-Coin Stats ===");
                ESP_LOGI(TAG, "State: %d", stats.state);
                ESP_LOGI(TAG, "Hashrate: %.2f H/s (avg: %.2f H/s)",
                         stats.current_hashrate, stats.avg_hashrate);
                ESP_LOGI(TAG, "Shares: %lu accepted, %lu rejected",
                         (unsigned long)stats.shares_accepted,
                         (unsigned long)stats.shares_rejected);
                ESP_LOGI(TAG, "DUCO Earned: %.8f (today: %.8f)",
                         stats.duco_earned_total, stats.duco_earned_today);
                ESP_LOGI(TAG, "Uptime: %lu seconds", (unsigned long)stats.uptime_seconds);
                ESP_LOGI(TAG, "=======================");
            }
        } else {
            ESP_LOGI(TAG, "System running...");
        }
    }
}
