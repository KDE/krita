/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTABSTRACTCARTESIANDIAGRAM_H
#define KDCHARTABSTRACTCARTESIANDIAGRAM_H

#include "KDChartCartesianCoordinatePlane.h"
#include "KDChartAbstractDiagram.h"
#include "KDChartCartesianAxis.h"

namespace KDChart {

    class GridAttributes;
//    class PaintContext;

    /**
     * @brief Base class for diagrams based on a cartesian coordianate system.
     *
     * The AbstractCartesianDiagram interface adds those elements that are
     * specific to diagrams based on a cartesian coordinate system to the
     * basic AbstractDiagram interface.
     */
    class KDCHART_EXPORT AbstractCartesianDiagram : public AbstractDiagram
    {
        Q_OBJECT
        Q_DISABLE_COPY( AbstractCartesianDiagram )
//        KDCHART_DECLARE_PRIVATE_DERIVED( AbstractCartesianDiagram )
        KDCHART_DECLARE_DERIVED_DIAGRAM( AbstractCartesianDiagram, CartesianCoordinatePlane )

    public:
        explicit AbstractCartesianDiagram ( QWidget* parent = 0, CartesianCoordinatePlane* plane = 0 );
        virtual ~AbstractCartesianDiagram();

        /**
         * Returns true if both diagrams have the same settings.
         */
        bool compare( const AbstractCartesianDiagram* other )const;

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
        virtual const int numberOfAbscissaSegments () const = 0;
        virtual const int numberOfOrdinateSegments () const = 0;
#else
        virtual int numberOfAbscissaSegments () const = 0;
        virtual int numberOfOrdinateSegments () const = 0;
#endif
        /**
         * Add the axis to the diagram. The diagram takes ownership of the axis
         * and will delete it.
         *
         * To gain back ownership (e.g. for assigning the axis to another diagram)
         * use the takeAxis method, before calling addAxis on the other diagram.
         *
         * \sa takeAxis
        */
        virtual void addAxis( CartesianAxis * axis );
        /**
         * Removes the axis from the diagram, without deleting it.
         *
         * The diagram no longer owns the axis, so it is
         * the caller's responsibility to delete the axis.
         *
         * \sa addAxis
        */
        virtual void takeAxis( CartesianAxis * axis );
        /**
         * @return a list of all axes added to the diagram
        */
        virtual KDChart::CartesianAxisList axes () const;

        /**
          * Triggers layouting of all coordinate planes on the current chart.
          * Normally you don't need to call this method. It's handled automatically for you.
         */
        virtual void layoutPlanes();
        /** \reimpl */
        virtual void setCoordinatePlane( AbstractCoordinatePlane* plane );

        /**
          * Makes this diagram use another diagram \a diagram as reference diagram with relative offset
          * \a offset.
          * To share cartesian axes between different diagrams there might be cases when you need that.
          * Normally you don't.
          * \sa examples/SharedAbscissa
          */
        virtual void setReferenceDiagram( AbstractCartesianDiagram* diagram, const QPointF& offset = QPointF() );
        /**
          * @return this diagram's reference diagram
          *  \sa setReferenceDiagram
          */
        virtual AbstractCartesianDiagram* referenceDiagram() const;
        /**
          * @return the relative offset of this diagram's reference diagram
          * \sa setReferenceDiagram
          */
        virtual QPointF referenceDiagramOffset() const;

        /* reimpl */
        void setModel( QAbstractItemModel* model );
        /* reimpl */
        void setRootIndex( const QModelIndex& index );
        /* reimpl */
        void setAttributesModel( AttributesModel* model );

    protected:
        /** @return the 3D item depth of the model index \a index */
        virtual double threeDItemDepth( const QModelIndex& index ) const = 0;
        /** @return the 3D item depth of the data set \a column */
        virtual double threeDItemDepth( int column ) const = 0;
    };

}

#endif
