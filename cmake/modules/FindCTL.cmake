# Module to find CTL
#
# This module will first look into the directories defined by the variables:
#   - CTL_PATH, CTL_INCLUDE_PATH, CTL_LIBRARY_PATH
#
# This module defines the following variables:
#
# CTL_FOUND       - True if CTL was found.
# CTL_INCLUDES -    where to find CTL.h
# CTL_LIBRARIES   - list of libraries to link against when using CTL

# Other standarnd issue macros
include (FindPackageHandleStandardArgs)
include (FindPackageMessage)

    if (NOT CTL_FIND_QUIETLY)
        if (CTL_PATH)
            message(STATUS "CTL path explicitly specified: ${CTL_PATH}")
        endif()
        if (CTL_INCLUDE_PATH)
            message(STATUS "CTL INCLUDE_PATH explicitly specified: ${CTL_INCLUDE_PATH}")
        endif()
        if (CTL_LIBRARY_PATH)
            message(STATUS "CTL LIBRARY_PATH explicitly specified: ${CTL_LIBRARY_PATH}")
        endif()
    endif ()
    FIND_PATH(CTL_INCLUDES
        CtlInterpreter.h
        PATHS
        ${CTL_INCLUDE_PATH}
        ${CTL_PATH}/include/
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        PATH_SUFFIXES CTL
        DOC "The directory where CtlInterpreter.h resides")

    set(CTL_ALL_LIBRARIES IlmCtl IlmCtlMath IlmCtlSimd IlmImfCtl)
    foreach(CTL_LIB ${CTL_ALL_LIBRARIES})
      string(TOUPPER ${CTL_LIB} _upper_ctl_lib)
      find_library(CTL_${_upper_ctl_lib}_LIBRARY
                   NAMES ${CTL_LIB} lib${CTL_LIB}
                   PATHS
                   ${CTL_LIBRARY_PATH}
                   ${CTL_PATH}/lib/
                   /usr/lib64
                   /usr/lib
                   /usr/local/lib64
                   /usr/local/lib
                   /sw/lib
                   /opt/local/lib
                   DOC "The CTL library"
      )
      if(CTL_${_upper_ctl_lib}_LIBRARY)
        set(CTL_LIBRARIES ${CTL_LIBRARIES} ${CTL_${_upper_ctl_lib}_LIBRARY})
        mark_as_advanced(CTL_${_upper_ctl_lib}_LIBRARY)
      endif()
    endforeach()

    if(CTL_INCLUDES AND CTL_LIBRARIES)
        set(CTL_FOUND TRUE)
        if (NOT CTL_FIND_QUIETLY)
            message(STATUS "Found CTL library ${CTL_LIBRARIES}")
            message(STATUS "Found CTL includes ${CTL_INCLUDES}")
        endif ()
    else()
        set(CTL_FOUND FALSE)
        message(STATUS "CTL not found. Specify CTL_PATH to locate it")
    endif()

