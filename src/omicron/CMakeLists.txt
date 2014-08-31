###############################################################################
# Compile definitions
add_definitions(-DOMICRON_EXPORTING)

###############################################################################
# Source files
SET( srcs 
		omicron/AssetCacheManager.cpp
		omicron/AssetCacheService.cpp
		omicron/ByteArray.cpp
        omicron/osystem.cpp
		omicron/Config.cpp
		omicron/Math.cpp
		omicron/ServiceManager.cpp
		omicron/Service.cpp
		omicron/HeartbeatService.cpp
		omicron/StringUtils.cpp
		omicron/DataManager.cpp
		omicron/FileDataStream.cpp
		omicron/FilesystemDataSource.cpp
		omicron/InputServer.cpp
		omicron/Tcp.cpp
		omicron/Thread.cpp
		omicron/WandService.cpp
		omicron/SagePointerService.cpp
		omicron/Timer.cpp
		omicron/TouchGestureManager.cpp
		omicron/MocapGestureManager.cpp
		omicron/GestureService.cpp
		omicron/RayPointMapper.cpp
		
		#omega::xml
		omicron/xml/tinystr.cpp
		omicron/xml/tinyxml.cpp
		omicron/xml/tinyxmlerror.cpp
		omicron/xml/tinyxmlparser.cpp
		
		#omega::libconfig
        omicron/libconfig/libconfigcpp.cpp
		omicron/libconfig/ArgumentHelper.cpp
		
		#omega::tinythread
		omicron/tinythread/tinythread.cpp
        )

SET( csrcs
        omicron/libconfig/libconfig.c
        omicron/libconfig/scanctx.c
        omicron/libconfig/scanner.c
        omicron/libconfig/strbuf.c
        omicron/libconfig/grammar.c
	)
		
###############################################################################
# Headers
SET( headers 
		${CMAKE_SOURCE_DIR}/include/omicron.h
		${CMAKE_SOURCE_DIR}/include/omicron/AssetCacheManager.h
		${CMAKE_SOURCE_DIR}/include/omicron/AssetCacheService.h
		${CMAKE_SOURCE_DIR}/include/omicron/ByteArray.h
        ${CMAKE_SOURCE_DIR}/include/omicron/osystem.h
        ${CMAKE_SOURCE_DIR}/include/omicron/otypes.h
        ${CMAKE_SOURCE_DIR}/include/omicron/Config.h
        ${CMAKE_SOURCE_DIR}/include/omicron/Event.h
        ${CMAKE_SOURCE_DIR}/include/omicron/HttpRequest.h
        ${CMAKE_SOURCE_DIR}/include/omicron/IEventListener.h
        ${CMAKE_SOURCE_DIR}/include/omicron/ServiceManager.h
        ${CMAKE_SOURCE_DIR}/include/omicron/Service.h
        ${CMAKE_SOURCE_DIR}/include/omicron/HeartbeatService.h
        ${CMAKE_SOURCE_DIR}/include/omicron/StringUtils.h
        ${CMAKE_SOURCE_DIR}/include/omicron/DataManager.h
        ${CMAKE_SOURCE_DIR}/include/omicron/FilesystemDataSource.h
        ${CMAKE_SOURCE_DIR}/include/omicron/FileDataStream.h
        ${CMAKE_SOURCE_DIR}/include/omicron/InputServer.h
		${CMAKE_SOURCE_DIR}/include/omicron/Tcp.h
        ${CMAKE_SOURCE_DIR}/include/omicron/Thread.h
        ${CMAKE_SOURCE_DIR}/include/omicron/WandService.h
        ${CMAKE_SOURCE_DIR}/include/omicron/SagePointerService.h
        ${CMAKE_SOURCE_DIR}/include/omicron/Timer.h
        ${CMAKE_SOURCE_DIR}/include/omicron/TouchGestureManager.h
		${CMAKE_SOURCE_DIR}/include/omicron/MocapGestureManager.h
		${CMAKE_SOURCE_DIR}/include/omicron/GestureService.h
		
		# eigenwrap
        ${CMAKE_SOURCE_DIR}/include/omicron/math/EigenWrappers.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/Math.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/Ray.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/Sphere.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/Plane.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/AlignedBox.h
        ${CMAKE_SOURCE_DIR}/include/omicron/math/PlaneBoundedVolume.h
		
		#omega::xml
		${CMAKE_SOURCE_DIR}/include/omicron/xml/tinyxml.h
		${CMAKE_SOURCE_DIR}/include/omicron/xml/tinystr.h
        ) 
        
###############################################################################
# Enable / disable specific modules within omegalib

# Network input support    
set(OMICRON_USE_NETSERVICE true CACHE BOOL "Enable/disable Network input support")
if(OMICRON_USE_NETSERVICE)
	set( srcs	 ${srcs} omicron/NetService.cpp )
	set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/NetService.h )
endif()

# Network input support    
set(OMICRON_USE_LEGACY_NETSERVICE false CACHE BOOL "Enable/disable Legacy network input support")
if(OMICRON_USE_LEGACY_NETSERVICE)
	set( srcs	 ${srcs} omicron/LegacyNetService.cpp )
	set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/LegacyNetService.h )
endif()

# OpenNI (Kinect) support
set(OMICRON_USE_OPENNI false CACHE BOOL "Enable/disable OpenNI support (OpenNI libraries must be installed in the system)")
if(OMICRON_USE_OPENNI)
	set( srcs	 ${srcs} omicron/OpenNIService.cpp )
	set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/OpenNIService.h )
	if(WIN32)
		include_directories($ENV{PROGRAMFILES}/OpenNI/Include)
	else(WIN32)
		include_directories(/usr/include/ni)
		include_directories(/usr/include/nite)
	endif(WIN32)
endif()

# Kinect for Windows support
if(WIN32)
	set(KINECTSDK10 $ENV{KINECTSDK10_DIR})
	set(KINECTSDK20 $ENV{KINECTSDK20_DIR})
	
	if( KINECTSDK10 OR KINECTSDK20 )
		set(OMICRON_USE_KINECT_FOR_WINDOWS "DISABLED" CACHE STRING "Enable/disable Kinect for Windows support (Requires Kinect for Windows SDK Redistributable - Xbox 360 Kinect requires full SDK (v1.x only)")
		SET_PROPERTY(CACHE OMICRON_USE_KINECT_FOR_WINDOWS PROPERTY STRINGS "v2.0" "v1.x" "DISABLED") 
		
		if( KINECTSDK10 )
			message("Kinect for Windows SDK v1.x found")
			#/set(OMICRON_USE_KINECT_FOR_WINDOWS_v1 false CACHE BOOL "Enable/disable Kinect for Windows v1.x support (Kinect for Windows SDK Redistributable must be installed - The full SDK must be installed if using an Xbox 360 Kinect)")
		else( KINECTSDK10 )
			SET_PROPERTY(CACHE OMICRON_USE_KINECT_FOR_WINDOWS PROPERTY STRINGS "v2.0" "DISABLED") 
		endif()
		
		if( KINECTSDK20 )
			message("Kinect for Windows SDK v2.x found")
			#set(OMICRON_USE_KINECT_FOR_WINDOWS_v2 false CACHE BOOL "Enable/disable Kinect for Windows v2.0 support (Kinect for Windows SDK Redistributable must be installed)")
		else( KINECTSDK20 )
			SET_PROPERTY(CACHE OMICRON_USE_KINECT_FOR_WINDOWS PROPERTY STRINGS "v1.x" "DISABLED") 
		endif()
		
		if(${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "v1.x")
			message("Using Kinect for Windows SDK v1.x")
			
			set( srcs	 ${srcs} omicron/MSKinectService.cpp
				)
			set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/MSKinectService.h
				)
			set(KINECT_SDK_INCLUDE_DIR $ENV{KINECTSDK10_DIR}inc CACHE STRING "Kinect for Windows SDK v1.x include directory")
			set(KINECT_SDK_DIR $ENV{KINECTSDK10_DIR})
			set(OMICRON_USE_KINECT_FOR_WINDOWS_v1 true)
			
			include_directories(${KINECT_SDK_INCLUDE_DIR})

			set(OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO false CACHE BOOL "Enable/disable Kinect for Windows Audio/Speech recognition support (Microsoft Speech Platform SDK Version 11 must be installed)")
		elseif(${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "v2.0")
			message("Using Kinect for Windows SDK v2.0")
		
			set( srcs	 ${srcs} omicron/MSKinect2Service.cpp
				)
			set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/MSKinect2Service.h
				)
			set(KINECT_SDK_INCLUDE_DIR $ENV{KINECTSDK20_DIR}inc STRING "Kinect for Windows SDK v2.0 include directory")
			set(KINECT_SDK_DIR $ENV{KINECTSDK20_DIR})
			set(OMICRON_USE_KINECT_FOR_WINDOWS_v2 true)
			
			include_directories(${KINECT_SDK_INCLUDE_DIR})

			set(OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO false CACHE BOOL "Enable/disable Kinect for Windows Audio/Speech recognition support (Microsoft Speech Platform SDK Version 11 must be installed)")
		endif()
		
		if(OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO)
			set( srcs	 ${srcs} omicron/mskinect/KinectAudioStream.cpp
				)
			set( headers ${headers} ${CMAKE_SOURCE_DIR}/external/include/mskinect/KinectAudioStream.h
									${CMAKE_SOURCE_DIR}/external/include/mskinect/stdafx.h
				)
			set(MS_SPEECH_SDK_INCLUDE_DIR $ENV{PROGRAMFILES}/Microsoft\ SDKs/Speech/v11.0/Include CACHE STRING "Microsoft Speech SDK include directory")
		
			include_directories(${MS_SPEECH_SDK_INCLUDE_DIR})
		endif()
	endif()
	
endif(WIN32)

# OSC support    
set(OMICRON_USE_SOUND true CACHE BOOL "Enable/disable sound support")
if(OMICRON_USE_SOUND)
	set( srcs	 ${srcs} omicron/SoundManager.cpp
						 omicron/Sound.cpp )
	set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/SoundManager.h
							${CMAKE_SOURCE_DIR}/include/omicron/Sound.h )
endif()

# NaturalPoint Motion capture system support
if(WIN32)
	set(OMICRON_USE_NATURAL_POINT true CACHE BOOL "Enable/disable NaturalPoint MoCap support")
	if(OMICRON_USE_NATURAL_POINT)
		set( srcs	 ${srcs} omicron/NaturalPointService.cpp )
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/NaturalPointService.h )
	ENDIF()
endif()

# Optitrack tracking system support
if(WIN32)
	set(OMICRON_USE_OPTITRACK false CACHE BOOL "Enable/disable OptiTrack camera support")
	if(OMICRON_USE_OPTITRACK)
		set( srcs	 ${srcs} omicron/OptiTrackService.cpp )
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/OptiTrackService.h )
	endif()
endif()

# PQLabs touch surface module
if(WIN32)
	set(OMICRON_USE_PQLABS true CACHE BOOL "Enable/disable PqLabs multitouch support")
	if(OMICRON_USE_PQLABS)
		set( srcs	 ${srcs} omicron/PQService.cpp )
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/PQService.h )
	endif()
endif()

# DirectInput support
if(WIN32)
	set(OMICRON_USE_DIRECTINPUT true CACHE BOOL "Enable DirectInput / XInput support (Supports Xbox360 and Wiimote controllers)")
	if(OMICRON_USE_DIRECTINPUT)
		set( srcs	 ${srcs} omicron/LegacyDirectInputService.cpp
							 omicron/DirectInputService.cpp
							 omicron/XInputService.cpp
							 omicron/WiimoteService.cpp
							 omicron/Wiimote.cpp )
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/LegacyDirectInputService.h
								${CMAKE_SOURCE_DIR}/include/omicron/DirectInputService.h
								${CMAKE_SOURCE_DIR}/include/omicron/XInputService.h
								${CMAKE_SOURCE_DIR}/include/omicron/Wiimote.h 
								${CMAKE_SOURCE_DIR}/include/omicron/WiimoteService.h 
								${CMAKE_SOURCE_DIR}/include/omicron/wiimote_common.h 
								${CMAKE_SOURCE_DIR}/include/omicron/wiimote_state.h )
#		include_directories( ${CMAKE_SOURCE_DIR}/include/omicron/DirectXInputService.h
#							 ${CMAKE_SOURCE_DIR}/include/omicron/Wiimote.h 
#							 ${CMAKE_SOURCE_DIR}/include/omicron/wiimote_common.h 
#							 ${CMAKE_SOURCE_DIR}/include/omicron/wiimote_state.h )
	endif()
endif()
	
# VRPN module support
if(WIN32)
	set(OMICRON_USE_VRPN true CACHE BOOL "Enable/disable VRPN Client support")
	if(OMICRON_USE_VRPN)
		set( srcs	 ${srcs} omicron/VRPNService.cpp
					 ${srcs} omicron/VRPNDevice.cpp
			)
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/VRPNService.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/VRPNDevice.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_BaseClass.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_Configure.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_Connection.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_Shared.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_Tracker.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn/vrpn_Types.h
					 
			)
		include_directories( ${CMAKE_SOURCE_DIR}/include/omicron/VRPNService.h
							 ${CMAKE_SOURCE_DIR}/include/omicron/VRPNDevice.h
							 ${CMAKE_SOURCE_DIR}/include/omicron/vrpn
			)
	endif()
endif()

# ThinkGear module support
if(WIN32)
	set(OMICRON_USE_THINKGEAR true CACHE BOOL "Enable/disable ThinkGear support")
	if(OMICRON_USE_THINKGEAR)
		set( srcs	 ${srcs} omicron/ThinkGearService.cpp )
		set( headers ${headers} ${CMAKE_SOURCE_DIR}/include/omicron/ThinkGearService.h
					 ${CMAKE_SOURCE_DIR}/include/omicron/thinkgear.h
			)
		include_directories( ${CMAKE_SOURCE_DIR}/include/omicron/ThinkGearService.h
			)
	endif()
endif()

###############################################################################
# Group source files into separate folders for visual studio project (this is just eye candy)
source_group(omicron REGULAR_EXPRESSION omicron/)		
source_group(omicron::math REGULAR_EXPRESSION omicron/math)		
source_group(omicron::libconfig REGULAR_EXPRESSION omicron/libconfig)		
source_group(omicron::tinythread REGULAR_EXPRESSION omicron/tinythread)		

###############################################################################
# Create the library with the provided sources and headers
enable_precompiled_headers(omicron/precompiled.h srcs)

# omicron always builds as a shared library
add_library( omicron SHARED ${srcs} ${csrcs} ${headers})

###############################################################################
# Setup module-specific link info
set(EXT_LIB_DIR ${CMAKE_SOURCE_DIR}/external/libs32)

if(OMICRON_USE_NATURAL_POINT)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/NatNetLibStatic.lib)
	endif(WIN32)
endif(OMICRON_USE_NATURAL_POINT)

if(OMICRON_USE_OPTITRACK)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/optitrackuuid.lib)
	endif(WIN32)
endif(OMICRON_USE_OPTITRACK)

if(OMICRON_USE_PQLABS)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/PQMTClient.lib)
		# Copy the PQLabs dll into the destination bin directory
		configure_file(${EXT_LIB_DIR}/PQMTClient.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${EXT_LIB_DIR}/PQMTClient.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endif(WIN32)
endif(OMICRON_USE_PQLABS)

if(OMICRON_USE_NETSERVICE)
	if(WIN32)
		target_link_libraries( omicron ws2_32.lib)
	endif(WIN32)
endif(OMICRON_USE_NETSERVICE)

IF(OMICRON_USE_OPENNI)
	IF(WIN32)
		TARGET_LINK_LIBRARIES( omicron $ENV{PROGRAMFILES}/OpenNI/Lib/openNI.lib )
	ELSE(WIN32)
		TARGET_LINK_LIBRARIES( omicron OpenNI ) 
	ENDIF(WIN32)
ENDIF(OMICRON_USE_OPENNI)

if(NOT ${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "DISABLED")
	if(WIN32)
		if(${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "v1.x")
			target_link_libraries( omicron ${EXT_LIB_DIR}/Kinect10.lib)
			#if(OMICRON_ARCH_32)
			#	target_link_libraries( omicron $ENV{KINECTSDK10_DIR}/lib/x86/Kinect10.lib)
			#endif()
			#if(OMICRON_ARCH_64)
			#	target_link_libraries( omicron $ENV{KINECTSDK10_DIR}/lib/amd64/Kinect10.lib)
			#endif()
			
			# Copy the dll into the destination bin directory
			configure_file(${EXT_LIB_DIR}/Microsoft.Kinect.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
			configure_file(${EXT_LIB_DIR}/Microsoft.Kinect.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
		elseif( ${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "v2.0" )
			target_link_libraries( omicron ${EXT_LIB_DIR}/Kinect20.lib)
			#if(OMICRON_ARCH_32)
			#	target_link_libraries( omicron $ENV{KINECTSDK20_DIR}/lib/x86/Kinect20.lib)
			#endif()
			#if(OMICRON_ARCH_64)
			#	target_link_libraries( omicron $ENV{KINECTSDK20_DIR}/lib/x64/Kinect20.lib)
			#endif()

			# Copy the dll into the destination bin directory
			configure_file(${EXT_LIB_DIR}/Kinect2/Microsoft.Kinect.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
			configure_file(${EXT_LIB_DIR}/Kinect2/Microsoft.Kinect.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
		endif()
		
		endif(WIN32)
endif(NOT ${OMICRON_USE_KINECT_FOR_WINDOWS} MATCHES "DISABLED")

if(OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/sapi.lib)
		configure_file(${EXT_LIB_DIR}/Microsoft.Speech.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		target_link_libraries( omicron Msdmo.lib )
		target_link_libraries( omicron dmoguids.lib )
		target_link_libraries( omicron amstrmid.lib )
		target_link_libraries( omicron avrt.lib )
	endif(WIN32)
endif(OMICRON_USE_KINECT_FOR_WINDOWS_AUDIO)

if(OMICRON_USE_DIRECTINPUT)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/dinput8.lib)
		target_link_libraries( omicron ${EXT_LIB_DIR}/dxguid.lib)
		target_link_libraries( omicron ${EXT_LIB_DIR}/hid.lib)
		target_link_libraries( omicron ${EXT_LIB_DIR}/setupapi.lib)
		target_link_libraries( omicron ${EXT_LIB_DIR}/XInput.lib)
	endif(WIN32)
endif(OMICRON_USE_DIRECTINPUT)

if(OMICRON_USE_VRPN)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/vrpn.lib)
	endif(WIN32)
endif(OMICRON_USE_VRPN)

if(OMICRON_USE_THINKGEAR)
	if(WIN32)
		target_link_libraries( omicron ${EXT_LIB_DIR}/thinkgear.lib)
		# Copy the dll into the destination bin directory
		configure_file(${EXT_LIB_DIR}/thinkgear.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${EXT_LIB_DIR}/thinkgear.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endif(WIN32)
endif(OMICRON_USE_THINKGEAR)

###############################################################################
configure_file(omicron/omicronConfig.h.in ${CMAKE_BINARY_DIR}/include/omicronConfig.h)
