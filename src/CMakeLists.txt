# ----------------------------------------------------------------------
#  src/CMakeLists.txt
#
#  Построение исполняемого файла qr_slam_demo.
#  Собираем всё содержимое src/, включая утилиты в utils/.
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

# 1) Сразу создаём таргет executable, указывая все исходники:
#    Убедитесь, что в папке src/ лежат:
#      - main.cpp
#      - App.cpp, App.hpp
#      - SlamWrapper.cpp, SlamWrapper.hpp
#      - MarkerTracker.cpp, MarkerTracker.hpp
#      - папка utils/ с Geometry.hpp и Timer.hpp

add_executable(qr_slam_demo
        main.cpp
        App.cpp
        SlamWrapper.cpp
        MarkerTracker.cpp
)

# 2) Указываем include-пути ДЛЯ ТАРГЕТА qr_slam_demo:
#    – сама папка src/ (там лежат App.hpp, MarkerTracker.hpp и т.д.)
#    – папка src/utils (там лежат Geometry.hpp, Timer.hpp)

target_include_directories(qr_slam_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}             # "./src"
        ${CMAKE_CURRENT_SOURCE_DIR}/utils       # "./src/utils"
)

# 3) Привязываем внешние библиотеки к таргету qr_slam_demo:
#    – OpenCV (из корневого CMake нашли OpenCV и сохранили OpenCV_LIBS)
#    – Eigen3::Eigen (из корневого CMake нашли Eigen3)
#    – StellaVSLAM::StellaVSLAM (наш Find-модуль создал импортированный таргет)

target_link_libraries(qr_slam_demo PRIVATE
        ${OpenCV_LIBS}
        Eigen3::Eigen
        StellaVSLAM::StellaVSLAM
)

# 4) (Опционально) здесь можно задать особые компиляционные флаги
#    для этого таргета, но чаще это делают в корневом CMakeLists.
