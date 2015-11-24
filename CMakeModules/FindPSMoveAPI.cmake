# ===========================================================================

#

# FindPSMoveAPI.cmake

#

# 2014 Koosha Mir

#


# PSMOVE_FOUND

# PSMOVE_INCLUDE_DIR

# PSMOVE_LIBRARY_DIR


#

# ===========================================================================


FIND_PATH(PSMOVEAPI_ROOT_DIR "include/psmove.h" PATH $ENV{PSMOVEAPIROOT})
if(PSMOVEAPI_ROOT_DIR)
	 FIND_PATH( PSMOVE_INCLUDE_DIR

			NAMES psmove.h

			PATHS ${PSMOVEAPI_ROOT_DIR}/include /usr/include/PSMoveAPI /usr/local/include/PSMoveAPI )
	 FIND_LIBRARY( PSMOVE_LIBRARY_DIR

			NAMES psmoveapi libpsmoveapi

			PATHS  ${PSMOVEAPI_ROOT_DIR}/build ${PSMOVEAPI_ROOT_DIR} ${PSMOVEAPI_ROOT_DIR}/lib64 ${PSMOVEAPI_ROOT_DIR}/lib /usr/lib /usr/local/lib )		
		
ENDIF()
INCLUDE(FindPackageHandleStandardArgs)


FIND_PACKAGE_HANDLE_STANDARD_ARGS( PSMOVE DEFAULT_MESSAGE

                                    PSMOVE_INCLUDE_DIR

                                    PSMOVE_LIBRARY_DIR )

                                    
MARK_AS_ADVANCED( PSMOVE_INCLUDE_DIR PSMOVE_LIBRARY_DIR )
