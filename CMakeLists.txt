cmake_minimum_required(VERSION 3.0)
project(ELook VERSION 0.2.3)


# -----------------------------------------------------------------------------
# Settings
# -----------------------------------------------------------------------------

option(USE_CCACHE "Use ccache if found" ON)

# -----------------------------------------------------------------------------
# Global
# -----------------------------------------------------------------------------

set(QT_PREFIX /usr/local/opt/qt)
set(LLVM_ROOT /usr/local/opt/llvm)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_PREFIX_PATH ${QT_PREFIX}/lib/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++")
set(CMAKE_CXX_FLAGS "-fcolor-diagnostics -Wall -Werror -Wfatal-errors -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fno-omit-frame-pointer -Wno-unused-variable -Wno-unused-private-field -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-L${LLVM_ROOT}/lib")

set(CXX_FILESYSTEM_LIB ${LLVM_ROOT}/lib/libc++fs.a)

#set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk")
#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# -----------------------------------------------------------------------------
# Build
# -----------------------------------------------------------------------------

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/ext/dist")
set(CMAKE_BUILD_RPATH "${INSTALL_DIR}/lib")
add_subdirectory(ext)

add_subdirectory(src)

# -----------------------------------------------------------------------------
# Packaging
# -----------------------------------------------------------------------------

add_custom_target(bundle
    COMMAND ${QT_PREFIX}/bin/macdeployqt src/ELook.app -dmg
    DEPENDS ${PROJECT_NAME}
)

# -----------------------------------------------------------------------------
# Checks
# -----------------------------------------------------------------------------

set(RUN_CLANG_TIDY_BIN ${LLVM_ROOT}/share/clang/run-clang-tidy.py)

add_custom_target(
    tidy
    COMMAND ${RUN_CLANG_TIDY_BIN}
    COMMENT "Running clang tidy"
)