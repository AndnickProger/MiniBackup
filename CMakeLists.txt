cmake_minimum_required(VERSION 3.10)
project(MiniBackup VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем unit_test_framework к списку компонентов Boost
find_package(Boost REQUIRED COMPONENTS 
    filesystem 
    serialization 
    system 
    unit_test_framework
)

# Поиск системных библиотек (Zstd и SQLite3)
find_library(ZSTD_LIB zstd REQUIRED)
find_library(SQLITE3_LIB sqlite3 REQUIRED)


# Исходники основной программы (без main.cpp для тестов)
set(COMMON_SOURCES 
    src/engine.cpp 
    src/storage.cpp 
    src/compressor.cpp
)

# Основной исполняемый файл
add_executable(mini_backup src/main.cpp ${COMMON_SOURCES})

target_include_directories(mini_backup PRIVATE include)

target_link_libraries(mini_backup 
    PRIVATE 
    Boost::filesystem 
    Boost::serialization 
    Boost::system
    ${ZSTD_LIB}
    ${SQLITE3_LIB}
    pthread
)

# Настройка Boost.Test
enable_testing()

add_executable(run_tests 
    tests/test_logic.cpp 
    ${COMMON_SOURCES} # Тестируем логику в связке с движком
)

target_include_directories(run_tests PRIVATE include)

target_link_libraries(run_tests 
    PRIVATE 
    Boost::unit_test_framework
    Boost::filesystem
    Boost::serialization
    ${ZSTD_LIB}
    ${SQLITE3_LIB}
    pthread
)

# Позволяет запускать через 'ctest'
add_test(NAME AllTests COMMAND run_tests)

