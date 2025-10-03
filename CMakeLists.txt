# ESP32 Hybrid Crypto Miner
# CMakeLists.txt - Top-level build configuration

cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project name and version
set(PROJECT_VER "1.0.0")
project(hybrid-crypto-miner)
