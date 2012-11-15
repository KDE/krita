/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
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
#ifndef KDGANTT_EXPORT_H
#define KDGANTT_EXPORT_H

#include <QtGlobal>

# ifdef KDGANTT_STATICLIB
#  undef KDGANTT_SHAREDLIB
#  define KDGANTT_EXPORT
# elif defined( KDGANTT_SHAREDLIB )
#  ifdef KDGANTT_BUILD_KDGANTT_LIB
#   define KDGANTT_EXPORT Q_DECL_EXPORT
#  else
#   define KDGANTT_EXPORT Q_DECL_IMPORT
#  endif
# else
#  define KDGANTT_EXPORT Q_DECL_IMPORT
# endif


#endif /* KDGANTT_EXPORT_H */

