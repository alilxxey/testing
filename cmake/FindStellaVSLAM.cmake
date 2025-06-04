# ----------------------------------------------------------------------
#  FindStellaVSLAM.cmake
#
#  Ищет установленную библиотеку Stella-/OpenVSLAM и создаёт
#  импортированную цель StellaVSLAM::StellaVSLAM
#
#  Пользователь может задать:
#     • StellaVSLAM_ROOT или STELLAVSLAM_ROOT    — абсолютный префикс
#     • STELLAVSLAM_INCLUDE_DIR / STELLAVSLAM_LIBRARY  — явные пути
#
#  Найденные переменные (public):
#     STELLAVSLAM_FOUND          TRUE / FALSE
#     STELLAVSLAM_INCLUDE_DIR    полный_путь_до_директории_с_заголовками
#     STELLAVSLAM_LIBRARY        полный_путь_до_файла_библиотеки
#     STELLAVSLAM_VERSION        X.Y.Z
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

# ----------------------------------------------------------------------------
#  Поддержка переменных окружения / кэша:
#    • StellaVSLAM_ROOT  / STELLAVSLAM_ROOT  — префикс установки
#    • STELLAVSLAM_INCLUDE_DIR               — полный путь до папки, содержащей openvslam/system.h
#    • STELLAVSLAM_LIBRARY                   — полный путь до libstella_vslam.so или libopenvslam.so
# ----------------------------------------------------------------------------

set(_STELLAVSLAM_HINTS "")
if(DEFINED ENV{StellaVSLAM_ROOT})
    list(APPEND _STELLAVSLAM_HINTS $ENV{StellaVSLAM_ROOT})
endif()
if(DEFINED ENV{stellavslam_ROOT})
    list(APPEND _STELLAVSLAM_HINTS $ENV{stellavslam_ROOT})
endif()
if(DEFINED StellaVSLAM_ROOT)
    list(APPEND _STELLAVSLAM_HINTS ${StellaVSLAM_ROOT})
endif()
if(DEFINED STELLAVSLAM_ROOT)
    list(APPEND _STELLAVSLAM_HINTS ${STELLAVSLAM_ROOT})
endif()

# ----------------------------------------------------------------------------
#  1) Поиск заголовков openvslam/system.h
#
#  Мы ожидаем, что после `sudo make install` у вас появился каталог:
#
#    /usr/local/include/stella_vslam/openvslam/system.h
#
#  Поэтому пробуем:
#    - HINTS:  <StellaVSLAM_ROOT>/include, <StellaVSLAM_ROOT>/include/stella_vslam, <StellaVSLAM_ROOT>/include/stella_vslam/openvslam
#    - Fallback: системные пути (/usr/local/include, /usr/include) + все подпапки stella_vslam/openvslam
# ----------------------------------------------------------------------------

if(NOT STELLAVSLAM_INCLUDE_DIR AND NOT _STELLAVSLAM_HINTS STREQUAL "")
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES  openvslam/system.h
            HINTS  ${_STELLAVSLAM_HINTS}
            PATH_SUFFIXES
            include
            include/openvslam
            include/stella_vslam
            include/stella_vslam/openvslam
    )
endif()

if(NOT STELLAVSLAM_INCLUDE_DIR)
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES  openvslam/system.h
            PATH_SUFFIXES
            include
            openvslam
            stella_vslam/openvslam
            stella_vslam
    )
endif()

# ----------------------------------------------------------------------------
#  2) Поиск библиотеки libstella_vslam.so  (или libopenvslam.so)
#
#  Ожидаем, что после установки библиотека находится в:
#    /usr/local/lib/libstella_vslam.so
#
#  Поэтому ищем:
#    - HINTS:  <StellaVSLAM_ROOT>/lib, <StellaVSLAM_ROOT>/lib64, <StellaVSLAM_ROOT>/lib/stella_vslam
#    - Fallback: системные пути (/usr/local/lib, /usr/lib) + подпапки lib/stella_vslam
# ----------------------------------------------------------------------------

if(NOT STELLAVSLAM_LIBRARY AND NOT _STELLAVSLAM_HINTS STREQUAL "")
    find_library(STELLAVSLAM_LIBRARY
            NAMES  stella_vslam openvslam
            HINTS  ${_STELLAVSLAM_HINTS}
            PATH_SUFFIXES
            lib
            lib64
            lib/stella_vslam
            lib64/stella_vslam
    )
endif()

if(NOT STELLAVSLAM_LIBRARY)
    find_library(STELLAVSLAM_LIBRARY
            NAMES stella_vslam openvslam
            PATH_SUFFIXES
            lib
            lib64
            lib/stella_vslam
            lib64/stella_vslam
    )
endif()

# ----------------------------------------------------------------------------
#  3) Извлечение версии OPENVSLAM из заголовка version.h (если есть)
# ----------------------------------------------------------------------------

if(STELLAVSLAM_INCLUDE_DIR AND EXISTS "${STELLAVSLAM_INCLUDE_DIR}/openvslam/version.h")
    file(STRINGS "${STELLAVSLAM_INCLUDE_DIR}/openvslam/version.h"
            _ver_line
            REGEX "#define +OPENVSLAM_VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+)\"")
    if(_ver_line)
        string(REGEX REPLACE ".*\"([0-9.]+)\".*" "\\1" STELLAVSLAM_VERSION "${_ver_line}")
    endif()
endif()

# ----------------------------------------------------------------------------
#  4) Создание импортированного target’а StellaVSLAM::StellaVSLAM
# ----------------------------------------------------------------------------

if(STELLAVSLAM_INCLUDE_DIR AND STELLAVSLAM_LIBRARY AND NOT TARGET StellaVSLAM::StellaVSLAM)
    add_library(StellaVSLAM::StellaVSLAM UNKNOWN IMPORTED)
    set_target_properties(StellaVSLAM::StellaVSLAM PROPERTIES
            IMPORTED_LOCATION             "${STELLAVSLAM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES  "${STELLAVSLAM_INCLUDE_DIR}"
    )
endif()

# ----------------------------------------------------------------------------
#  5) Завершение и проверка:  если не найден ни include, ни library — выдаём ошибку
# ----------------------------------------------------------------------------

find_package_handle_standard_args(StellaVSLAM
        REQUIRED_VARS STELLAVSLAM_LIBRARY STELLAVSLAM_INCLUDE_DIR
        VERSION_VAR   STELLAVSLAM_VERSION
)

mark_as_advanced(STELLAVSLAM_INCLUDE_DIR STELLAVSLAM_LIBRARY STELLAVSLAM_ROOT)
