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
set(OMICRON_BUILD_EXAMPLES true CACHE BOOL "Enable building of omicron examples.")

###############################################################################
# Set include paths
include_directories(
  ${CMAKE_BINARY_DIR}/include/
  ${CMAKE_SOURCE_DIR}/include/
  ${CMAKE_SOURCE_DIR}/external/include
  )

# Add connector examples before including pch support, so connector examples won't depend on omicron precompiled headers.
add_subdirectory(apps/connectorClient)

# From this point on, enabl pch support.
include(UsePch)

###############################################################################
# Add libraries and applications to the project.
# TODO: should this be moved inside then omega OPENNI module block?
if(WIN32)
	include_directories($ENV{PROGRAMFILES}/OpenNI/Include)
endif(WIN32)

# Add the core omega project
add_subdirectory(omicron)

# oinputserver currently works only on Windows (uses WINSOCK)
if(WIN32)
	add_subdirectory(apps/sageinputserver)
	add_subdirectory(apps/legacy-oinputserver)
endif(WIN32)
add_subdirectory(apps/oinputserver)
add_subdirectory(apps/eventlogger)
add_subdirectory(apps/ocachesync)
add_subdirectory(apps/ocachesrv)

if(OMICRON_USE_SOUND)
	add_subdirectory(apps/soundtest)
endif(OMICRON_USE_SOUND)

if(OMICRON_BUILD_EXAMPLES)
	omicron_add_example(localService)
	omicron_add_example(remoteService)
endif(OMICRON_BUILD_EXAMPLES)

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/UseOmicron.cmake.in ${CMAKE_BINARY_DIR}/UseOmicron.cmake)

