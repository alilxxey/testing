cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(qr_slam_demo
        VERSION 0.0.0
        DESCRIPTION "Realtime monocular SLAM fused with QR-code tracking"
        LANGUAGES CXX)

# 1) Указываем, где искать наши Find-модули
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 2) (опционально) вычисляем PROJECT_VERSION из git
include(cmake/version.cmake)

# 3) Конфигурация компиляции
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

# (Отладочные санитайзеры отключены по умолчанию, включаются флагом)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
endif()

# 4) Находим внешние зависимости
find_package(OpenCV   REQUIRED)
find_package(Eigen3    REQUIRED)
find_package(StellaVSLAM REQUIRED)

# 5) Добавляем поддиректорию src, где лежит свой CMakeLists.txt
add_subdirectory(src)

# 6) Генерация compile_commands.json для IDE/линтеров
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 7) Инсталляция получившегося бинаря и скриптов
install(TARGETS qr_slam_demo
        RUNTIME DESTINATION bin)

install(DIRECTORY scripts/
        DESTINATION share/qr_slam_demo/scripts
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES config.yaml orb_vocab.fbow
        DESTINATION share/qr_slam_demo)

# 8) CPack (только если нужен тарбол)
include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_CONTACT "dev@yourcompany.com")
