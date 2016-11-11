/*
* @(#)thinkgear.h    v1    Aug 12, 2015
*
* Copyright (c) 2008-2015 NeuroSky, Inc. All Rights Reserved
* NEUROSKY PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
*/

/* Ensure this header is only included once */
#ifndef THINKGEAR_H_
#define THINKGEAR_H_

/**
* @file thinkgear.h
*
* This header file defines the Stream SDK for PC API,
* which is a set of functions for users to connect to and receive data
* from a ThinkGear data stream.
*
* There are three "tiers" of functions in this API, where functions
* of a lower "tier" should not be called until an appropriate function
* in the "tier" above it is called first:
*
* @code
*
* Tier 1:  Call anytime
*     TG_GetVersion()
*     TG_GetNewConnectionId()
*
* Tier 2:  Requires TG_GetNewConnectionId()
*     TG_SetStreamLog()
*     TG_SetDataLog()
*     TG_Connect()
*     TG_FreeConnection()
*
* Tier 3:  Requires TG_Connect()
*     TG_ReadPackets()
*     TG_GetValueStatus()
*     TG_GetValue()
*     TG_SendByte()
*     TG_SetBaudrate()
*     TG_SetDataFormat()
*     TG_Disconnect()
*
* @endcode
*
* Regarding preprocessor macro constants:  Typical users do not need to
* (and should NOT) define any special macros in order to use this header
* file with a Stream SDK shared library (i.e. .dll
* or .so). The following macros should only be defined by maintainers of
* the Stream SDK shared library itself:
*     THINKGEAR_DLL_COMPILE (or NO_DLL_MODIFIERS)
*
* @version 1 Aug 12, 2015 Rico Wang
*   - Initial version. This version is based on ThinkGear Connect Driver v4.2
*/

/* Disable name-mangling when compiling as C++ */
#ifdef __cplusplus
extern "C" {
#endif

	/* This block is standard for DLL macro definitions, and
	* typical users should ignore this block entirely.
	* Maintainers of the DLL should define THINKGEAR_DLL_COMPILE
	* when compiling the DLL.
	*/
#define THINKGEAR_DLL_COMPILE
#ifdef NO_DLL_MODIFIERS
#define THINKGEAR_API
#else
#if defined(_WIN32)
#if defined(THINKGEAR_DLL_COMPILE)
#define THINKGEAR_API __declspec( dllexport )
#else /* !THINKGEAR_DLL_COMPILE */
#define THINKGEAR_API __declspec( dllimport )
#endif /* !THINKGEAR_DLL_COMPILE */
#else /* !_WIN32 */
#define THINKGEAR_API extern
#endif /* !_WIN32 */
#endif /* !NO_DLL_MODIFIERS */
	/* Allow only for XP and above */
#if !defined(WINVER)
#define WINVER 0x0501
#endif


	/**
	* Maximum number of Connections that can be requested before being
	* required to free one.
	*/
#define TG_MAX_CONNECTION_HANDLES 128

	/**
	* Baud rate for use with TG_Connect() and TG_SetBaudrate().
	*/
#define TG_BAUD_1200         1200
#define TG_BAUD_2400         2400
#define TG_BAUD_4800         4800
#define TG_BAUD_9600         9600
#define TG_BAUD_57600       57600
#define TG_BAUD_115200     115200

	/**
	* Data format for use with TG_Connect() and TG_SetDataFormat().
	*/
#define TG_STREAM_PACKETS      0
#define TG_STREAM_FILE_PACKETS 2

	/**
	* Data types that can be requested from TG_GetValue().  Only
	* certain data types are output by certain ThinkGear chips
	* and headsets.  Please refer to the Communications Protocol
	* document for your chip/headset to determine which data types
	* are available for your hardware.
	*/
#define TG_DATA_BATTERY             0
#define TG_DATA_POOR_SIGNAL         1
#define TG_DATA_ATTENTION           2
#define TG_DATA_MEDITATION          3
#define TG_DATA_RAW                 4
#define TG_DATA_DELTA               5
#define TG_DATA_THETA               6
#define TG_DATA_ALPHA1              7
#define TG_DATA_ALPHA2              8
#define TG_DATA_BETA1               9
#define TG_DATA_BETA2              10
#define TG_DATA_GAMMA1             11
#define TG_DATA_GAMMA2             12
#define MWM15_DATA_FILTER_TYPE             49

#define MWM15_FILTER_TYPE_50HZ		4
#define MWM15_FILTER_TYPE_60HZ		5



	/**
	* Returns a number indicating the version of the Stream SDK for PC
	* library accessed by this API.  Useful for debugging
	* version-related issues.
	*
	* @return The Stream SDK's version number.
	*/
	THINKGEAR_API int
		TG_GetVersion();


	/**
	* Returns an ID handle (an int) to a newly-allocated ThinkGear Connection
	* object.  The Connection is used to perform all other operations of this
	* API, so the ID handle is passed as the first argument to all functions
	* of this API.
	*
	* When the ThinkGear Connection is no longer needed, be sure to call
	* TG_FreeConnection() on the ID handle to free its resources.  No more
	* than TG_MAX_CONNECTION_HANDLES Connection handles may exist
	* simultaneously without being freed.
	*
	* @return -1 if too many Connections have been created without being freed
	*         by TG_FreeConnection().
	*
	* @return -2 if there is not enough free memory to allocate to a new
	*         ThinkGear Connection.
	*
	* @return The ID handle of a newly-allocated ThinkGear Connection.
	*/
	THINKGEAR_API int
		TG_GetNewConnectionId();


	/**
	* As a ThinkGear Connection reads bytes from its serial stream, it may
	* automatically log those bytes into a log file.  This is useful primarily
	* for debugging purposes.  Calling this function with a valid @c filename
	* will turn this feature on.  Calling this function with an invalid
	* @c filename, or with @c filename set to NULL, will turn this feature
	* off.  This function may be called at any time for either purpose on a
	* ThinkGear Connection.
	*
	* @param connectionId The ID of the ThinkGear Connection to enable stream
	*                     logging for, as obtained from TG_GetNewConnectionId().
	* @param filename     The name of the file to use for stream logging.
	*                     Any existing contents of the file will be erased.
	*                     Set to NULL or an empty string to disable stream
	*                     logging by the ThinkGear Connection.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c filename could not be opened for writing.  You may
	*         check errno for the reason.
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_SetStreamLog(int connectionId, const char *filename);


	/**
	* As a ThinkGear Connection reads and parses Packets of data from its
	* serial stream, it may log the parsed data into a log file.  This is
	* useful primarily for debugging purposes.  Calling this function with
	* a valid @c filename will turn this feature on.  Calling this function
	* with an invalid @c filename, or with @c filename set to NULL, will turn
	* this feature off.  This function may be called at any time for either
	* purpose on a ThinkGear Connection.
	*
	* @param connectionId The ID of the ThinkGear Connection to enable data
	*                     logging for, as obtained from TG_GetNewConnectionId().
	* @param filename     The name of the file to use for data logging.
	*                     Any existing contents of the file will be erased.
	*                     Set to NULL or an empty string to disable stream
	*                     logging by the ThinkGear Connection.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c filename could not be opened for writing.  You may
	*         check errno for the reason.
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_SetDataLog(int connectionId, const char *filename);


	/*
	* Writes a message given by @c msg into the Connection's Stream Log.
	* Optionally the message can be written onto a new line preceded by
	* a timestamp.  The Connection's Stream Log must be already opened
	* by TG_SetStreamLog(), otherwise this function returns an error.
	*
	* @param connectionId    The ID of the ThinkGear Connection to write
	*                        into the Stream Log for, as obtained from
	*                        TG_GetNewConnectionId().
	* @param insertTimestamp If set to any non-zero number, a newline
	*                        and timestamp are automatically prepended
	*                        to the @c msg in the Stream Log file.  Pass
	*                        a zero for this parameter to disable the
	*                        insertion of newline and timestamp before
	*                        @c msg.
	* @param msg             The message to write into the Stream Log
	*                        File.  For Stream Log parsers to ignore
	*                        the message as a human-readable comment
	*                        instead of hex bytes, prepend a '#' sign
	*                        to indicate it is a comment.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if Stream Log for the @c connectionId has not been
	*         opened for writing via TG_SetStreamLog().
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_WriteStreamLog(int connectionId, int insertTimestamp, const char *msg);


	/*
	* Writes a message given by @c msg into the ThinkGear Connection's Data Log.
	* Optionally the message can be written onto a new line preceded by
	* a timestamp.  The Connection's Data Log must be already opened
	* by TG_SetDataLog(), otherwise this function returns an error.
	*
	* @param connectionId    The ID of the ThinkGear Connection to write
	*                        into the Data Log for, as obtained from
	*                        TG_GetNewConnectionId().
	* @param insertTimestamp If set to any non-zero number, a newline
	*                        and timestamp are automatically prepended
	*                        to the @c msg in the Stream Log file.  Pass
	*                        a zero for this parameter to disable the
	*                        insertion of timestamp before
	*                        @c msg.
	* @param msg             The message to write into the Data Log
	*                        File.  For Data Log parsers to ignore
	*                        the message as a human-readable comment
	*                        instead of hex bytes, prepend a '#' sign
	*                        to indicate it is a comment.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if Data Log for the @c connectionId has not been
	*         opened for writing via TG_SetDataLog().
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_WriteDataLog(int connectionId, int insertTimestamp, const char *msg);

	/**
	* Connects a ThinkGear Connection, given by @c connectionId, to a serial
	* communication (COM) port in order to communicate with a ThinkGear module.
	* It is important to check the return value of this function before
	* attempting to use the Connection further for other functions in this API.
	*
	* @param connectionId     The ID of the ThinkGear Connection to connect, as
	*                         obtained from TG_GetNewConnectionId().
	* @param serialPortName   The name of the serial communication (COM) stream
	*                         port.  COM ports on PC Windows systems are named
	*                         like '\\.\COM4' (remember that backslashes in
	*                         strings in most programming languages need to be
	*                         escaped), while COM ports on Windows Mobile
	*                         systems are named like 'COM4:' (note the colon at
	*                         the end).  Linux COM ports may be named like
	*                         '/dev/ttys0'.  Refer to the documentation for
	*                         your particular platform to determine the
	*                         available COM port names on your system.
	* @param serialBaudrate   The baudrate to use to attempt to communicate
	*                         on the serial communication port.  Select from
	*                         one of the TG_BAUD_* constants defined above,
	*                         such as TG_BAUD_9600 or TG_BAUD_57600.
	* @param serialDataFormat The type of ThinkGear data stream.  Select from
	*                         one of the TG_STREAM_* constants defined above.
	*                         Most applications should use TG_STREAM_PACKETS
	*                         (the data format for Embedded ThinkGear).
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c serialPortName could not be opened as a serial
	*         communication port for any reason.  Check that the name
	*         is a valid COM port on your system.
	*
	* @return -3 if @c serialBaudrate is not a valid TG_BAUD_* value.
	*
	* @return -4 if @c serialDataFormat is not a valid TG_STREAM_* type.
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_Connect(int connectionId, const char *serialPortName, int serialBaudrate,
		int serialDataFormat);


	/**
	* Attempts to use the ThinkGear Connection, given by @c connectionId,
	* to read @c numPackets of data from the serial stream.  The Connection
	* will (internally) "remember" the most recent value it has seen of
	* each possible ThinkGear data type, so that any subsequent call to
	* @c TG_GetValue() will return the most recently seen values.
	*
	* Set @c numPackets to -1 to attempt to read all Packets of data that
	* may be currently available on the serial stream.
	*
	* Note that different models of ThinkGear hardware and headsets may
	* output different types of data values at different rates.  Refer to
	* the "Communications Protocol" document for your particular headset
	* to determine the rate at which you need to call this function.
	*
	* @param connectionId The ID of the ThinkGear Connection which should
	*                     read packets from its serial communication stream,
	*                     as obtained from TG_GetNewConnectionId().
	* @param numPackets   The number of data Packets to attempt to read from
	*                     the ThinkGear Connection.  Only the most recently
	*                     read value of each data type will be "remembered"
	*                     by the ThinkGear Connection.  Setting this parameter
	*                     to -1 will attempt to read all currently available
	*                     Packets that are on the data stream.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if there were not even any bytes available to be read from
	*         the Connection's serial communication stream.
	*
	* @return -3 if an I/O error occurs attempting to read from the Connection's
	*         serial communication stream.
	*
	* @return The number of Packets that were successfully read and parsed
	*         from the Connection.
	*/
	THINKGEAR_API int
		TG_ReadPackets(int connectionId, int numPackets);


	/**
	* Returns Non-zero if the @c dataType was updated by the most recent call
	* to TG_ReadPackets().  Returns 0 otherwise.
	*
	* @param connectionId The ID of the ThinkGear Connection to get a data
	*                     value from, as obtained from TG_GetNewConnectionId().
	* @param dataType     The type of data value desired.  Select from one of
	*                     the TG_DATA_* constants defined above.
	*
	* NOTE: This function will terminate the program with a message printed
	* to stderr if @c connectionId is not a valid ThinkGear Connection, or
	* if @c dataType is not a valid TG_DATA_* constant.
	*
	* @return Non-zero if the @c dataType was updated by the most recent call
	*         to TG_GetValue().  Returns 0 otherwise.
	*/
	THINKGEAR_API int
		TG_GetValueStatus(int connectionId, int dataType);


	/**
	* Returns the most recently read value of the given @c dataType, which
	* is one of the TG_DATA_* constants defined above.  Use @c TG_ReadPackets()
	* to read more Packets in order to obtain updated values.  Afterwards, use
	* @c TG_GetValueStatus() to check if a call to @c TG_ReadPackets() actually
	* updated a particular @c dataType.
	*
	* NOTE: This function will terminate the program with a message printed
	* to stderr if @c connectionId is not a valid ThinkGear Connection, or
	* if @c dataType is not a valid TG_DATA_* constant.
	*
	* @param connectionId The ID of the ThinkGear Connection to get a data
	*                     value from, as obtained from TG_GetNewConnectionId().
	* @param dataType     The type of data value desired.  Select from one of
	*                     the TG_DATA_* constants defined above.  Although many
	*                     types of TG_DATA_* constants are available, each model
	*                     of ThinkGear hardware and headset will only output a
	*                     certain subset of these data types. Refer to the
	*                     Communication Protocol document for your particular
	*                     ThinkGear hardware or headset to determine which data
	*                     types are actually output by that hardware or headset.
	*                     Data types that are not output by the headset will
	*                     always return their default value of 0.0 when this
	*                     function is called.
	*
	* @return The most recent value of the requested @c dataType.
	*/
	THINKGEAR_API float
		TG_GetValue(int connectionId, int dataType);


	/**
	* Sends a byte through the ThinkGear Connection (presumably to a ThinkGear
	* module).  This function is intended for advanced ThinkGear Command Byte
	* operations.
	*
	* WARNING: Always make sure at least one valid Packet has been read (i.e.
	*          through the @c TG_ReadPackets() function) at some point BEFORE
	*          calling this function.  This is to ENSURE the Connection is
	*          communicating at the right baud rate.  Sending Command Byte
	*          at the wrong baud rate may put a ThinkGear module into an
	*          indeterminate and inoperable state until it is reset by power
	*          cycling (turning it off and then on again).
	*
	* NOTE: After sending a Command Byte that changes a ThinkGear baud rate,
	*       you will need to call @c TG_SetBaudrate() to change the baud rate
	*       of your @c connectionId as well.  After such a baud rate change,
	*       it is important to check for a valid Packet to be received by
	*       @c TG_ReadPacket() before attempting to send any other Command
	*       Bytes, for the same reasons as describe in the WARNING above.
	*
	* @param connectionId The ID of the ThinkGear Connection to send a byte
	*                     through, as obtained from TG_GetNewConnectionId().
	* @param b            The byte to send through.  Note that only the lowest
	*                     8-bits of the value will actually be sent through.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c connectionId is connected to an input file stream instead
	*         of an actual ThinkGear COM stream (i.e. nowhere to send the byte
	*         to).
	*
	* @return -3 if an I/O error occurs attempting to send the byte (i.e. broken
	*         stream connection).
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_SendByte(int connectionId, int b);


	/**
	* Get the working filter type information. Only support MindWave Mobile 1.5.
	* This is an asynchronous call, it will return immediately,
	* and the filter type will return by data type MWM15_DATA_FILTER_TYPE.
	* Please confirm the state is connected before calling this funciton.
	*
	*
	* @param connectionId The ID of the ThinkGear Connection , obtained from TG_GetNewConnectionId().
	*
	* @return <0 on fail
	* @return 0 on success.
	*/
	THINKGEAR_API int
		MWM15_getFilterType(int connectionId);





	/**
	* Set the filter type with MWM15_FILTER_TYPE_50HZ or MWM15_FILTER_TYPE_60HZ.
	* The value depends on the alternating current frequency(or AC Frequency) of your country or territory, for examle, USA 60HZ, China 50HZ.
	* If you are not sure about it, please google "alternating current frequency YOUR LOCATION".
	* This function only support MindWave Mobile1.5.
	* This is an asynchronous call, it will return immediately.
	* Please confirm the state is connected before calling this funciton.
	* If call this function success, MindWave Mobile 1.5 will stop send out data for 1 seconds.
	* If you want to check the result, please call MWM15_getFilterType 1 seconds later.
	*
	* @param connectionId The ID of the ThinkGear Connection to send a byte
	*                     through, as obtained from TG_GetNewConnectionId().
	* @param filterType    The filter type, including MWM15_FILTER_TYPE_50HZ
	*                      and MWM15_FILTER_TYPE_60HZ
	*
	* @return <0 on fail
	* @return 0 on success.
	*/
	THINKGEAR_API int
		MWM15_setFilterType(int connectionId, int filterType);


	/**
	* Attempts to change the baud rate of the ThinkGear Connection, given by
	* @c connectionId, to @c serialBaudrate.  This function does not typically
	* need to be called, except after calling @c TG_SendByte() to send a
	* Command Byte that changes the ThinkGear module's baud rate.  See
	* TG_SendByte() for details and NOTE.
	*
	* @param connectionId   The ID of the ThinkGear Connection to send a byte
	*                       through, as obtained from TG_GetNewConnectionId().
	* @param serialBaudrate The baudrate to use to attempt to communicate
	*                       on the serial communication port.  Select from
	*                       one of the TG_BAUD_* constants defined above,
	*                       such as TG_BAUD_9600 or TG_BAUD_57600.
	*                       TG_BAUD_57600 is the typical default baud rate
	*                       for most ThinkGear models.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c serialBaudrate is not a valid TG_BAUD_* value.
	*
	* @return -3 if an error occurs attempting to set the baud rate.
	*
	* @return -4 if @c connectionId is connected to an input file stream instead
	*         of an actual ThinkGear COM stream.
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_SetBaudrate(int connectionId, int serialBaudrate);


	/**
	* Attempts to change the data Packet parsing format used by the ThinkGear
	* Connection, given by @c connectionId, to @c serialDataFormat.  This
	* function does not typically need to be called, and is provided only
	* for special testing purposes.
	*
	* @param connectionId     The ID of the ThinkGear Connection to send a byte
	*                         through, as obtained from TG_GetNewConnectionId().
	* @param serialDataFormat The type of ThinkGear data stream.  Select from
	*                         one of the TG_STREAM_* constants defined above.
	*                         Most applications should use TG_STREAM_PACKETS
	*                         (the data format for Embedded ThinkGear modules).
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if @c serialDataFormat is not a valid TG_STREAM_* type.
	*
	* @return 0 on success.
	*/
	//THINKGEAR_API int
	//TG_SetDataFormat( int connectionId, int serialDataFormat );


	/**
	* Enables or disables background auto-reading of the connection.  This
	* has the following implications:
	*
	*  - Setting @c enabled to anything other than 0 will enable background
	*    auto-reading on the specified connection.  Setting @c enabled to 0
	*    will disable it.
	*  - Enabling causes a background thread to be spawned for the connection
	*    (only if one was not already previously spawned), which continuously
	*    calls TG_ReadPacket( connectionId, -1 ) at 1ms intervals.
	*  - Disabling will kill the background thread for the connection.
	*  - While background auto-reading is enabled, the calling program can use
	*    TG_GetValue() at any time to get the most-recently-received value of
	*    any data type. The calling program will have no way of knowing when
	*    a value has been updated.  For most data types other than raw wave
	*    value, this is not much of a problem if the program simply polls
	*    TG_GetValue() once a second or so.
	*  - The current implementation of this function will not include proper
	*    data synchronization. This means it is possible for a value to be
	*    read (by TG_GetValue()) at the same time it is being written to by
	*    the background thread, resulting in a corrupted value being read.
	*    However, this is extremely unlikely for most data types other than
	*    raw wave value.
	*  - While background auto-reading is enabled, the TG_GetValueStatus()
	*    function is pretty much useless.  Also, the TG_ReadPackets()
	*    function should probably not be called.
	*
	* @param connectionId The connection to enable/disable background
	*                     auto-reading on.
	* @param enable       Zero (0) to disable background auto-reading,
	*                     any other value to enable.
	*
	* @return -1 if @c connectionId does not refer to a valid ThinkGear
	*         Connection ID handle.
	*
	* @return -2 if unable to enable background auto-reading.
	*
	* @return -3 if an error occurs while attempting to disable background
	*         auto-reading.
	*
	* @return 0 on success.
	*/
	THINKGEAR_API int
		TG_EnableAutoRead(int connectionId, int enable);


	/**
	* Disconnects the ThinkGear Connection, given by @c connectionId, from
	* its serial communication (COM) port.  Note that after this call, the
	* Connection will not be valid to use with any of the API functions
	* that require a valid ThinkGear Connection, except TG_SetStreamLog(),
	* TG_SetDataLog(), TG_Connect(), and TG_FreeConnection().
	*
	* Note that TG_FreeConnection() will automatically disconnect a
	* Connection as well, so it is not necessary to call this function
	* unless you wish to reuse the @c connectionId to call TG_Connect()
	* again.
	*
	* @param connectionId The ID of the ThinkGear Connection to disconnect, as
	*                     obtained from TG_GetNewConnectionId().
	*/
	THINKGEAR_API void
		TG_Disconnect(int connectionId);


	/**
	* Frees all memory associated with the given ThinkGear Connection.
	*
	* Note that this function will automatically call TG_Disconnect() to
	* disconnect the Connection first, if appropriate, so that it is not
	* necessary to explicitly call TG_Disconnect() at all, unless you wish
	* to reuse the @c connectionId without freeing it first for whatever
	* reason.
	*
	* @param connectionId The ID of the ThinkGear Connection to disconnect, as
	*                     obtained from TG_GetNewConnectionId().
	*/
	THINKGEAR_API void
		TG_FreeConnection(int connectionId);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* THINKGEAR_H_ */
