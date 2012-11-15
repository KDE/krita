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
#ifndef KDGANTTDATETIMEGRID_H
#define KDGANTTDATETIMEGRID_H

#include "kdganttabstractgrid.h"

#include <QDateTime>
#include <QSet>

namespace KDGantt {
    class KDGANTT_EXPORT DateTimeGrid : public AbstractGrid {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_DERIVED( DateTimeGrid )
    public:
        enum Scale { ScaleAuto, ScaleHour, ScaleDay, ScaleWeek, ScaleMonth, ScaleYear };
	
        DateTimeGrid();
        virtual ~DateTimeGrid();

        QDateTime startDateTime() const;
        void setStartDateTime( const QDateTime& dt );

        qreal dayWidth() const;
        void setDayWidth( qreal );

        void setWeekStart( Qt::DayOfWeek );
        Qt::DayOfWeek weekStart() const;

        void setFreeDays( const QSet<Qt::DayOfWeek>& fd );
        QSet<Qt::DayOfWeek> freeDays() const;

        void setScale( Scale s );
        Scale scale() const;

        bool rowSeparators() const;
        void setRowSeparators( bool enable );

        /*reimp*/ Span mapToChart( const QModelIndex& idx ) const;
        /*reimp*/ bool mapFromChart( const Span& span, const QModelIndex& idx,
                                     const QList<Constraint>& constraints=QList<Constraint>() ) const;
        /*reimp*/ qreal mapToChart( const QVariant& value ) const;
        /*reimp*/ QVariant mapFromChart( qreal x ) const;

        /*reimp*/ void paintGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect,
                                  AbstractRowController* rowController = 0,
                                  QWidget* widget=0 );
        /*reimp*/ void paintHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );

        void render( QPainter* painter,  const QRectF &target, const QRectF& headerRect, const QRectF& exposedRect, QWidget *widget, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio );
        void setHourFormat( const QString &format );
        QString hourFormat() const;

    public Q_SLOTS:
        void zoomIn( qreal factor = 1.25 );
        void zoomOut( qreal factor = 0.8 );
        
    protected:
        virtual void paintHourGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );

        virtual void paintDayGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );

        virtual void paintWeekGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );

        virtual void paintMonthGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );
        
        virtual void paintYearGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );
        
        virtual void paintRowGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect, AbstractRowController* rowController = 0, QWidget* widget=0 );

        virtual void paintFreeDay( QPainter* painter, qreal x, const QRectF& exposedRect, const QDate &dt, QWidget* widget=0 );
        
		virtual void paintHourScaleHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );
		virtual void paintDayScaleHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                    qreal offset, QWidget* widget=0 );
        virtual void paintWeekScaleHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                          qreal offset, QWidget* widget=0 );
        virtual void paintMonthScaleHeader( QPainter* painter, Scale scale, const QRectF& headerRect, const QRectF& exposedRect,
                                           qreal offset, QWidget* widget=0 );


        Scale autoScale() const;
    };
}

#endif /* KDGANTTDATETIMEGRID_H */

