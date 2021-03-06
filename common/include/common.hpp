/*
    *
    * Global.hpp - Globally used header
    *
*/

#pragma once

// STL
#include <iostream>
#include <fstream>

// Qt Headers
#include <QtCore>
// C++ Standard Library
#include <fstream>

// C Standard Library
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

// SystemWide Headers
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <mntent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <utime.h>
#include <errno.h>
#include <sys/vfs.h>
#include <libgen.h>
#include <sys/inotify.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <mntent.h>

#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>

#if QT_VERSION >= 0x050100
	#include <QCryptographicHash>
	#define QCryptographicHash5 QCryptographicHash
#else
	#include "QCryptographicHash5.hpp"
#endif

#if defined COMMON
	#define PBCOMMON_DLLSPEC Q_DECL_EXPORT
#else
	#define PBCOMMON_DLLSPEC Q_DECL_IMPORT
#endif
