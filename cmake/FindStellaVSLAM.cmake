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
#     STELLAVSLAM_INCLUDE_DIR    полный_путь_до_заголовков
#     STELLAVSLAM_LIBRARY        полный_путь_до_библиотеки
#     STELLAVSLAM_VERSION        X.Y.Z
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
include(CMakeParseArguments)

# ---------- Поддерживаемые переменные окружения / кэш -------------
# Можно задать StellaVSLAM_ROOT=/opt/stella_vslam, или
# STELLAVSLAM_ROOT, либо вручную STELLAVSLAM_INCLUDE_DIR/STELLAVSLAM_LIBRARY.

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

# ---------- Поиск заголовков (openvslam/system.h)  -------------------
if(NOT STELLAVSLAM_INCLUDE_DIR AND NOT _STELLAVSLAM_HINTS STREQUAL "")
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES  openvslam/system.h
            HINTS  ${_STELLAVSLAM_HINTS}
            PATH_SUFFIXES include include/openvslam include/stella_vslam)
endif()

# Fallback: системные пути (/usr/include, /usr/local/include)
if(NOT STELLAVSLAM_INCLUDE_DIR)
    find_path(STELLAVSLAM_INCLUDE_DIR
            NAMES  openvslam/system.h
            PATH_SUFFIXES openvslam stella_vslam)
endif()

# ---------- Поиск библиотеки (libstella_vslam.so/.a) -----------------
if(NOT STELLAVSLAM_LIBRARY AND NOT _STELLAVSLAM_HINTS STREQUAL "")
    find_library(STELLAVSLAM_LIBRARY
            NAMES  stella_vslam openvslam
            HINTS  ${_STELLAVSLAM_HINTS}
            PATH_SUFFIXES lib lib64)
endif()

# Fallback: системные пути (/usr/lib, /usr/local/lib)
if(NOT STELLAVSLAM_LIBRARY)
    find_library(STELLAVSLAM_LIBRARY
            NAMES stella_vslam openvslam
            PATH_SUFFIXES lib lib64)
endif()

# ---------- Извлечение версии из version.h (если есть) -------------
if(STELLAVSLAM_INCLUDE_DIR AND EXISTS "${STELLAVSLAM_INCLUDE_DIR}/openvslam/version.h")
    file(STRINGS "${STELLAVSLAM_INCLUDE_DIR}/openvslam/version.h"
            _ver_line REGEX "#define +OPENVSLAM_VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+)\"")
    if(_ver_line)
        string(REGEX REPLACE ".*\"([0-9.]+)\".*" "\\1" STELLAVSLAM_VERSION "${_ver_line}")
    endif()
endif()

# ---------- Создание импортированной цели ----------------------------
if(STELLAVSLAM_INCLUDE_DIR AND STELLAVSLAM_LIBRARY AND NOT TARGET StellaVSLAM::StellaVSLAM)
    add_library(StellaVSLAM::StellaVSLAM UNKNOWN IMPORTED)
    set_target_properties(StellaVSLAM::StellaVSLAM PROPERTIES
            IMPORTED_LOCATION             "${STELLAVSLAM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES  "${STELLAVSLAM_INCLUDE_DIR}"
    )
endif()

# ---------- Результат find_package ------------------------------------
find_package_handle_standard_args(StellaVSLAM
        REQUIRED_VARS STELLAVSLAM_LIBRARY STELLAVSLAM_INCLUDE_DIR
        VERSION_VAR   STELLAVSLAM_VERSION
)

mark_as_advanced(STELLAVSLAM_INCLUDE_DIR STELLAVSLAM_LIBRARY STELLAVSLAM_ROOT)
