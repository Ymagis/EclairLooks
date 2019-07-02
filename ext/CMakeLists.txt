include(ExternalProject)

set(COMMON_CMAKE_ARGS
    -DCMAKE_CXX_STANDARD=11
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
    -DPKG_CONFIG_USE_CMAKE_PREFIX_PATH=ON
    -DCMAKE_CXX_COMPILER=${LLVM_ROOT}/bin/clang++
    -DCMAKE_EXE_LINKER_FLAGS=-L${LLVM_ROOT}/lib
    -DCMAKE_SHARED_LINKER_FLAGS=-L${LLVM_ROOT}/lib
)

# Use ccache if found
find_program (CCACHE_FOUND ccache)
if (CCACHE_FOUND AND USE_CCACHE)
    set_property(DIRECTORY PROPERTY RULE_LAUNCH_COMPILE "ccache")
    set_property(DIRECTORY PROPERTY RULE_LAUNCH_LINK "ccache")
endif ()

# ----------------------------------------------------------------------------
# OpenEXR

add_library(openexr INTERFACE IMPORTED GLOBAL)

set(OPENEXR_INCLUDE_DIRS "${INSTALL_DIR}/include/OpenEXR")
set(OPENEXR_LIBRARY
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Half${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Iex${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IexMath${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmThread${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Imath${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmImf${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmImfUtil${CMAKE_SHARED_LIBRARY_SUFFIX}
)

set_property(TARGET openexr PROPERTY INTERFACE_LINK_LIBRARIES ${OPENEXR_LIBRARY})
set_property(TARGET openexr PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENEXR_INCLUDE_DIRS})
file(MAKE_DIRECTORY ${OPENEXR_INCLUDE_DIRS}) # Hack to let imported target be built from ExternalProject_Add

set(OPENEXR_CMAKE_ARGS
    ${COMMON_CMAKE_ARGS}
    -DOPENEXR_NAMESPACE_VERSIONING=OFF
    -DOPENEXR_BUILD_PYTHON_LIBS=OFF
    -DOPENEXR_BUILD_TESTS=OFF
    -DOPENEXR_BUILD_UTILS=OFF
)

ExternalProject_Add(OPENEXR
    URL https://github.com/openexr/openexr/archive/v2.3.0.tar.gz
    BINARY_DIR "${CMAKE_BINARY_DIR}/ext/build/openexr"
    BUILD_BYPRODUCTS "${OPENEXR_LIBRARY}"
    INSTALL_DIR "${INSTALL_DIR}"
    CMAKE_ARGS ${OPENEXR_CMAKE_ARGS}
)

ExternalProject_Add_Step(OPENEXR preconfig
    COMMAND mkdir -p <BINARY_DIR>/OpenEXR/IlmImf
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/b44ExpLogTable.h <BINARY_DIR>/OpenEXR/IlmImf
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/dwaLookups.h <BINARY_DIR>/OpenEXR/IlmImf
    DEPENDERS update
    DEPENDEES download
)

add_dependencies(openexr OPENEXR)

# ----------------------------------------------------------------------------
# CTL

add_library(ctl INTERFACE IMPORTED GLOBAL)

set(CTL_INCLUDE_DIRS "${INSTALL_DIR}/include/CTL")
set(CTL_LIBRARY
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmCtl${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmCtlMath${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmCtlSimd${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}IlmImfCtl${CMAKE_SHARED_LIBRARY_SUFFIX}
)

set_property(TARGET ctl PROPERTY INTERFACE_LINK_LIBRARIES ${CTL_LIBRARY})
set_property(TARGET ctl PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CTL_INCLUDE_DIRS})
file(MAKE_DIRECTORY ${CTL_INCLUDE_DIRS}) # Hack to let imported target be built from ExternalProject_Add

ExternalProject_Add(CTL
    DEPENDS OPENEXR
    URL https://github.com/ampas/CTL/archive/ctl-1.5.2.tar.gz
    BINARY_DIR "${CMAKE_BINARY_DIR}/ext/build/ctl"
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-compatibility-with-ilmbase-2.3.patch
          COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-dpx-lib-cxx-17-compilation.patch
          COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/0003-cmake-3-macos_rpath-compliance.patch
    BUILD_BYPRODUCTS "${CTL_LIBRARY}"
    INSTALL_DIR "${INSTALL_DIR}"
    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
)

add_dependencies(ctl CTL)

# ----------------------------------------------------------------------------
# OpenImageIO

add_library(openimageio INTERFACE IMPORTED GLOBAL)

set(OIIO_INCLUDE_DIRS
    "${INSTALL_DIR}/include"
    "${INSTALL_DIR}/include/OpenImageIO")
set(OIIO_LIBRARY
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}OpenImageIO${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}OpenImageIO_Util${CMAKE_SHARED_LIBRARY_SUFFIX}
)

set_property(TARGET openimageio PROPERTY INTERFACE_LINK_LIBRARIES ${OIIO_LIBRARY})
set_property(TARGET openimageio PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OIIO_INCLUDE_DIRS})
file(MAKE_DIRECTORY ${OIIO_INCLUDE_DIRS}) # Hack to let imported target be built from ExternalProject_Add

set(OIIO_CMAKE_ARGS
    ${COMMON_CMAKE_ARGS}
    -DOPENEXR_ROOT_DIR=${INSTALL_DIR}
    -DUSE_FIELD3D=OFF
    -DOIIO_BUILD_TESTS=OFF
    -DUSE_PYTHON=OFF
    -DUSE_OPENGL=OFF
    -DUSE_QT=OFF
    -DUSE_OPENCV=OFF
)

ExternalProject_Add(OIIO
    DEPENDS OPENEXR
    URL https://github.com/OpenImageIO/oiio/archive/Release-2.0.7.tar.gz
    BINARY_DIR "${CMAKE_BINARY_DIR}/ext/build/oiio"
    BUILD_BYPRODUCTS "${OIIO_LIBRARY}"
    INSTALL_DIR "${INSTALL_DIR}"
    CMAKE_ARGS ${OIIO_CMAKE_ARGS}
)

add_dependencies(openimageio OIIO)

# ----------------------------------------------------------------------------
# OpenColorIO

add_library(opencolorio INTERFACE IMPORTED GLOBAL)

set(OCIO_INCLUDE_DIRS "${INSTALL_DIR}/include/OpenColorIO")
set(OCIO_LIBRARY
    ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}OpenColorIO${CMAKE_SHARED_LIBRARY_SUFFIX}
)

set_property(TARGET opencolorio PROPERTY INTERFACE_LINK_LIBRARIES ${OCIO_LIBRARY})
set_property(TARGET opencolorio PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OCIO_INCLUDE_DIRS})
file(MAKE_DIRECTORY ${OCIO_INCLUDE_DIRS}) # Hack to let imported target be built from ExternalProject_Add

set(OCIO_CMAKE_ARGS
    ${COMMON_CMAKE_ARGS}
    -DOCIO_BUILD_APPS=OFF
    -DOCIO_BUILD_NUKE=OFF
    -DOCIO_BUILD_TESTS=OFF
    -DOCIO_BUILD_GPU_TESTS=OFF
    -DOCIO_BUILD_PYTHON=OFF
)

ExternalProject_Add(OCIO
    DEPENDS OIIO
    URL https://github.com/imageworks/OpenColorIO/archive/8deb32f33e28ca8435a6c78a356937a057faad50.zip
    BINARY_DIR "${CMAKE_BINARY_DIR}/ext/build/ocio"
    BUILD_BYPRODUCTS "${OCIO_LIBRARY}"
    INSTALL_DIR "${INSTALL_DIR}"
    CMAKE_ARGS ${OCIO_CMAKE_ARGS}
)

add_dependencies(opencolorio OCIO)
