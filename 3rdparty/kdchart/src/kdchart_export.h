/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDCHART_EXPORT_H
#define KDCHART_EXPORT_H

#include <QtGlobal>

# ifdef KDCHART_STATICLIB
#  undef KDCHART_SHAREDLIB
#  define KDCHART_EXPORT
#  define UITOOLS_EXPORT
#  define KDCHART_COMPAT_EXPORT
#  define KDCHART_PLUGIN_EXPORT
# else
#  ifdef KDCHART_BUILD_KDCHART_LIB
#   define KDCHART_EXPORT Q_DECL_EXPORT
#  else
#   define KDCHART_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef UITOOLS_BUILD_UITOOLS_LIB
#   define UITOOLS_EXPORT Q_DECL_EXPORT
#  else
#   define UITOOLS_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef KDCHART_BUILD_KDCHART_COMPAT_LIB
#   define KDCHART_COMPAT_EXPORT Q_DECL_EXPORT
#  else
#   define KDCHART_COMPAT_EXPORT Q_DECL_IMPORT
#  endif
#  ifdef KDCHART_BUILD_PLUGIN_LIB
#   define KDCHART_PLUGIN_EXPORT Q_DECL_EXPORT
#  else
#   define KDCHART_PLUGIN_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif // KDCHART_EXPORT_H
