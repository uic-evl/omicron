#ifndef __OMICRON__
#define __OMICRON__

#define OMICRON_VERSION "3.0"

#include "omicronConfig.h"
#include "omicron/libconfig/ArgumentHelper.h"
#include "omicron/AssetCacheManager.h"
#include "omicron/AssetCacheService.h"
#include "omicron/ByteArray.h"
#include "omicron/Config.h"
#include "omicron/DataManager.h"
#include "omicron/Event.h"
#include "omicron/FileDataStream.h"
#include "omicron/FilesystemDataSource.h"
#include "omicron/IEventListener.h"
#include "omicron/Library.h"
#include "omicron/NameGenerator.h"
#include "omicron/PointSetId.h"
#include "omicron/Thread.h"
#include "omicron/Service.h"
#include "omicron/ServiceManager.h"
#include "omicron/StringUtils.h"
#include "omicron/Tcp.h"
#include "omicron/Timer.h"
#include "omicron/xml/tinyxml.h"
#include "omicron/RayPointMapper.h"

#endif