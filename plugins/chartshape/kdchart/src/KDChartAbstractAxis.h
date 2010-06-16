  /* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTABSTRACTAXIS_H
#define KDCHARTABSTRACTAXIS_H

// #include <QObject>
// #include <QRectF>
// #include <QWidget>

#include "kdchart_export.h"
#include "KDChartGlobal.h"
#include "KDChartAbstractArea.h"
#include "KDChartTextAttributes.h"
#include "KDChartRulerAttributes.h"


class QPainter;
class QSizeF;
// class QRectF;


namespace KDChart {

    class Area;
    class AbstractCoordinatePlane;
    class PaintContext;
    class AbstractDiagram;

    /**
      * The base class for axes.
      *
      * For being useful, axes need to be assigned to a diagram, see
      * AbstractCartesianDiagram::addAxis and AbstractCartesianDiagram::takeAxis.
      *
      * \sa PolarAxis, AbstractCartesianDiagram
      */
    class KDCHART_EXPORT AbstractAxis : public AbstractArea
    {
        Q_OBJECT

        Q_DISABLE_COPY( AbstractAxis )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( AbstractAxis, AbstractDiagram* )

    public:
        explicit AbstractAxis( AbstractDiagram* diagram = 0 );
        virtual ~AbstractAxis();

        // FIXME implement when code os ready for it:
        // virtual Area* clone() const = 0;

        // FIXME (Mirko) readd when needed
        // void copyRelevantDetailsFrom( const KDChartAxis* axis );

        /*    virtual void paint( PaintContext* ) const = 0;
              virtual QSize sizeHint() const = 0;*/
	//virtual void paintEvent( QPaintEvent* event) = 0;

        /**
         * \brief Implement this method if you want to adjust axis labels
         * before they are printed.
         *
         * KD Chart is calling this method immediately before drawing the
         * text, this  means: What you return here will be drawn without
         * further modifications.
         * 
         * \param label The text of the label as KD Chart has calculated it
         * automatically (or as it was taken from a QStringList provided
         * by you, resp.)
         *
         * \return The text to be drawn. By default this is the same as \c label.
         */
        virtual const QString customizedLabel( const QString& label )const;

        /**
         * Returns true if both axes have the same settings.
         */
        bool compare( const AbstractAxis* other )const;

        void createObserver( AbstractDiagram* diagram );
        void deleteObserver( AbstractDiagram* diagram );
        const AbstractDiagram* diagram() const;
        bool observedBy( AbstractDiagram* diagram ) const;
        virtual void connectSignals();

        void setTextAttributes( const TextAttributes &a );
        TextAttributes textAttributes() const;
        
        void setRulerAttributes( const RulerAttributes &a );
        RulerAttributes rulerAttributes() const;

        void setLabels( const QStringList& list );
        QStringList labels() const;
        void setShortLabels( const QStringList& list );
        QStringList shortLabels() const;

        virtual void setGeometry( const QRect& rect ) = 0;
        virtual QRect geometry() const = 0;

        const AbstractCoordinatePlane* coordinatePlane() const;

    protected Q_SLOTS:
        /** called for initializing after the c'tor has completed */
        virtual void delayedInit();

    public Q_SLOTS:
        void update();
    };
}

#endif // KDCHARTABSTRACTAXIS_H
