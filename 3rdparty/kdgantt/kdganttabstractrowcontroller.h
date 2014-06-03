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
#ifndef KDGANTTABSTRACTROWCONTROLLER_H
#define KDGANTTABSTRACTROWCONTROLLER_H

#include <QPair>
#include "kdganttglobal.h"

class QModelIndex;

namespace KDGantt {
    class KDGANTT_EXPORT AbstractRowController {
    public:
        AbstractRowController();
        virtual ~AbstractRowController();

        virtual int headerHeight() const = 0;
        virtual int maximumItemHeight() const = 0;

        virtual bool isRowVisible( const QModelIndex& idx ) const = 0;
        virtual Span rowGeometry( const QModelIndex& idx ) const = 0;


        virtual QModelIndex indexAt( int height ) const = 0;
        virtual QModelIndex indexAbove( const QModelIndex& idx ) const = 0;
        virtual QModelIndex indexBelow( const QModelIndex& idx ) const = 0;
    };
}

#endif /* KDGANTTABSTRACTROWCONTROLLER_H */

