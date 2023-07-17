#
# Find the GEOMETRYKERNEL includes and library
#
# This module uses
# GEOMETRYKERNEL_ROOT
#
# This module defines
# GEOMETRYKERNEL_FOUND
# GEOMETRYKERNEL_INCLUDE_DIRS
# GEOMETRYKERNEL_LIBRARIES
#
# Target geometry-kernel

if(NOT GEOMETRYKERNEL_ROOT)
  set(GEOMETRYKERNEL_ROOT $ENV{GEOMETRYKERNEL_ROOT})
endif()

if(GEOMETRYKERNEL_ROOT)
  set(_GEOMETRYKERNEL_SEARCH_OPTS NO_DEFAULT_PATH)
else()
  set(_GEOMETRYKERNEL_SEARCH_OPTS)
endif()

if(NOT GEOMETRYKERNEL_FOUND)

  unset(GEOMETRYKERNEL_LIBRARY CACHE)
  unset(GEOMETRYKERNEL_INCLUDE_DIR CACHE)

  set(libname GeometryKernel)
  if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)
	set(libname GeometryKernelD)
  endif()
  
  find_library(GEOMETRYKERNEL_LIBRARY 
    NAMES ${libname}
    HINTS ${GEOMETRYKERNEL_ROOT} 
    PATH_SUFFIXES lib bin/${CMAKE_BUILD_TYPE}
    ${_GEOMETRYKERNEL_SEARCH_OPTS}
    )
  mark_as_advanced(GEOMETRYKERNEL_LIBRARY)

  find_path(GEOMETRYKERNEL_INCLUDE_DIR GeometryKernel
    HINTS ${GEOMETRYKERNEL_ROOT} 
    PATH_SUFFIXES include
    ${_GEOMETRYKERNEL_SEARCH_OPTS}
    )
  mark_as_advanced(GEOMETRYKERNEL_INCLUDE_DIR)
  
endif()

# pour limiter le mode verbose
set(GEOMETRYKERNEL_FIND_QUIETLY ON)

find_package_handle_standard_args(GEOMETRYKERNEL 
  DEFAULT_MSG 
  GEOMETRYKERNEL_INCLUDE_DIR 
  GEOMETRYKERNEL_LIBRARY)

if(GEOMETRYKERNEL_FOUND AND NOT TARGET geometrykernel)

  set(GEOMETRYKERNEL_INCLUDE_DIRS ${GEOMETRYKERNEL_INCLUDE_DIR})

  set(GEOMETRYKERNEL_LIBRARIES ${GEOMETRYKERNEL_LIBRARY})

  add_library(geometrykernel UNKNOWN IMPORTED)
    
  set_target_properties(geometrykernel PROPERTIES 
    INTERFACE_INCLUDE_DIRECTORIES "${GEOMETRYKERNEL_INCLUDE_DIRS}")
    
  set_target_properties(geometrykernel PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${GEOMETRYKERNEL_LIBRARY}")

  # pour historique   
  add_library(geometry-kernel INTERFACE IMPORTED)
  
  set_property(TARGET geometry-kernel APPEND PROPERTY 
    INTERFACE_LINK_LIBRARIES "geometrykernel")
 
endif()