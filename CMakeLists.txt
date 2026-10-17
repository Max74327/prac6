cmake_minimum_required(VERSION 3.28)

project(prac6
    VERSION 1.0
    DESCRIPTION "Practical work 6_1: hash table + binary file indexing"
    LANGUAGES CXX)

# ---- Стандарт и обязательные фичи ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Сканирование модулей (CMake >= 3.28)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Экспорт compile_commands.json для clangd/IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Библиотека модулей ----
# Один интерфейсный таргет: все .cppm, общие для основного приложения и утилит.
add_library(prac6_modules)

target_sources(prac6_modules
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/record_concept.cppm
            src/hash_table.cppm
            src/bin_io.cppm
            src/record_car.cppm
            src/record_select.cppm
            src/file_hash.cppm
            src/file_io.cppm
)

# Публичный дефайн выбора варианта для потребителей
target_compile_definitions(prac6_modules
    PUBLIC PRAC6_VARIANT=${PRAC6_VARIANT})

# ---- Основное приложение ----
add_executable(prac6 src/main.cpp)
target_link_libraries(prac6 PRIVATE prac6_modules)

# ---- Утилиты (требование задания) ----
add_executable(gen  src/gen.cpp)
target_link_libraries(gen PRIVATE prac6_modules)

add_executable(dump src/dump.cpp)
target_link_libraries(dump PRIVATE prac6_modules)

# ---- Каталог для данных ----
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(prac6_modules PUBLIC
        -stdlib=libc++
        -Wall -Wextra -Wpedantic)
    target_link_options(prac6 PRIVATE -stdlib=libc++)
    target_link_options(gen  PRIVATE -stdlib=libc++)
    target_link_options(dump PRIVATE -stdlib=libc++)
endif()
