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

        /**
          * \internal
          *
          * Method invoked by AbstractCartesianDiagram::addAxis().
          *
          * You should not call this function, unless you know exactly,
          * what you are doing.
          *
          * \sa connectSignals(), AbstractCartesianDiagram::addAxis()
          */
        void createObserver( AbstractDiagram* diagram );

        /**
          * \internal
          *
          * Method invoked by AbstractCartesianDiagram::takeAxis().
          *
          * You should not call this function, unless you know exactly,
          * what you are doing.
          *
          * \sa AbstractCartesianDiagram::takeAxis()
          */
        void deleteObserver( AbstractDiagram* diagram );
        const AbstractDiagram* diagram() const;
        bool observedBy( AbstractDiagram* diagram ) const;

        /**
          * Wireing the signal/slot connections.
          *
          * This method gets called automatically, each time, when you assign
          * the axis to a diagram, either by passing a diagram* to the c'tor,
          * or by calling the diagram's setAxis method, resp.
          *
          * If overwriting this method in derived classes, make sure to call
          * this base method AbstractAxis::connectSignals(), so your axis
          * gets connected to the diagram's built-in signals.
          *
          * \sa AbstractCartesianDiagram::addAxis()
          */
        virtual void connectSignals();

        /**
          \brief Use this to specify the text attributes to be used for axis labels.

          By default, the reference area will be set at painting time.
          It will be the then-valid coordinate plane's parent widget,
          so normally, it will be the KDChart::Chart.
          Thus the labels of all of your axes in all of your diagrams
          within that Chart will be drawn in same font size, by default.

          \sa textAttributes, setLabels
        */
        void setTextAttributes( const TextAttributes &a );

        /**
          \brief Returns the text attributes to be used for axis labels.

          \sa setTextAttributes
        */
        TextAttributes textAttributes() const;
        
        /**
          \brief Use this to specify the attributes used to paint the axis ruler

          Every axis has a default set of ruler attributes that is exactly the
          same among them. Use this method to specify your own attributes.

          \sa rulerAttributes
        */
        void setRulerAttributes( const RulerAttributes &a );

        /**
          \brief Returns the attributes to be used for painting the rulers

          \sa setRulerAttributes
        */
        RulerAttributes rulerAttributes() const;

        /**
          \brief Use this to specify your own set of strings, to be used as axis labels.

          Labels specified via setLabels take precedence:
          If a non-empty list is passed, KD Chart will use these strings as axis labels,
          instead of calculating them.

          If you a smaller number of strings than the number of labels drawn at this
          axis, KD Chart will iterate over the list, repeating the strings, until all
          labels are drawn.
          As an example you could specify the seven days of the week as abscissa labels,
          which would be repeatedly used then.

          By passing an empty QStringList you can reset the default behaviour.

          \sa labels, setShortLabels
        */
        void setLabels( const QStringList& list );

        /**
          Returns a list of strings, that are used as axis labels, as set via setLabels.

          \sa setLabels
        */
        QStringList labels() const;

        /**
          \brief Use this to specify your own set of strings, to be used as axis labels,
          in case the normal labels are too long.

          \note Setting done via setShortLabels will be ignored, if you did not pass
          a non-empty string list via setLabels too!

          By passing an empty QStringList you can reset the default behaviour.

          \sa shortLabels, setLabels
        */
        void setShortLabels( const QStringList& list );

        /**
          Returns a list of strings, that are used as axis labels, as set via setShortLabels.

          \note Setting done via setShortLabels will be ignored, if you did not pass
          a non-empty string list via setLabels too!

          \sa setShortLabels
        */
        QStringList shortLabels() const;

        virtual void setGeometry( const QRect& rect ) = 0;
        virtual QRect geometry() const = 0;

        /**
            \brief Convenience function, returns the coordinate plane, in which this axis is used.

            If the axis is not used in a coordinate plane, the return value is Zero.
         */
        const AbstractCoordinatePlane* coordinatePlane() const;

    protected Q_SLOTS:
        /** called for initializing after the c'tor has completed */
        virtual void delayedInit();

    public Q_SLOTS:
        void update();
    };
}

#endif // KDCHARTABSTRACTAXIS_H
