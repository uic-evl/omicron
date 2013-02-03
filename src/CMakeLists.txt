###################################################################################################
# THE OMICRON PROJECT
#-------------------------------------------------------------------------------------------------
# Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
# Authors:										
#  Alessandro Febretti		febret@gmail.com
#-------------------------------------------------------------------------------------------------
# Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted 
# provided that the following conditions are met:
# 
# Redistributions of source code must retain the above copyright notice, this list of conditions 
# and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the documentation and/or other 
# materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###################################################################################################
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
include(ExternalProject)
include(OmicronAddExample)


# make sure the inlcude directory exists before extracting Boost, Asio and Eigen.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/)

include(${CMAKE_SOURCE_DIR}/external/UseBoost.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseAsio.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseEigen.cmake)

###################################################################################################
# Options
set(OMICRON_BUILD_EXAMPLES true CACHE BOOL "Enable building of omicron examples.")

###################################################################################################
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

###################################################################################################
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
add_subdirectory(apps/soundtest)
add_subdirectory(apps/eventlogger)

if(OMICRON_BUILD_EXAMPLES)
	omicron_add_example(localService)
	omicron_add_example(remoteService)
endif(OMICRON_BUILD_EXAMPLES)

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/UseOmicron.cmake.in ${CMAKE_BINARY_DIR}/UseOmicron.cmake)

