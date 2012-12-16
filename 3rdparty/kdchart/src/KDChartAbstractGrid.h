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

#ifndef KDCHARTABSTRACTGRID_H
#define KDCHARTABSTRACTGRID_H

#include <QPair>

#include "KDChartAbstractCoordinatePlane.h"
#include "KDChartGridAttributes.h"
#include "KDChartAbstractDiagram.h"
#include "KDChartCartesianAxis.h"

namespace KDChart {

    class PaintContext;


    /**
     * \internal
     *
     * \brief Abstract base class for grid classes: cartesian, polar, ...
     *
     * The AbstractGrid interface is the base class used by
     * AbstractCoordinatePlane, for calculating and for drawing
     * the grid lines of the plane.
     */
    class AbstractGrid
    {
    public:
        virtual ~AbstractGrid();
    protected:
        AbstractGrid ();


    public:
        /** \brief Returns the cached result of data calculation.
          *
          * For this, all derived classes need to implement the
          * pure-virtual calculateGrid() method.
          */
        DataDimensionsList updateData( AbstractCoordinatePlane* plane );

        /**
          * Doing the actual drawing.
          *
          * Every derived class must implement this.
          *
          * \note When implementing drawGrid():  Before you start drawing,
          * make sure to call updateData(), to get the data boundaries
          * recalculated.
          * For an example, see the implementation of CartesianGrid:drawGrid().
          */
        virtual void drawGrid( PaintContext* context ) = 0;

        /**
         * Causes grid to be recalculated upon the next call
         * of updateData().
         *
         * \see calculateGrid
         */
        void setNeedRecalculate();

        /**
         * Checks whether both coordinates of r are valid according
         * to isValueValid
         *
         * \see isValueValid
         */
        static bool isBoundariesValid(const QRectF& r );

        /**
         * Checks whether both coordinates of both points are valid
         * according to isValueValid
         *
         * \see isValueValid
         */
        static bool isBoundariesValid(const QPair<QPointF,QPointF>& b );

        /**
         * Checks whether all start and end properties of every
         * DataDimension in the list l are valid according to
         * isValueValid().
         *
         * \see isValueValid
         */
        static bool isBoundariesValid(const DataDimensionsList& l );

        /**
         * Checks if r is neither NaN nor infinity.
         */
        static bool isValueValid(const qreal& r );

        /**
         * Adjusts \a start and/or \a end so that they are a multiple of
         * \a stepWidth
         */
        static void adjustLowerUpperRange(
                qreal& start, qreal& end,
                qreal stepWidth,
                bool adjustLower, bool adjustUpper );

        /**
         * Adjusts \a dim so that \c dim.start and/or \c dim.end are a multiple
         * of \c dim.stepWidth.
         *
         * \see adjustLowerUpperRange
         */
        static const DataDimension adjustedLowerUpperRange(
                const DataDimension& dim,
                bool adjustLower, bool adjustUpper );

        GridAttributes gridAttributes;

    protected:
        DataDimensionsList mData;
        AbstractCoordinatePlane* mPlane;

    private:
        /**
          * \brief Calculates the grid start/end/step width values.
          *
          * Gets the raw data dimensions - e.g. the data model's boundaries,
          * together with their isCalculated flags.
          *
          * Returns the calculated start/end values for the grid, and their
          * respective step widths.
          * If at least one of the step widths is Zero, all dimensions of
          * the returned list are considered invalid!
          *
          * \note This function needs to be implemented by all derived classes,
          * like CartesianGrid, PolarGrid, ...
          */
        virtual DataDimensionsList calculateGrid( const DataDimensionsList& rawDataDimensions ) const = 0;
        DataDimensionsList mCachedRawDataDimensions;
    };

}

#endif
