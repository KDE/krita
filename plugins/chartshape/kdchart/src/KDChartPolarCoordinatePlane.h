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

#ifndef KDCHART_POLAR_COORDINATEPLANE_H
#define KDCHART_POLAR_COORDINATEPLANE_H

#include "KDChartAbstractCoordinatePlane.h"

namespace KDChart {

    class Chart;
    class PaintContext;

    /**
      * @brief Polar coordinate plane
      */
    class KDCHART_EXPORT PolarCoordinatePlane : public AbstractCoordinatePlane
    {
        Q_OBJECT

        Q_DISABLE_COPY( PolarCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( PolarCoordinatePlane, Chart* )

    public:
        struct CoordinateTransformation;
        typedef QList<CoordinateTransformation> CoordinateTransformationList;

        explicit PolarCoordinatePlane ( Chart* parent = 0 );
        ~PolarCoordinatePlane();

        void addDiagram ( AbstractDiagram* diagram );

        const QPointF translate ( const QPointF& diagramPoint ) const;
        const QPointF translatePolar ( const QPointF& diagramPoint ) const;

        /** \brief Specify the rotation of the coordinate plane.
          *
          * In a Pie diagram this indicates the position where the first pie starts,
          * in a Polar diagram it specifies the Zero position of the circular axis.
          * \sa startPosition
          */
        void setStartPosition( qreal degrees );
        /** Retrieve the rotation of the coordinate plane.
          * \sa setStartPosition
          */
        qreal startPosition() const;

        virtual double zoomFactorX() const;
        virtual double zoomFactorY() const;

        virtual void setZoomFactors( double factorX, double factorY );
        virtual void setZoomFactorX( double factor );
        virtual void setZoomFactorY( double factor );

        virtual QPointF zoomCenter() const;

        virtual void setZoomCenter( const QPointF& center );

        /**
         * Set the attributes to be used for grid lines drawn in circular
         * direction (or in sagittal direction, resp.).
         *
         * To disable circular grid painting, for example, your code should like this:
         * \code
         * GridAttributes ga = plane->gridAttributes( bool );
         * ga.setGridVisible( false );
         * plane-setGridAttributes( bool, ga );
         * \endcode
         *
         * \note setGridAttributes overwrites the global attributes that
         * were set by AbstractCoordinatePlane::setGlobalGridAttributes.
         * To re-activate these global attributes you can call
         * resetGridAttributes.
         *
         * \sa resetGridAttributes, gridAttributes
         * \sa AbstractCoordinatePlane::setGlobalGridAttributes
         * \sa hasOwnGridAttributes
         */
        void setGridAttributes( bool circular, const GridAttributes & );

        /**
         * Reset the attributes to be used for grid lines drawn in circular
         * direction (or in sagittal direction, resp.).
         * By calling this method you specify that the global attributes set by
         * AbstractCoordinatePlane::setGlobalGridAttributes be used.
         *
         * \sa setGridAttributes, gridAttributes
         * \sa AbstractCoordinatePlane::globalGridAttributes
         * \sa hasOwnGridAttributes
         */
        void resetGridAttributes( bool circular );

        /**
         * \return The attributes used for grid lines drawn in circular
         * direction (or in sagittal direction, resp.).
         *
         * \note This function always returns a valid set of grid attributes:
         * If no special grid attributes were set for this direction
         * the global attributes are returned, as returned by
         * AbstractCoordinatePlane::globalGridAttributes.
         *
         * \sa setGridAttributes
         * \sa resetGridAttributes
         * \sa AbstractCoordinatePlane::globalGridAttributes
         * \sa hasOwnGridAttributes
         */
        const GridAttributes gridAttributes( bool circular ) const;

        /**
         * \return Returns whether the grid attributes have been set for the
         * respective direction via setGridAttributes( bool circular ).
         *
         * If false, the grid will use the global attributes set
         * by AbstractCoordinatePlane::globalGridAttributes (or the default
         * attributes, resp.)
         *
         * \sa setGridAttributes
         * \sa resetGridAttributes
         * \sa AbstractCoordinatePlane::globalGridAttributes
         */
        bool hasOwnGridAttributes( bool circular ) const;

        qreal angleUnit() const;
        qreal radiusUnit() const;

        /** reimpl */
        virtual void paint( QPainter* );

    protected:
        virtual DataDimensionsList getDataDimensionsList() const;
        void paintEvent ( QPaintEvent* );
        void resizeEvent ( QResizeEvent* );

        void layoutDiagrams();
    protected Q_SLOTS:
        void slotLayoutChanged( AbstractDiagram* diagram );
        void adjustZoomAndRepaint();

    private:
        void setHasOwnGridAttributes(
            bool circular, bool on );
    };

}

#endif
