set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
include(ExternalProject)
include(OmicronAddExample)


# make sure the inlcude directory exists before extracting Boost, Asio and Eigen.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/)

include(${CMAKE_SOURCE_DIR}/external/UseBoost.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseAsio.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseEigen.cmake)

###############################################################################
# Options
set(OMICRON_BUILD_EXAMPLES false CACHE BOOL "Enable building of omicron examples.")
set(OMICRON_BUILD_APPS false CACHE BOOL "Enable building of additional omicron apps.")

###############################################################################
# Set include paths
include_directories(
  ${CMAKE_BINARY_DIR}/include/
  ${CMAKE_SOURCE_DIR}/include/
  ${CMAKE_SOURCE_DIR}/external/include
  )

# Add connector examples before including pch support, so connector examples won't depend on omicron precompiled headers.
if(OMICRON_BUILD_EXAMPLES)
    add_subdirectory(apps/connectorClient)
endif()

# From this point on, enabl pch support.
include(UsePch)

###############################################################################
# Add libraries and applications to the project.
# TODO: should this be moved inside then omega OPENNI module block?
if(WIN32)
	include_directories($ENV{PROGRAMFILES}/OpenNI/Include)
endif()

# Add the core omega project
add_subdirectory(omicron)

# oinputserver currently works only on Windows (uses WINSOCK)
if(WIN32)
    if(OMICRON_BUILD_APPS)
        add_subdirectory(apps/sageinputserver)
        add_subdirectory(apps/legacy-oinputserver)
    endif()
endif()
if(OMICRON_BUILD_APPS)
    add_subdirectory(apps/oinputserver)
    add_subdirectory(apps/eventlogger)
    add_subdirectory(apps/ocachesync)
    add_subdirectory(apps/ocachesrv)
endif()

if(OMICRON_USE_SOUND AND OMICRON_BUILD_EXAMPLES)
	add_subdirectory(apps/soundtest)
endif()

if(OMICRON_BUILD_EXAMPLES)
	omicron_add_example(localService)
	omicron_add_example(remoteService)
endif()

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/UseOmicron.cmake.in ${CMAKE_BINARY_DIR}/UseOmicron.cmake)

