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
#ifndef KDGANTTCONSTRAINT_P_H
#define KDGANTTCONSTRAINT_P_H

#include <QSharedData>
#include <QPersistentModelIndex>
#include <QMap>

#include "kdganttconstraint.h"

namespace KDGantt {
    class Constraint::Private : public QSharedData {
    public:
        Private();
        Private( const Private& other );

        inline bool equals( const Private& other ) const {
	    /* Due to a Qt bug we have to check separately for invalid indexes */
            return (start==other.start || (!start.isValid() && !other.start.isValid())) 
		&& (end==other.end || (!end.isValid() && !other.end.isValid())) 
		&& type==other.type
                && relationType==other.relationType
        && data==other.data;
        }

        QPersistentModelIndex start;
        QPersistentModelIndex end;
        Type type;
        RelationType relationType;
        QMap< int, QVariant > data;
    };
}

#endif /* KDGANTTCONSTRAINT_P_H */

