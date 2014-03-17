![logo](https://code.google.com/p/omicron-sdk/logo?cct=1370629544) omicron

=======
The omicron SDK is a library providing access to a variety of input devices, mostly used in immersive installations and stereo display systems. 

Some of the devices currently supported by omicron are:
 * All motion capture systems supporting the VRPN protocol
 * NaturalPoint trackers (TrackIR, Optitrack)
 * Wii controllers
 * Xbox 360 controllers
 * Microsoft Kinect (multiple Kinects supported. omicron also offers some functions for Multi-kinect transform calibration)
 * SAGE pointer connections
 * PQLabs multitouch overlays
 * iPad touch interfaces and dynamic GUIs through the custom Porthole protocol
 * Thinkgear brainwave interfaces

omicron abstracts input using modular event services, each providing access to a specific input device. Events can be easily streamed over the network using the omicron connector API. Also, Event services can be chained together to provide advanced functionality.

In addition to event services, omicron provides additional utility APIs to simplify the developement of the non-graphics part of VR applications. These APIs are mostly lightweight wrappers on top of well established open source libraries. omicron includes:
 * a configuration file reading system (libconfig)
 * an xml reading/writing API (tinyxml-2 http://www.grinninglizard.com/tinyxml2/index.html)
 * multithreading support (tinythread++ http://tinythread.sourceforge.net/)
 * a simple tcp client/server API (asio http://think-async.com/)
 * a mathematical/geometry library (eigen http://eigen.tuxfamily.org/index.php?title=Main_Page)

omicron input support can be integrated into C++ applications as a static library, or it can be run as a standalone *input server*, streaming input data to multiple applications. Omicron comes with client-side interfaces for:
 * [http://unity3d.com/ Unity]
 * [http://processing.org/ Processing]
 * C++, using a *single header*. Only a few lines of code needed. For more information see the *omicronConnector* section on the [GettingStarted Getting started page].
