/**
 * Duino-Coin Mining Component
 *
 * Implements DUCO-S1 algorithm for ESP32
 * Based on official Duino-Coin ESP32 implementation
 */

#ifndef DUINOCOIN_MINER_H
#define DUINOCOIN_MINER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Mining state
typedef enum {
    DUCO_STATE_IDLE = 0,
    DUCO_STATE_CONNECTING,
    DUCO_STATE_CONNECTED,
    DUCO_STATE_MINING,
    DUCO_STATE_ERROR
} duco_state_t;

// Mining statistics
typedef struct {
    uint32_t shares_accepted;
    uint32_t shares_rejected;
    float duco_earned_today;
    float duco_earned_total;
    float current_hashrate;
    float avg_hashrate;
    uint32_t current_difficulty;
    uint32_t uptime_seconds;
    duco_state_t state;
    char last_message[128];
} duco_stats_t;

/**
 * @brief Initialize Duino-Coin miner
 *
 * Sets up the mining engine with configuration from NVS
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t duco_miner_init(void);

/**
 * @brief Start Duino-Coin mining
 *
 * Connects to server and begins mining loop
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t duco_miner_start(void);

/**
 * @brief Stop Duino-Coin mining
 *
 * Gracefully stops mining and disconnects
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t duco_miner_stop(void);

/**
 * @brief Get current mining state
 *
 * @return Current state
 */
duco_state_t duco_miner_get_state(void);

/**
 * @brief Get mining statistics
 *
 * @param stats Pointer to stats structure to populate
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t duco_miner_get_stats(duco_stats_t *stats);

/**
 * @brief Check if miner is running
 *
 * @return true if mining, false otherwise
 */
bool duco_miner_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // DUINOCOIN_MINER_H
