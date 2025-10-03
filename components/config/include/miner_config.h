/**
 * Configuration Component
 *
 * Manages persistent configuration storage using NVS (Non-Volatile Storage).
 * Supports dual-mode mining configuration (Bitcoin + Duino-Coin).
 */

#ifndef MINER_CONFIG_H
#define MINER_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Mining mode enumeration
typedef enum {
    MINING_MODE_BITCOIN = 0,
    MINING_MODE_DUINOCOIN = 1
} mining_mode_t;

// Configuration structure
typedef struct {
    // WiFi configuration
    char wifi_ssid[32];
    char wifi_password[64];

    // Bitcoin configuration
    char btc_pool_url[128];
    uint16_t btc_pool_port;
    char btc_wallet[64];
    char btc_worker[32];

    // Duino-Coin configuration
    char duco_username[32];
    char duco_mining_key[64];
    char duco_server[128];
    uint16_t duco_port;

    // General settings
    mining_mode_t active_mode;
    uint8_t backlight_timeout_sec;
    uint8_t backlight_brightness;

    // Internal flags
    bool configured;
    uint32_t magic;  // Magic number to verify valid config
} miner_config_t;

#define CONFIG_MAGIC 0xDEADBEEF  // Magic number for config validation

/**
 * @brief Initialize configuration system
 *
 * Initializes NVS and loads configuration from storage.
 * If no valid config exists, loads defaults from config.h
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_init(void);

/**
 * @brief Load configuration from NVS
 *
 * @param config Pointer to configuration structure to populate
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no config exists
 */
esp_err_t config_load(miner_config_t *config);

/**
 * @brief Save configuration to NVS
 *
 * @param config Pointer to configuration structure to save
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_save(const miner_config_t *config);

/**
 * @brief Reset configuration to defaults
 *
 * Loads default values from config.h and saves to NVS
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_reset(void);

/**
 * @brief Get current mining mode
 *
 * @return Current mining mode (BITCOIN or DUINOCOIN)
 */
mining_mode_t config_get_mode(void);

/**
 * @brief Set mining mode
 *
 * Updates the active mining mode and saves to NVS
 *
 * @param mode New mining mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_set_mode(mining_mode_t mode);

/**
 * @brief Validate configuration for current mode
 *
 * Checks if all required fields are set for the active mining mode
 *
 * @param config Pointer to configuration to validate
 * @return true if valid, false otherwise
 */
bool config_is_valid(const miner_config_t *config);

/**
 * @brief Get current configuration
 *
 * Returns pointer to current configuration (read-only)
 * Use config_save() to persist changes
 *
 * @return Pointer to current config, or NULL if not initialized
 */
const miner_config_t* config_get_current(void);

/**
 * @brief Print configuration (for debugging)
 *
 * Prints current configuration to console (masks sensitive data)
 */
void config_print(void);

#ifdef __cplusplus
}
#endif

#endif // MINER_CONFIG_H
