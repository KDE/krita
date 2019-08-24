/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_DEBUG_H_
#define KIS_DEBUG_H_

#include <QDebug>
#include <QLoggingCategory>

#include "kritaglobal_export.h"

/**
 * To show debug output, start krita like:
 *
 * QT_LOGGING_RULES="*.*=false;krita.metadata.*=true;krita.file.*=true"
 *
 */

extern const KRITAGLOBAL_EXPORT QLoggingCategory &_30009();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41000();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41001();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41002();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41003();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41004();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41005();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41006();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41007();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41008();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41009();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41010();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41011();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41012();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41013();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41014();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41015();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41016();
extern const KRITAGLOBAL_EXPORT QLoggingCategory &_41017();

#define dbgResources qCDebug(_30009)
#define dbgKrita qCDebug(_41000)
#define dbgImage qCDebug(_41001)
#define dbgRegistry qCDebug(_41002)
#define dbgTools qCDebug(_41003)
#define dbgTiles qCDebug(_41004)
#define dbgFilters qCDebug(_41005)
#define dbgPlugins qCDebug(_41006)
#define dbgUI qCDebug(_41007)
#define dbgFile qCDebug(_41008)
#define dbgMath qCDebug(_41009)
#define dbgRender qCDebug(_41010)
#define dbgScript qCDebug(_41011)
#define dbgInput qCDebug(_41012)
#define dbgAction qCDebug(_41013)
#define dbgTablet qCDebug(_41014)
#define dbgOpenGL qCDebug(_41015)
#define dbgMetaData qCDebug(_41016)
#define dbgAndroid qCDebug(_41017)

// Defined in Qt 5.6
#if QT_VERSION >= 0x050600
#define infoResources qCInfo(_30009)
#define infoKrita qCInfo(_41000)
#define infoImage qCInfo(_41001)
#define infoRegistry qCInfo(_41002)
#define infoTools qCInfo(_41003)
#define infoTiles qCInfo(_41004)
#define infoFilters qCInfo(_41005)
#define infoPlugins qCInfo(_41006)
#define infoUI qCInfo(_41007)
#define infoFile qCInfo(_41008)
#define infoMath qCInfo(_41009)
#define infoRender qCInfo(_41010)
#define infoScript qCInfo(_41011)
#define infoInput qCInfo(_41012)
#define infoAction qCDebug(_41013)
#define infoTablet qCDebug(_41014)
#define infoOpenGL qCDebug(_41015)
#define infoMetaData qCDebug(_41016)
#endif

#define warnResources qCWarning(_30009)
#define warnKrita qCWarning(_41000)
#define warnImage qCWarning(_41001)
#define warnRegistry qCWarning(_41002)
#define warnTools qCWarning(_41003)
#define warnTiles qCWarning(_41004)
#define warnFilters qCWarning(_41005)
#define warnPlugins qCWarning(_41006)
#define warnUI qCWarning(_41007)
#define warnFile qCWarning(_41008)
#define warnMath qCWarning(_41009)
#define warnRender qCWarning(_41010)
#define warnScript qCWarning(_41011)
#define warnInput qCWarning(_41012)
#define warnAction qCDebug(_41013)
#define warnTablet qCDebug(_41014)
#define warnOpenGL qCDebug(_41015)
#define warnMetaData qCDebug(_41016)

#define errResources qCCritical(_30009)
#define errKrita qCCritical(_41000)
#define errImage qCCritical(_41001)
#define errRegistry qCCritical(_41002)
#define errTools qCCritical(_41003)
#define errTiles qCCritical(_41004)
#define errFilters qCCritical(_41005)
#define errPlugins qCCritical(_41006)
#define errUI qCCritical(_41007)
#define errFile qCCritical(_41008)
#define errMath qCCritical(_41009)
#define errRender qCCritical(_41010)
#define errScript qCCritical(_41011)
#define errInput qCCritical(_41012)
#define errAction qCDebug(_41013)
#define errTablet qCDebug(_41014)
#define errOpenGL qCDebug(_41015)
#define errMetaData qCDebug(_41016)

// Qt does not yet define qCFatal (TODO: this is an oversight, submit a patch upstream)
/*
#define fatalResources qCFatal(_30009)
#define fatalKrita qCFatal(_41000)
#define fatalImage qCFatal(_41001)
#define fatalRegistry qCFatal(_41002)
#define fatalTools qCFatal(_41003)
#define fatalTiles qCFatal(_41004)
#define fatalFilters qCFatal(_41005)
#define fatalPlugins qCFatal(_41006)
#define fatalUI qCFatal(_41007)
#define fatalFile qCFatal(_41008)
#define fatalMath qCFatal(_41009)
#define fatalRender qCFatal(_41010)
#define fatalScript qCFatal(_41011)
#define fatalInput qCFatal(_41012)
#define fatalAction qCDebug(_41013)
*/


/**
 * Show a nicely formatted backtrace.
 */
KRITAGLOBAL_EXPORT QString kisBacktrace();

/**
 * Please pretty print my variable
 *
 * Use this macro to display in the output stream the name of a variable followed by its value.
 */
#define ppVar( var ) #var << "=" << (var)

#  ifndef QT_NO_DEBUG
#    undef Q_ASSERT
#    define Q_ASSERT(cond) if(!(cond)) { errKrita << kisBacktrace(); qt_assert(#cond,__FILE__,__LINE__); } qt_noop()
#  endif


#ifdef __GNUC__
KRITAGLOBAL_EXPORT QString __methodName(const char *prettyFunction);
#define __METHOD_NAME__ __methodName(__PRETTY_FUNCTION__)
#else
#define __METHOD_NAME__ "<unknown>:<unknown>"
#endif

#define PREPEND_METHOD(msg) QString("%1: %2").arg(__METHOD_NAME__).arg(msg)

#ifdef __GNUC__
#define ENTER_FUNCTION() qDebug() << "Entering" << __METHOD_NAME__
#define LEAVE_FUNCTION() qDebug() << "Leaving " << __METHOD_NAME__
#else
#define ENTER_FUNCTION() qDebug() << "Entering" << "<unknown>"
#define LEAVE_FUNCTION() qDebug() << "Leaving " << "<unknown>"
#endif

#include "kis_assert.h"

#endif
