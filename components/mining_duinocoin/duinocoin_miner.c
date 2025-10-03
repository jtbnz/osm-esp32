/**
 * Duino-Coin Mining Implementation
 *
 * DUCO-S1 Protocol:
 * 1. Connect to server.duinocoin.com:2811
 * 2. Server sends version (e.g., "3.0")
 * 3. Send: "JOB,username,difficulty_level"
 * 4. Receive: "last_hash,expected_hash,difficulty"
 * 5. Find nonce where SHA1(last_hash + nonce) == expected_hash
 * 6. Send: "nonce,hashrate,miner_name,rig_id"
 * 7. Receive: "GOOD" or "BAD" + share_value
 * 8. Repeat from step 3
 */

#include "duinocoin_miner.h"
#include "miner_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "mbedtls/sha1.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "DUCO";

// Protocol constants
#define DUCO_MINER_NAME "ESP32-Miner"
#define DUCO_DIFFICULTY "ESP32"  // Request ESP32-appropriate difficulty
#define DUCO_BUFFER_SIZE 256
#define DUCO_CONNECT_TIMEOUT_MS 10000
#define DUCO_READ_TIMEOUT_MS 30000

// Mining state
static duco_state_t current_state = DUCO_STATE_IDLE;
static TaskHandle_t mining_task_handle = NULL;
static int sock = -1;
static bool stop_requested = false;

// Statistics
static duco_stats_t stats = {0};
static uint64_t total_hashes = 0;
static int64_t mining_start_time = 0;

/**
 * @brief Calculate SHA1 hash
 */
static void sha1_hash(const char *input, char *output_hex)
{
    unsigned char hash[20];
    mbedtls_sha1_context ctx;

    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts(&ctx);
    mbedtls_sha1_update(&ctx, (const unsigned char *)input, strlen(input));
    mbedtls_sha1_finish(&ctx, hash);
    mbedtls_sha1_free(&ctx);

    // Convert to hex string
    for (int i = 0; i < 20; i++) {
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    }
    output_hex[40] = '\0';
}

/**
 * @brief Connect to Duino-Coin server
 */
static esp_err_t duco_connect(void)
{
    const miner_config_t *config = config_get_current();
    if (!config) {
        ESP_LOGE(TAG, "Config not available");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connecting to %s:%d...", config->duco_server, config->duco_port);
    current_state = DUCO_STATE_CONNECTING;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
        current_state = DUCO_STATE_ERROR;
        return ESP_FAIL;
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = DUCO_CONNECT_TIMEOUT_MS / 1000;
    timeout.tv_usec = (DUCO_CONNECT_TIMEOUT_MS % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Resolve hostname
    struct hostent *host = gethostbyname(config->duco_server);
    if (host == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed for %s", config->duco_server);
        close(sock);
        sock = -1;
        current_state = DUCO_STATE_ERROR;
        return ESP_FAIL;
    }

    // Connect
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(config->duco_port);
    dest_addr.sin_addr.s_addr = *(uint32_t *)host->h_addr;

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket connect failed: errno %d", errno);
        close(sock);
        sock = -1;
        current_state = DUCO_STATE_ERROR;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connected to Duino-Coin server");
    current_state = DUCO_STATE_CONNECTED;

    // Read server version
    char buffer[DUCO_BUFFER_SIZE];
    int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        // Remove newline
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        ESP_LOGI(TAG, "Server version: %s", buffer);
    }

    return ESP_OK;
}

/**
 * @brief Disconnect from server
 */
static void duco_disconnect(void)
{
    if (sock >= 0) {
        close(sock);
        sock = -1;
        ESP_LOGI(TAG, "Disconnected from server");
    }
    current_state = DUCO_STATE_IDLE;
}

/**
 * @brief Mine a single job
 */
static esp_err_t duco_mine_job(void)
{
    const miner_config_t *config = config_get_current();
    if (!config) return ESP_FAIL;

    char buffer[DUCO_BUFFER_SIZE];
    int len;

    // Request job
    const char *mining_key = strlen(config->duco_mining_key) > 0 ? config->duco_mining_key : "";
    snprintf(buffer, sizeof(buffer), "JOB,%s,%s,%s\n",
             config->duco_username, DUCO_DIFFICULTY, mining_key);

    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        ESP_LOGE(TAG, "Failed to send job request");
        return ESP_FAIL;
    }

    // Receive job
    len = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        ESP_LOGE(TAG, "Failed to receive job");
        return ESP_FAIL;
    }
    buffer[len] = '\0';

    // Parse job: "last_hash,expected_hash,difficulty"
    char *last_hash = strtok(buffer, ",");
    char *expected_hash = strtok(NULL, ",");
    char *diff_str = strtok(NULL, ",\n");

    if (!last_hash || !expected_hash || !diff_str) {
        ESP_LOGE(TAG, "Invalid job format");
        return ESP_FAIL;
    }

    uint32_t difficulty = atoi(diff_str);
    stats.current_difficulty = difficulty;

    ESP_LOGI(TAG, "Job received - Difficulty: %lu", (unsigned long)difficulty);
    ESP_LOGD(TAG, "Last hash: %.20s...", last_hash);
    ESP_LOGD(TAG, "Expected: %.20s...", expected_hash);

    // Mine: find nonce where SHA1(last_hash + nonce) == expected_hash
    int64_t start_time = esp_timer_get_time();
    char hash_input[256];
    char hash_output[41];
    uint32_t nonce = 0;

    for (nonce = 0; nonce < difficulty * 100 + 1; nonce++) {
        // Check for stop request
        if (stop_requested) {
            return ESP_ERR_INVALID_STATE;
        }

        // Create hash input: last_hash + nonce
        snprintf(hash_input, sizeof(hash_input), "%s%u", last_hash, nonce);

        // Calculate SHA1
        sha1_hash(hash_input, hash_output);

        // Count hash
        total_hashes++;

        // Check if it matches
        if (strcmp(hash_output, expected_hash) == 0) {
            // Found it!
            int64_t end_time = esp_timer_get_time();
            float duration_sec = (end_time - start_time) / 1000000.0f;
            float hashrate = nonce / duration_sec;

            stats.current_hashrate = hashrate;

            ESP_LOGI(TAG, "Share found! Nonce: %lu, Hashrate: %.2f H/s",
                     (unsigned long)nonce, hashrate);

            // Submit result
            snprintf(buffer, sizeof(buffer), "%u,%.2f,%s,%s\n",
                     nonce, hashrate, DUCO_MINER_NAME, "");

            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                ESP_LOGE(TAG, "Failed to send result");
                return ESP_FAIL;
            }

            // Receive response: "GOOD" or "BAD" + optional share value
            len = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (len <= 0) {
                ESP_LOGE(TAG, "Failed to receive response");
                return ESP_FAIL;
            }
            buffer[len] = '\0';

            // Remove newline
            char *newline = strchr(buffer, '\n');
            if (newline) *newline = '\0';

            // Parse response
            if (strncmp(buffer, "GOOD", 4) == 0) {
                stats.shares_accepted++;

                // Try to parse share value (DUCO earned)
                char *comma = strchr(buffer, ',');
                if (comma) {
                    float share_value = atof(comma + 1);
                    stats.duco_earned_today += share_value;
                    stats.duco_earned_total += share_value;

                    ESP_LOGI(TAG, "✓ GOOD! Earned: %.8f DUCO (Total: %.8f)",
                             share_value, stats.duco_earned_total);
                } else {
                    ESP_LOGI(TAG, "✓ GOOD! Share accepted");
                }

                strncpy(stats.last_message, "GOOD - Share accepted", sizeof(stats.last_message) - 1);
            } else if (strncmp(buffer, "BAD", 3) == 0) {
                stats.shares_rejected++;
                ESP_LOGW(TAG, "✗ BAD! Share rejected");
                strncpy(stats.last_message, "BAD - Share rejected", sizeof(stats.last_message) - 1);
            } else {
                ESP_LOGW(TAG, "Unknown response: %s", buffer);
            }

            return ESP_OK;
        }

        // Yield every 1000 hashes to not block other tasks
        if (nonce % 1000 == 0) {
            taskYIELD();
        }
    }

    ESP_LOGW(TAG, "Failed to find nonce within difficulty range");
    return ESP_FAIL;
}

/**
 * @brief Mining task
 */
static void duco_mining_task(void *param)
{
    ESP_LOGI(TAG, "Mining task started");
    mining_start_time = esp_timer_get_time();

    while (!stop_requested) {
        // Connect if not connected
        if (sock < 0) {
            if (duco_connect() != ESP_OK) {
                ESP_LOGE(TAG, "Connection failed, retrying in 10s...");
                vTaskDelay(pdMS_TO_TICKS(10000));
                continue;
            }
        }

        // Mine a job
        current_state = DUCO_STATE_MINING;
        esp_err_t ret = duco_mine_job();

        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "Mining job failed, reconnecting...");
            duco_disconnect();
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

        // Update stats
        int64_t now = esp_timer_get_time();
        stats.uptime_seconds = (now - mining_start_time) / 1000000;

        // Calculate average hashrate
        if (stats.uptime_seconds > 0) {
            stats.avg_hashrate = (float)total_hashes / (float)stats.uptime_seconds;
        }

        // Small delay between jobs
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Cleanup
    duco_disconnect();
    ESP_LOGI(TAG, "Mining task stopped");
    mining_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t duco_miner_init(void)
{
    ESP_LOGI(TAG, "Initializing Duino-Coin miner...");

    // Verify config
    const miner_config_t *config = config_get_current();
    if (!config) {
        ESP_LOGE(TAG, "Configuration not available");
        return ESP_FAIL;
    }

    if (strlen(config->duco_username) == 0) {
        ESP_LOGE(TAG, "Duino-Coin username not configured");
        return ESP_FAIL;
    }

    // Initialize stats
    memset(&stats, 0, sizeof(stats));
    stats.state = DUCO_STATE_IDLE;
    total_hashes = 0;
    stop_requested = false;

    ESP_LOGI(TAG, "Duino-Coin miner initialized");
    ESP_LOGI(TAG, "Username: %s", config->duco_username);
    ESP_LOGI(TAG, "Server: %s:%d", config->duco_server, config->duco_port);
    ESP_LOGI(TAG, "Mining key: %s", strlen(config->duco_mining_key) > 0 ? "Set" : "Not set");

    return ESP_OK;
}

esp_err_t duco_miner_start(void)
{
    if (mining_task_handle != NULL) {
        ESP_LOGW(TAG, "Miner already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting Duino-Coin mining...");
    stop_requested = false;

    // Create mining task
    BaseType_t ret = xTaskCreatePinnedToCore(
        duco_mining_task,
        "duco_miner",
        8192,  // Stack size
        NULL,
        5,     // Priority
        &mining_task_handle,
        1      // Core 1
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create mining task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Duino-Coin mining started");
    return ESP_OK;
}

esp_err_t duco_miner_stop(void)
{
    if (mining_task_handle == NULL) {
        ESP_LOGW(TAG, "Miner not running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping Duino-Coin mining...");
    stop_requested = true;

    // Wait for task to stop (max 5 seconds)
    int timeout = 50;
    while (mining_task_handle != NULL && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(100));
        timeout--;
    }

    if (mining_task_handle != NULL) {
        ESP_LOGW(TAG, "Force deleting mining task");
        vTaskDelete(mining_task_handle);
        mining_task_handle = NULL;
    }

    duco_disconnect();
    current_state = DUCO_STATE_IDLE;

    ESP_LOGI(TAG, "Duino-Coin mining stopped");
    return ESP_OK;
}

duco_state_t duco_miner_get_state(void)
{
    return current_state;
}

esp_err_t duco_miner_get_stats(duco_stats_t *out_stats)
{
    if (!out_stats) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(out_stats, &stats, sizeof(duco_stats_t));
    return ESP_OK;
}

bool duco_miner_is_running(void)
{
    return mining_task_handle != NULL;
}
