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
#ifndef KDGANTTABSTRACTGRID_H
#define KDGANTTABSTRACTGRID_H

#include "kdganttglobal.h"
#include "kdganttconstraint.h"

class QPainter;
class QRectF;
class QAbstractItemModel;
class QModelIndex;

namespace KDGantt {
    class AbstractRowController;
    class Span;

    class KDGANTT_EXPORT AbstractGrid : public QObject {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC( AbstractGrid )
	friend class GraphicsScene;
    public:
        AbstractGrid(QObject* parent = 0);
        virtual ~AbstractGrid();

        QAbstractItemModel* model() const;
        QModelIndex rootIndex() const;

        virtual Span mapToChart( const QModelIndex& idx ) const = 0;
        virtual bool mapFromChart( const Span& span, const QModelIndex& idx,
                                   const QList<Constraint>& constraints=QList<Constraint>() ) const = 0;
        virtual qreal mapToChart( const QVariant &value ) const;
        virtual QVariant mapFromChart( qreal x ) const;
        
        bool isSatisfiedConstraint( const Constraint& c ) const;

        virtual void paintGrid( QPainter* painter, const QRectF& sceneRect, const QRectF& exposedRect,
                                AbstractRowController* rowController = 0, QWidget* widget=0 ) = 0;
        virtual void paintHeader( QPainter* painter, const QRectF& headerRect, const QRectF& exposedRect,
                                  qreal offset, QWidget* widget=0 ) = 0;
        
        virtual void render( QPainter* /*painter*/,  const QRectF & /*target*/, const QRectF&  /*headerRect*/, const QRectF& /*exposedRect*/, QWidget* /*widget*/, Qt::AspectRatioMode /*aspectRatioMode*/ = Qt::KeepAspectRatio ) {}
        
    public Q_SLOTS:
        /*internal*/ virtual void setModel( QAbstractItemModel* model );
        /*internal*/ virtual void setRootIndex( const QModelIndex& idx );
    Q_SIGNALS:
        void gridChanged();
    };
}

#endif /* KDGANTTABSTRACTGRID_H */

