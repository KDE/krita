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

#ifndef KDCHARTCARTESIANAXIS_H
#define KDCHARTCARTESIANAXIS_H

#include <QList>

#include "KDChartAbstractAxis.h"

namespace KDChart {

    class AbstractCartesianDiagram;

    /**
      * The class for cartesian axes.
      *
      * For being useful, axes need to be assigned to a diagram, see
      * AbstractCartesianDiagram::addAxis and AbstractCartesianDiagram::takeAxis.
      *
      * \sa PolarAxis, AbstractCartesianDiagram
      */
    class KDCHART_EXPORT CartesianAxis : public AbstractAxis
    {
        Q_OBJECT

        Q_DISABLE_COPY( CartesianAxis )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( CartesianAxis, AbstractDiagram* )

    public:
        enum Position {
            Bottom,
            Top,
            Right,
            Left
        };

        /**
          * C'tor of the class for cartesian axes.
          *
          * \note If using a zero parent for the constructor, you need to call
          * your diagram's addAxis function to add your axis to the diagram.
          * Otherwise, there is no need to call addAxis, since the constructor
          * does that automatically for you, if you pass a diagram as parameter.
          *
          * \sa AbstractCartesianDiagram::addAxis
          */
        explicit CartesianAxis ( AbstractCartesianDiagram* diagram = 0 );
        ~CartesianAxis();

        /**
         * Returns true if both axes have the same settings.
         */
        bool compare( const CartesianAxis* other )const;

        /** reimpl */
        virtual void paint( QPainter* );
        /** reimpl */
        virtual void paintCtx( PaintContext* );

        /**
         * Sets the optional text displayed as chart title.
         */
        void setTitleText( const QString& text );
        QString titleText() const;
        
        /**
         * Sets the spacing between the title and the diagram.
         */
        void setTitleSpace( qreal value );
        qreal titleSpace() const;

        void setTitleTextAttributes( const TextAttributes &a );
        /**
          * Returns the text attributes that will be used for displaying the
          * title text.
          * This is either the text attributes as specified by setTitleTextAttributes,
          * or (if setTitleTextAttributes() was not called) the default text attributes.
          * \sa resetTitleTextAttributes, hasDefaultTitleTextAttributes
          */
        TextAttributes titleTextAttributes() const;
        /**
          * Reset the title text attributes to the built-in default:
          *
          * Same font and pen as AbstractAxis::textAttributes()
          * and 1.5 times their size.
          */
        void resetTitleTextAttributes();
        bool hasDefaultTitleTextAttributes() const;

        virtual void setPosition ( Position p );
#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
        virtual const Position position () const;
#else
        virtual Position position () const;
#endif

        virtual void layoutPlanes();

        virtual bool isAbscissa() const;
        virtual bool isOrdinate() const;

        /**
         * Sets the axis annotations to \a annotations.
         * Annotations are a QMap of doubles and QStrings defining special
         * markers and their position.
         * If you use annotations, the normal ticks and values will be invisible.
         * To unset the annotations, pass an empty QMap.
         */
        void setAnnotations( const QMap< double, QString >& annotations );
        /**
         * Returns the currently set axis annotations.
         */
        QMap< double, QString > annotations() const;

        /**
         * Sets custom ticks on the axis.
         * Ticks are a QList of doubles defining their special position.
         */
        void setCustomTicks( const QList< double >& ticksPostions );
        /**
         * Returns the currently set custom ticks on the axis.
         */
        QList< double > customTicks() const;


        /** pure virtual in QLayoutItem */
        virtual bool isEmpty() const;
        /** pure virtual in QLayoutItem */
        virtual Qt::Orientations expandingDirections() const;
        /** pure virtual in QLayoutItem */
        virtual QSize maximumSize() const;
        /** pure virtual in QLayoutItem */
        virtual QSize minimumSize() const;
        /** pure virtual in QLayoutItem */
        virtual QSize sizeHint() const;
        /** pure virtual in QLayoutItem */
        virtual void setGeometry( const QRect& r );
        /** pure virtual in QLayoutItem */
        virtual QRect geometry() const;

    public Q_SLOTS:
        void setCachedSizeDirty() const;

        virtual int tickLength( bool subUnitTicks = false ) const;
    };

    typedef QList<CartesianAxis*> CartesianAxisList;
}

#endif
