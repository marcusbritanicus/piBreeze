/*
    *
    * Global.hpp - Globally used header
    *
*/

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <iostream>
#include <fstream>

#include <QtGui>
#include <QtCore>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	#include <QtWidgets>
#endif

#include <PBTTools.hpp>

// NewBreeze Preferences
static QSettings settings( "PBTerminal", "PBTerminal" );

static QFileSystemWatcher *watcher = new QFileSystemWatcher();

#endif
