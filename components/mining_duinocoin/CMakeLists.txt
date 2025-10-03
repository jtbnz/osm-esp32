idf_component_register(
    SRCS "duinocoin_miner.c"
    INCLUDE_DIRS "include"
    REQUIRES "lwip" "mbedtls" "config" "esp_timer"
)
