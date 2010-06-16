/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHART_EXPORT_H
#define KDCHART_EXPORT_H

#include <qglobal.h>

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
