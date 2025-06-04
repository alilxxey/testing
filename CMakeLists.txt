cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# ---------------------------------------------------------------
# QR-SLAM  —  realtime monocular SLAM + QR-code tracking demo
# ---------------------------------------------------------------
project(qr_slam_demo
        VERSION 0.0.0       # авто-подставится из git-тега ↓
        DESCRIPTION "Realtime monocular SLAM fused with QR-code tracking"
        LANGUAGES CXX)

# ---------- версия ----------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/version.cmake)                 # генерирует PROJECT_VERSION*

# ---------- пользовательские опции -----------------------------------
option(USE_PANGOLIN_VIEWER "Enable 3-D trajectory window (requires Pangolin)" ON)
option(WARNINGS_AS_ERRORS   "Treat compiler warnings as errors"             OFF)

# ---------- компилятор + санитайзеры -----------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic $<$<BOOL:${WARNINGS_AS_ERRORS}>:-Werror>)
# убедимся, что debug-сборки ловят UB
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
endif()

# ---------- зависимости -----------------------------------------------
find_package(OpenCV REQUIRED)
find_package(Eigen3  REQUIRED)
find_package(StellaVSLAM REQUIRED)          # <— наш собственный Find-модуль

# ---------- исходники --------------------------------------------------
add_subdirectory(src)

# ---------- экспорт compile_commands.json -----------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- установка --------------------------------------------------
install(TARGETS qr_slam_demo
        RUNTIME DESTINATION bin)

install(DIRECTORY scripts/
        DESTINATION share/qr_slam_demo/scripts
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES config.yaml orb_vocab.fbow
        DESTINATION share/qr_slam_demo)

# ---------- пакеты / CPack --------------------------------------------
include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_CONTACT "dev@yourcompany.com")
