###################################################################################################
# THE OMICRON PROJECT
#-------------------------------------------------------------------------------------------------
# Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
# Authors:										
#  Alessandro Febretti		febret@gmail.com
#-------------------------------------------------------------------------------------------------
# Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
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
set_property(GLOBAL PROPERTY USE_FOLDERS On)
cmake_minimum_required(VERSION 2.8.1) 

project(OmicronSDK)

include(CPack)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
      #cmake_policy(SET CMP0008 NEW)
endif()

###################################################################################################
# Create a build name depending on the OS, Toolset and architecture.
if(CMAKE_GENERATOR STREQUAL "Visual Studio 9 2008")
	set(OMICRON_OS_WIN 1)
	set(OMICRON_ARCH_32 1)
	set(OMICRON_ARCH_WIN32 1)
	set(OMICRON_TOOL_VS9 1)
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 9 2008 Win64")
	set(OMICRON_OS_WIN 1)
	set(OMICRON_ARCH_64 1)
	set(OMICRON_ARCH_WIN32 1)
	set(OMICRON_TOOL_VS9 1)
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 10")
	set(OMICRON_OS_WIN 1)
	set(OMICRON_ARCH_32 1)
	set(OMICRON_ARCH_WIN32 1)
	set(OMICRON_TOOL_VS10 1)
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 10 Win64")
	set(OMICRON_OS_WIN 1)
	set(OMICRON_ARCH_64 1)
	set(OMICRON_ARCH_WIN32 1)
	set(OMICRON_TOOL_VS10 1)
elseif(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
	if(APPLE)
		set(OMICRON_OS_OSX 1)
		set(OMICRON_ARCH_32 1)
		set(OMICRON_ARCH_I386 1)
		set(OMICRON_TOOL_GCC 1)
	else()
		if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
			set(OMICRON_OS_LINUX 1)
			set(OMICRON_ARCH_64 1)
			set(OMICRON_ARCH_AMD64 1)
			set(OMICRON_TOOL_GCC 1)
		else()
			set(OMICRON_OS_LINUX 1)
			set(OMICRON_ARCH_32 1)
			set(OMICRON_ARCH_I386 1)
			set(OMICRON_TOOL_GCC 1)
		endif()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl -lrt -pthread")	
	endif()
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	set(OMICRON_OS_OSX 1)
	set(OMICRON_ARCH_32 1)
	set(OMICRON_ARCH_I386 1)
	set(OMICRON_TOOL_GCC 1)
endif()

set(OMICRON_DATA_PATH "${CMAKE_SOURCE_DIR}/data")

###################################################################################################
# Set the output directories for libraries and binary files
if(OMICRON_USE_CUSTOM_OUTPUT)
message("set custom output")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OMICRON_LIBRARY_OUTPUT_DIRECTORY})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OMICRON_ARCHIVE_OUTPUT_DIRECTORY})
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OMICRON_RUNTIME_OUTPUT_DIRECTORY})
		
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OMICRON_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OMICRON_LIBRARY_OUTPUT_DIRECTORY_RELEASE})
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OMICRON_LIBRARY_OUTPUT_DIRECTORY_RELEASE})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OMICRON_ARCHIVE_OUTPUT_DIRECTORY_DEBUG})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OMICRON_ARCHIVE_OUTPUT_DIRECTORY_RELEASE})
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OMICRON_ARCHIVE_OUTPUT_DIRECTORY_RELEASE})
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OMICRON_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OMICRON_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OMICRON_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
else()
message("NO set custom output")
	if(MSVC OR CMAKE_GENERATOR STREQUAL "Xcode")
		# Since visual studio builds are multiconfiguration, set two separate directories for debug and release builds
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/debug)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/release)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
		# Make sure the output bin directories exist, so the cmake scripts can copy 
		# third party dlls there if needed.
		file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
		file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
	else()
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
		file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	endif()
endif()
###################################################################################################
# Visual studio specific options.
if(MSVC)
	# Exclude libcmt when linking in visual studio
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4018)
endif(MSVC)

###################################################################################################
# Add subdirectiories
add_subdirectory(src)

