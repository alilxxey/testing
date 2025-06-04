# ----------------------------------------------------------------------
#  FindStellaVSLAM.cmake
#
#  Ищет установленную библиотеку Stella-/OpenVSLAM (StellaVSLAM) и создаёт
#  IMPORTED-таргет StellaVSLAM::StellaVSLAM
#
#  Пользователь может задать:
#     • StellaVSLAM_ROOT или STELLAVSLAM_ROOT    — абсолютный префикс установки
#     • STELLAVSLAM_INCLUDE_DIR / STELLAVSLAM_LIBRARY  — явные пути
#
#  Выставляются следующие переменные (public):
#     STELLAVSLAM_FOUND          TRUE / FALSE
#     STELLAVSLAM_INCLUDE_DIR    полный_путь_до_директории_с_заголовками
#     STELLAVSLAM_LIBRARY        полный_путь_до_файла_библиотеки
#     STELLAVSLAM_VERSION        X.Y.Z    (если удалось прочитать openvslam/version.h)
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

# ----------------------------------------------------------------------------
#   Поддержка переменных окружения / кэша:
#     • StellaVSLAM_ROOT / STELLAVSLAM_ROOT    — префикс (например, /usr/local или /opt/stella_vslam)
#     • STELLAVSLAM_INCLUDE_DIR                 — полный путь до каталога с system.h
#     • STELLAVSLAM_LIBRARY                     — полный путь до .so (или .a)
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
#   1) Поиск заголовков: пробуем несколько вариантов
#      – openvslam/system.h   (классический путь для OpenVSLAM)
#      – stella_vslam/system.h (новый путь в community-ветке StellaVSLAM)
# ----------------------------------------------------------------------------

if(NOT STELLAVSLAM_INCLUDE_DIR AND NOT _STELLAVSLAM_HINTS STREQUAL "")
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES
            openvslam/system.h
            stella_vslam/system.h
            HINTS ${_STELLAVSLAM_HINTS}
            PATH_SUFFIXES
            include
            include/openvslam
            include/stella_vslam
            include/stella_vslam/openvslam
    )
endif()

if(NOT STELLAVSLAM_INCLUDE_DIR)
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES
            openvslam/system.h
            stella_vslam/system.h
            PATH_SUFFIXES
            include
            include/openvslam
            include/stella_vslam
            include/stella_vslam/openvslam
            openvslam
            stella_vslam/openvslam
            stella_vslam
    )
endif()

# ----------------------------------------------------------------------------
#   2) Поиск библиотеки: пробуем
#      – libstella_vslam.so (обычно)
#      – libopenvslam.so      (альтернатива)
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
#   3) Читаем версию (если version.h найдён)
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
#   4) Создаём импортированный таргет StellaVSLAM::StellaVSLAM
# ----------------------------------------------------------------------------

if(STELLAVSLAM_INCLUDE_DIR AND STELLAVSLAM_LIBRARY AND NOT TARGET StellaVSLAM::StellaVSLAM)
    add_library(StellaVSLAM::StellaVSLAM UNKNOWN IMPORTED)
    set_target_properties(StellaVSLAM::StellaVSLAM PROPERTIES
            IMPORTED_LOCATION             "${STELLAVSLAM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES  "${STELLAVSLAM_INCLUDE_DIR}"
    )
endif()

# ----------------------------------------------------------------------------
#   5) Итог: проверяем, что нашли и include, и библиотеку
# ----------------------------------------------------------------------------

find_package_handle_standard_args(StellaVSLAM
        REQUIRED_VARS STELLAVSLAM_LIBRARY STELLAVSLAM_INCLUDE_DIR
        VERSION_VAR   STELLAVSLAM_VERSION
)

mark_as_advanced(STELLAVSLAM_INCLUDE_DIR STELLAVSLAM_LIBRARY STELLAVSLAM_ROOT)
