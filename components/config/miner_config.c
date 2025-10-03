/**
 * Configuration Component Implementation
 */

#include "miner_config.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

// Include user configuration
#include "config.h"

static const char *TAG = "CONFIG";
static const char *NVS_NAMESPACE = "miner";
static const char *NVS_KEY = "config";

// Current configuration (cached in RAM)
static miner_config_t current_config = {0};
static bool config_initialized = false;

/**
 * @brief Load default configuration from config.h
 */
static void config_load_defaults(miner_config_t *config)
{
    memset(config, 0, sizeof(miner_config_t));

    // WiFi defaults
    strncpy(config->wifi_ssid, WIFI_SSID, sizeof(config->wifi_ssid) - 1);
    strncpy(config->wifi_password, WIFI_PASSWORD, sizeof(config->wifi_password) - 1);

    // Bitcoin defaults
    strncpy(config->btc_pool_url, BTC_POOL_URL, sizeof(config->btc_pool_url) - 1);
    config->btc_pool_port = BTC_POOL_PORT;
    strncpy(config->btc_wallet, BTC_WALLET_ADDRESS, sizeof(config->btc_wallet) - 1);
    strncpy(config->btc_worker, BTC_WORKER_NAME, sizeof(config->btc_worker) - 1);

    // Duino-Coin defaults
    strncpy(config->duco_username, DUCO_USERNAME, sizeof(config->duco_username) - 1);
    strncpy(config->duco_mining_key, DUCO_MINING_KEY, sizeof(config->duco_mining_key) - 1);
    strncpy(config->duco_server, DUCO_SERVER, sizeof(config->duco_server) - 1);
    config->duco_port = DUCO_PORT;

    // General settings
    config->active_mode = (mining_mode_t)DEFAULT_MINING_MODE;
    config->backlight_timeout_sec = BACKLIGHT_TIMEOUT_SEC;
    config->backlight_brightness = BACKLIGHT_DEFAULT_BRIGHTNESS;

    // Set magic number and configured flag
    config->magic = CONFIG_MAGIC;
    config->configured = true;

    ESP_LOGI(TAG, "Loaded default configuration from config.h");
}

esp_err_t config_init(void)
{
    if (config_initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing configuration system...");

    // Try to load from NVS first
    esp_err_t ret = config_load(&current_config);

    if (ret == ESP_ERR_NVS_NOT_FOUND || ret == ESP_FAIL) {
        // No valid config in NVS, load defaults
        ESP_LOGW(TAG, "No valid configuration found in NVS, loading defaults");
        config_load_defaults(&current_config);

        // Save defaults to NVS
        ret = config_save(&current_config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save default config to NVS: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "Default configuration saved to NVS");
    } else if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configuration loaded from NVS");
    } else {
        ESP_LOGE(TAG, "Failed to load configuration: %s", esp_err_to_name(ret));
        return ret;
    }

    config_initialized = true;
    ESP_LOGI(TAG, "Configuration system initialized successfully");
    ESP_LOGI(TAG, "Active mining mode: %s",
             current_config.active_mode == MINING_MODE_BITCOIN ? "Bitcoin" : "Duino-Coin");

    return ESP_OK;
}

esp_err_t config_load(miner_config_t *config)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    // Open NVS
    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Read configuration blob
    size_t required_size = sizeof(miner_config_t);
    ret = nvs_get_blob(nvs_handle, NVS_KEY, config, &required_size);

    nvs_close(nvs_handle);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read config from NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Validate magic number
    if (config->magic != CONFIG_MAGIC) {
        ESP_LOGW(TAG, "Invalid config magic number: 0x%08lX (expected 0x%08lX)",
                 (unsigned long)config->magic, (unsigned long)CONFIG_MAGIC);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t config_save(const miner_config_t *config)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;

    // Open NVS
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS for write: %s", esp_err_to_name(ret));
        return ret;
    }

    // Write configuration blob
    ret = nvs_set_blob(nvs_handle, NVS_KEY, config, sizeof(miner_config_t));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write config to NVS: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit NVS: %s", esp_err_to_name(ret));
        nvs_close(nvs_handle);
        return ret;
    }

    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Configuration saved to NVS");
    return ESP_OK;
}

esp_err_t config_reset(void)
{
    ESP_LOGI(TAG, "Resetting configuration to defaults...");

    // Load defaults
    config_load_defaults(&current_config);

    // Save to NVS
    esp_err_t ret = config_save(&current_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset configuration: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Configuration reset successful");
    return ESP_OK;
}

mining_mode_t config_get_mode(void)
{
    if (!config_initialized) {
        ESP_LOGW(TAG, "Config not initialized, returning default mode");
        return MINING_MODE_DUINOCOIN;
    }
    return current_config.active_mode;
}

esp_err_t config_set_mode(mining_mode_t mode)
{
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config not initialized");
        return ESP_FAIL;
    }

    if (mode != MINING_MODE_BITCOIN && mode != MINING_MODE_DUINOCOIN) {
        ESP_LOGE(TAG, "Invalid mining mode: %d", mode);
        return ESP_ERR_INVALID_ARG;
    }

    current_config.active_mode = mode;

    ESP_LOGI(TAG, "Mining mode set to: %s",
             mode == MINING_MODE_BITCOIN ? "Bitcoin" : "Duino-Coin");

    // Save to NVS
    return config_save(&current_config);
}

bool config_is_valid(const miner_config_t *config)
{
    if (!config || config->magic != CONFIG_MAGIC) {
        return false;
    }

    // Check WiFi config
    if (strlen(config->wifi_ssid) == 0) {
        ESP_LOGW(TAG, "WiFi SSID not configured");
        return false;
    }

    // Check mode-specific config
    if (config->active_mode == MINING_MODE_BITCOIN) {
        if (strlen(config->btc_pool_url) == 0) {
            ESP_LOGW(TAG, "Bitcoin pool URL not configured");
            return false;
        }
        if (config->btc_pool_port == 0) {
            ESP_LOGW(TAG, "Bitcoin pool port not configured");
            return false;
        }
        if (strlen(config->btc_wallet) == 0) {
            ESP_LOGW(TAG, "Bitcoin wallet not configured");
            return false;
        }
    } else if (config->active_mode == MINING_MODE_DUINOCOIN) {
        if (strlen(config->duco_username) == 0) {
            ESP_LOGW(TAG, "Duino-Coin username not configured");
            return false;
        }
        if (strlen(config->duco_server) == 0) {
            ESP_LOGW(TAG, "Duino-Coin server not configured");
            return false;
        }
        if (config->duco_port == 0) {
            ESP_LOGW(TAG, "Duino-Coin port not configured");
            return false;
        }
    }

    return true;
}

const miner_config_t* config_get_current(void)
{
    if (!config_initialized) {
        ESP_LOGW(TAG, "Config not initialized");
        return NULL;
    }
    return &current_config;
}

void config_print(void)
{
    if (!config_initialized) {
        ESP_LOGW(TAG, "Config not initialized");
        return;
    }

    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "Current Configuration:");
    ESP_LOGI(TAG, "===========================================");

    // WiFi (mask password)
    ESP_LOGI(TAG, "WiFi SSID: %s", current_config.wifi_ssid);
    ESP_LOGI(TAG, "WiFi Password: %s", strlen(current_config.wifi_password) > 0 ? "***" : "(not set)");

    // Bitcoin
    ESP_LOGI(TAG, "--- Bitcoin Configuration ---");
    ESP_LOGI(TAG, "Pool: %s:%d", current_config.btc_pool_url, current_config.btc_pool_port);
    ESP_LOGI(TAG, "Wallet: %s", current_config.btc_wallet);
    ESP_LOGI(TAG, "Worker: %s", current_config.btc_worker);

    // Duino-Coin
    ESP_LOGI(TAG, "--- Duino-Coin Configuration ---");
    ESP_LOGI(TAG, "Username: %s", current_config.duco_username);
    ESP_LOGI(TAG, "Mining Key: %s", strlen(current_config.duco_mining_key) > 0 ? "***" : "(not set)");
    ESP_LOGI(TAG, "Server: %s:%d", current_config.duco_server, current_config.duco_port);

    // General
    ESP_LOGI(TAG, "--- General Settings ---");
    ESP_LOGI(TAG, "Active Mode: %s",
             current_config.active_mode == MINING_MODE_BITCOIN ? "Bitcoin" : "Duino-Coin");
    ESP_LOGI(TAG, "Backlight Timeout: %d seconds", current_config.backlight_timeout_sec);
    ESP_LOGI(TAG, "Backlight Brightness: %d%%", current_config.backlight_brightness);
    ESP_LOGI(TAG, "Configured: %s", current_config.configured ? "Yes" : "No");
    ESP_LOGI(TAG, "Valid: %s", config_is_valid(&current_config) ? "Yes" : "No");
    ESP_LOGI(TAG, "===========================================");
}
