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
#ifndef KDGANTTDATETIMEGRID_P_H
#define KDGANTTDATETIMEGRID_P_H

#include "kdganttdatetimegrid.h"
#include "kdganttabstractgrid_p.h"

#include <QDateTime>

namespace KDGantt {
    class DateTimeGrid::Private : public AbstractGrid::Private {
    public:
        Private() : startDateTime( QDateTime::currentDateTime().addDays( -3 ) ),
                    dayWidth( 100. ), scale(ScaleAuto), hourFormat("hh"), weekStart( Qt::Monday ),
                    freeDays( QSet<Qt::DayOfWeek>() << Qt::Saturday << Qt::Sunday ),
                    rowSeparators( false ) {}

        qreal dateTimeToChartX( const QDateTime& dt ) const;
        QDateTime chartXtoDateTime( qreal x ) const;

        QDateTime startDateTime;
        QDateTime endDateTime;
        qreal dayWidth;
        Scale scale;
        QString hourFormat;
        Qt::DayOfWeek weekStart;
        QSet<Qt::DayOfWeek> freeDays;
        bool rowSeparators;
    };

    inline DateTimeGrid::DateTimeGrid( DateTimeGrid::Private* d ) : AbstractGrid( d ) {}

    inline DateTimeGrid::Private* DateTimeGrid::d_func() {
        return static_cast<Private*>( AbstractGrid::d_func() );
    }
    inline const DateTimeGrid::Private* DateTimeGrid::d_func() const {
        return static_cast<const Private*>( AbstractGrid::d_func() );
    }
}

#endif /* KDGANTTDATETIMEGRID_P_H */

