# ----------------------------------------------------------------------
#  version.cmake  —  генерация PROJECT_VERSION из git-тегов
#
#  1. Если родительский CMake уже установил PROJECT_VERSION → ничего не делаем
#  2. Иначе пытаемся `git describe --tags --dirty`
#  3. Если git недоступен — ставим 0.0.0
#
#  Доступные переменные после include():
#     PROJECT_VERSION         "1.2.3"
#     PROJECT_VERSION_MAJOR   1
#     PROJECT_VERSION_MINOR   2
#     PROJECT_VERSION_PATCH   3
#
#  © 2025 YourCompany — MIT License
# ----------------------------------------------------------------------

if(DEFINED PROJECT_VERSION AND NOT "${PROJECT_VERSION}" STREQUAL "")
    message(STATUS "[version] using preset PROJECT_VERSION=${PROJECT_VERSION}")
    return()
endif()

# ---------- try git describe ------------------------------------------
execute_process(
        COMMAND git describe --tags --dirty
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_ver
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)

if(NOT _git_ver)
    set(_git_ver "0.0.0")
endif()

# drop leading 'v', remove -dirty/-g<hash>
string(REGEX REPLACE "^v" "" _git_ver "${_git_ver}")
string(REGEX REPLACE "-.*$" "" _git_ver "${_git_ver}")

# validate regex X[.Y[.Z]]
if(NOT _git_ver MATCHES "^[0-9]+\\.[0-9]+(\\.[0-9]+)?$")
    message(WARNING "[version] invalid git tag '${_git_ver}', fallback to 0.0.0")
    set(_git_ver "0.0.0")
endif()

set(PROJECT_VERSION "${_git_ver}" CACHE INTERNAL "Project version" FORCE)

# ---------- split parts -----------------------------------------------
string(REPLACE "." ";" _vers "${PROJECT_VERSION}")
list(GET _vers 0 PROJECT_VERSION_MAJOR)
list(GET _vers 1 PROJECT_VERSION_MINOR)
if(_vers AND (list(LENGTH _vers) GREATER 2))
    list(GET _vers 2 PROJECT_VERSION_PATCH)
else()
    set(PROJECT_VERSION_PATCH 0)
endif()

message(STATUS "[version] PROJECT_VERSION=${PROJECT_VERSION}")
