# ----------------------------------------------------------------------
#  src/CMakeLists.txt
#
#  Построение исполняемого файла qr_slam_demo.
#  Собираем всё содержимое src/, включая утилиты в utils/.
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

# 1) Указываем, что все исходники лежат в этой директории
cmake_minimum_required(VERSION 3.18)
project(qr_slam_demo_src LANGUAGES CXX)

# 2) Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3) Составляем список исходных файлов
#    Предполагается, что в папке src/ лежат как минимум:
#      - main.cpp
#      - App.cpp, App.hpp
#      - SlamWrapper.cpp, SlamWrapper.hpp
#      - MarkerTracker.cpp, MarkerTracker.hpp
#      - Папка utils/ с Timer.hpp, Geometry.hpp
#
#    Если у вас имя main-файла другое, поправьте здесь.

# Явно указываем каждую единицу исходников, чтобы CMake легко отслеживал
set(SRC_FILES
        main.cpp
        App.cpp
        SlamWrapper.cpp
        MarkerTracker.cpp
)

# Если у вас внутри src/ есть дополнительные поддиректории (.cpp/.hpp в них),
# можно добавить их сюда или воспользоваться командой file(GLOB ...).
# В данном примере мы знаем конкретные имена, поэтому не используем GLOB.

# 4) Указываем include-пути для нашего таргета
#    + сама папка src (куда сложены .hpp рядом с .cpp)
#    + папка src/utils (где лежат Geometry.hpp, Timer.hpp)
target_include_directories(qr_slam_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}            # чтобы App.hpp, MarkerTracker.hpp были на include path
        ${CMAKE_CURRENT_SOURCE_DIR}/utils      # чтобы Geometry.hpp, Timer.hpp были на include path
)

# 5) Создаём исполняемый таргет qr_slam_demo
add_executable(qr_slam_demo
        ${SRC_FILES}
)

# 6) Линкуем все необходимые внешние библиотеки:
#    – OpenCV   (project-level CMakeLists уже нашёл OpenCV и экспортировал переменную)
#    – Eigen3   (то же самое)
#    – StellaVSLAM (наш Find-модуль вернул импортированный таргет StellaVSLAM::StellaVSLAM)
#
#    Также, если вы хотите, чтобы CMake-логи четко показывали версии, можно прописать:
#
#      find_package(OpenCV   REQUIRED)
#      find_package(Eigen3   REQUIRED)
#      find_package(StellaVSLAM REQUIRED)
#
#    Но лаконичный способ — взять из родительского пространства имён:
target_link_libraries(qr_slam_demo PRIVATE
        ${OpenCV_LIBS}          # автоматически собирает все модули OpenCV (core, highgui, calib3d и т.д.)
        Eigen3::Eigen           # экспортируемая цель для Eigen
        StellaVSLAM::StellaVSLAM
)

# 7) (опционально) если вы хотите включить какие-либо специфичные дефайны или компиля-флаги
#    для Debug/Release — можно здесь прописать. Но обычно это делается на уровне корневого CMakeLists.txt.

# 8) Указываем, какие файлы-заголовки следует «установить» вместе с проектом (если нужен install для dev-пакетов)
#    В данном примере установка таргета qr_slam_demo делается на уровне корня (см. root/CMakeLists.txt).
#    Если вам нужно также устанавливать заголовки (.hpp) куда-либо, добавьте install(FILES ...) здесь.

# Пример (необязательно):
# install(FILES
#     App.hpp
#     SlamWrapper.hpp
#     MarkerTracker.hpp
#     DESTINATION include/qr_slam_demo
# )
