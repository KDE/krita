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
#ifndef KDGANTTCONSTRAINT_H
#define KDGANTTCONSTRAINT_H

#include <QModelIndex>
#include <QObject>
#include <QSharedDataPointer>

#include "kdgantt_export.h"
#ifndef QT_NO_DEBUG_STREAM
#include <QDebug>
#endif

namespace KDGantt {
    class KDGANTT_EXPORT Constraint {
        class Private;
    public:
        enum Type
        { 
            TypeSoft = 0, 
            TypeHard = 1
        };	
        enum RelationType
        { 
            FinishStart = 0,
            FinishFinish = 1,
            StartStart = 2,
            StartFinish = 3
        };

        enum ConstraintDataRole
        {
            ValidConstraintPen = Qt::UserRole,
            InvalidConstraintPen
        };

        Constraint( const QModelIndex& idx1,  const QModelIndex& idx2, Type type=TypeSoft, RelationType=FinishStart );
        Constraint( const Constraint& other);
        ~Constraint();

        Type type() const;
        RelationType relationType() const;
        QModelIndex startIndex() const;
        QModelIndex endIndex() const;

        void setData( int role, const QVariant& value );
        QVariant data( int role ) const;

        Constraint& operator=( const Constraint& other );

        bool operator==( const Constraint& other ) const;

        inline bool operator!=( const Constraint& other ) const {
            return !operator==( other );
        }

        uint hash() const;
#ifndef QT_NO_DEBUG_STREAM
        QDebug debug( QDebug dbg) const;
#endif

    private:
        QSharedDataPointer<Private> d;
    };

    inline uint qHash( const Constraint& c ) {return c.hash();}
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<( QDebug dbg, const KDGantt::Constraint& c );
#endif /* QT_NO_DEBUG_STREAM */

#endif /* KDGANTTCONSTRAINT_H */

