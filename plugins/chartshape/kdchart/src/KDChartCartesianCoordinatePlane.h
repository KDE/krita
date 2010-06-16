/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTCARTESIANCOORDINATEPLANE_H
#define KDCHARTCARTESIANCOORDINATEPLANE_H

#include "KDChartAbstractCoordinatePlane.h"

namespace KDChart {

    class Chart;
    class PaintContext;
    class AbstractDiagram;
    class CartesianAxis;
    class CartesianGrid;

    /**
      * @brief Cartesian coordinate plane
      */
    class KDCHART_EXPORT CartesianCoordinatePlane : public AbstractCoordinatePlane
    {
        Q_OBJECT

        Q_DISABLE_COPY( CartesianCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( CartesianCoordinatePlane, Chart* )

    friend class CartesianAxis;
    friend class CartesianGrid;

    public:
        explicit CartesianCoordinatePlane ( Chart* parent = 0 );
        ~CartesianCoordinatePlane();

        void addDiagram ( AbstractDiagram* diagram );

        void setIsometricScaling ( bool onOff );

        bool doesIsometricScaling() const;

        const QPointF translate ( const QPointF& diagramPoint ) const;

        /**
         * \sa setZoomFactorX, setZoomCenter
         */
        virtual double zoomFactorX() const;
        /**
         * \sa setZoomFactorY, setZoomCenter
         */
        virtual double zoomFactorY() const;

        /**
         * \sa setZoomFactorX,setZoomFactorY
         */
        virtual void setZoomFactors( double factorX, double factorY );
        /**
         * \sa zoomFactorX, setZoomCenter
         */
        virtual void setZoomFactorX( double factor );
        /**
         * \sa zoomFactorY, setZoomCenter
         */
        virtual void setZoomFactorY( double factor );

        /**
         * \sa setZoomCenter, setZoomFactorX, setZoomFactorY
         */
        virtual QPointF zoomCenter() const;

        /**
         * \sa zoomCenter, setZoomFactorX, setZoomFactorY
         */
        virtual void setZoomCenter( const QPointF& center );

        /**
         * Allows to specify a fixed data-space / coordinate-space relation. If set
         * to true then fixed bar widths are used, so you see more bars as the window
         * is made wider.
         *
         * This allows to completely restrict the size of bars in a graph such that,
         * upon resizing a window, the graphs coordinate plane will grow (add more
         * ticks to x- and y-coordinates) rather than have the image grow.
         */
        void setFixedDataCoordinateSpaceRelation( bool fixed );
        bool hasFixedDataCoordinateSpaceRelation() const;


        /**
         * \brief Set the boundaries of the visible value space displayed in horizontal direction.
         *
         * This is also known as the horizontal viewport.
         *
         * By default the horizontal range is adjusted to the range covered by the model's data,
         * see setAutoAdjustHorizontalRangeToData for details.
         * Calling setHorizontalRange with a valid range disables this default automatic adjusting,
         * while on the other hand automatic adjusting will set these ranges.
         *
         * To disable use of this range you can either pass an empty pair by using the default
         * constructor QPair() or you can set both values to the same which constitutes
         * a null range.
         *
         * \note By default the visible data range often is larger than the
         * range calculated from the data model (or set by setHoriz.|Vert.Range(), resp.).
         * This is due to the built-in grid calculation feature: The visible start/end
         * values get adjusted so that they match a main-grid line.
         * You can turn this feature off for any of the four bounds by calling
         * GridAttributes::setAdjustBoundsToGrid() for either the global grid-attributes
         * or for the horizontal/vertical attrs separately.
         *
         * \note If you use user defined vertical ranges together with logarithmic scale, only
         * positive values are supported. If you set it to negative values, the result is undefined.
         *
         * \param range a pair of values representing the smalles and the largest
         * horizontal value space coordinate displayed.
         *
         * \sa setAutoAdjustHorizontalRangeToData, setVerticalRange
         * \sa GridAttributes::setAdjustBoundsToGrid()
         */
        void setHorizontalRange( const QPair<qreal, qreal> & range );

        /**
         * \brief Set the boundaries of the visible value space displayed in vertical direction.
         *
         * This is also known as the vertical viewport.
         *
         * By default the vertical range is adjusted to the range covered by the model's data,
         * see setAutoAdjustVerticalRangeToData for details.
         * Calling setVerticalRange with a valid range disables this default automatic adjusting,
         * while on the other hand automatic adjusting will set these ranges.
         *
         * To disable use of this range you can either pass an empty pair by using the default
         * constructor QPair() or you can set setting both values to the same which constitutes
         * a null range.
         *
         * \note By default the visible data range often is larger than the
         * range calculated from the data model (or set by setHoriz.|Vert.Range(), resp.).
         * This is due to the built-in grid calculation feature: The visible start/end
         * values get adjusted so that they match a main-grid line.
         * You can turn this feature off for any of the four bounds by calling
         * GridAttributes::setAdjustBoundsToGrid() for either the global grid-attributes
         * or for the horizontal/vertical attrs separately.
         *
         * \note If you use user defined vertical ranges together with logarithmic scale, only
         * positive values are supported. If you set it to negative values, the result is undefined.
         *
         * \param range a pair of values representing the smalles and the largest
         * vertical value space coordinate displayed.
         *
         * \sa setAutoAdjustVerticalRangeToData, setHorizontalRange
         * \sa GridAttributes::setAdjustBoundsToGrid()
         */
        void setVerticalRange( const QPair<qreal, qreal> & range );

        /**
         * @return The largest and smallest visible horizontal value space
         * value. If this is not explicitly set,or if both values are the same,
         * the plane will use the union of the dataBoundaries of all
         * associated diagrams.
         * \see KDChart::AbstractDiagram::dataBoundaries
         */
        QPair<qreal, qreal> horizontalRange() const;

        /**
         * @return The largest and smallest visible horizontal value space
         * value. If this is not explicitly set, or if both values are the same,
         * the plane will use the union of the dataBoundaries of all
         * associated diagrams.
         * \see KDChart::AbstractDiagram::dataBoundaries
         */
        QPair<qreal, qreal> verticalRange() const;

        /**
         * \brief Automatically adjust horizontal range settings to the ranges covered by
         * the model's values, when ever the data have changed, and then emit horizontalRangeAutomaticallyAdjusted.
         *
         * By default the horizontal range is adjusted automatically, if more than 67 percent of
         * the available horizontal space would be empty otherwise.
         *
         * Range setting is adjusted if more than \c percentEmpty percent of the horizontal
         * space covered by the coordinate plane would otherwise be empty.
         * Automatic range adjusting can happen, when either all of the data are positive or all are negative.
         *
         * Set percentEmpty to 100 to disable automatic range adjusting.
         *
         * \param percentEmpty The maximal percentage of horizontal space that may be empty.
         *
         * \sa horizontalRangeAutomaticallyAdjusted
         * \sa autoAdjustHorizontalRangeToData, adjustRangesToData
         * \sa setHorizontalRange, setVerticalRange
         * \sa setAutoAdjustVerticalRangeToData
         */
        void setAutoAdjustHorizontalRangeToData( unsigned int percentEmpty = 67 );

        /**
         * \brief Automatically adjust vertical range settings to the ranges covered by
         * the model's values, when ever the data have changed, and then emit verticalRangeAutomaticallyAdjusted.
         *
         * By default the vertical range is adjusted automatically, if more than 67 percent of
         * the available vertical space would be empty otherwise.
         *
         * Range setting is adjusted if more than \c percentEmpty percent of the horizontal
         * space covered by the coordinate plane would otherwise be empty.
         * Automatic range adjusting can happen, when either all of the data are positive or all are negative.
         *
         * Set percentEmpty to 100 to disable automatic range adjusting.
         *
         * \param percentEmpty The maximal percentage of horizontal space that may be empty.
         *
         * \sa verticalRangeAutomaticallyAdjusted
         * \sa autoAdjustVerticalRangeToData, adjustRangesToData
         * \sa setHorizontalRange, setVerticalRange
         * \sa setAutoAdjustHorizontalRangeToData
         */
        void setAutoAdjustVerticalRangeToData( unsigned int percentEmpty = 67  );

        /**
         * \brief Returns the maximal allowed percent of the horizontal
         * space covered by the coordinate plane that may be empty.
         *
         * \return A percent value indicating how much of the horizontal space may be empty.
         * If more than this is empty, automatic range adjusting is applied.
         * A return value of 100 indicates that no such automatic adjusting is done at all.
         *
         * \sa setAutoAdjustHorizontalRangeToData, adjustRangesToData
         */
        unsigned int autoAdjustHorizontalRangeToData() const;

        /**
         * \brief Returns the maximal allowed percent of the vertical
         * space covered by the coordinate plane that may be empty.
         *
         * \return A percent value indicating how much of the vertical space may be empty.
         * If more than this is empty, automatic range adjusting is applied.
         * A return value of 100 indicates that no such automatic adjusting is done at all.
         *
         * \sa setAutoAdjustVerticalRangeToData, adjustRangesToData
         */
        unsigned int autoAdjustVerticalRangeToData() const;


        /**
         * Set the attributes to be used for grid lines drawn in horizontal
         * direction (or in vertical direction, resp.).
         *
         * To disable horizontal grid painting, for example, your code should like this:
         * \code
         * GridAttributes ga = plane->gridAttributes( Qt::Horizontal );
         * ga.setGridVisible( false );
         * plane-setGridAttributes( Qt::Horizontal, ga );
         * \endcode
         *
         * \note setGridAttributes overwrites the global attributes that
         * were set by AbstractCoordinatePlane::setGlobalGridAttributes.
         * To re-activate these global attributes you can call
         * resetGridAttributes.
         *
         * \sa resetGridAttributes, gridAttributes
         * \sa setAutoAdjustGridToZoom
         * \sa AbstractCoordinatePlane::setGlobalGridAttributes
         * \sa hasOwnGridAttributes
         */
        void setGridAttributes( Qt::Orientation orientation, const GridAttributes & );

        /**
         * Reset the attributes to be used for grid lines drawn in horizontal
         * direction (or in vertical direction, resp.).
         * By calling this method you specify that the global attributes set by
         * AbstractCoordinatePlane::setGlobalGridAttributes be used.
         *
         * \sa setGridAttributes, gridAttributes
         * \sa setAutoAdjustGridToZoom
         * \sa AbstractCoordinatePlane::globalGridAttributes
         * \sa hasOwnGridAttributes
         */
        void resetGridAttributes( Qt::Orientation orientation );

        /**
         * \return The attributes used for grid lines drawn in horizontal
         * direction (or in vertical direction, resp.).
         *
         * \note This function always returns a valid set of grid attributes:
         * If no special grid attributes were set foe this orientation
         * the global attributes are returned, as returned by
         * AbstractCoordinatePlane::globalGridAttributes.
         *
         * \sa setGridAttributes
         * \sa resetGridAttributes
         * \sa AbstractCoordinatePlane::globalGridAttributes
         * \sa hasOwnGridAttributes
         */
        const GridAttributes gridAttributes( Qt::Orientation orientation ) const;

        /**
         * \return Returns whether the grid attributes have been set for the
         * respective direction via setGridAttributes( orientation ).
         *
         * If false, the grid will use the global attributes set
         * by AbstractCoordinatePlane::globalGridAttributes (or the default
         * attributes, resp.)
         *
         * \sa setGridAttributes
         * \sa resetGridAttributes
         * \sa AbstractCoordinatePlane::globalGridAttributes
         */
        bool hasOwnGridAttributes( Qt::Orientation orientation ) const;

        /**
         * Disable / re-enable the built-in grid adjusting feature.
         *
         * By default additional lines will be drawn in a Linear grid when zooming in.
         *
         * \sa autoAdjustGridToZoom, setGridAttributes
         */
        void setAutoAdjustGridToZoom( bool autoAdjust );

        /**
         * Return the status of the built-in grid adjusting feature.
         *
         * \sa setAutoAdjustGridToZoom
         */
#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
        const bool autoAdjustGridToZoom() const;
#else
        bool autoAdjustGridToZoom() const;
#endif

        AxesCalcMode axesCalcModeY() const;
        AxesCalcMode axesCalcModeX() const;

        /** Specifies the calculation modes for all axes */
        void setAxesCalcModes( AxesCalcMode mode );
        /** Specifies the calculation mode for all Ordinate axes */
        void setAxesCalcModeY( AxesCalcMode mode );
        /** Specifies the calculation mode for all Abscissa axes */
        void setAxesCalcModeX( AxesCalcMode mode );

        /** reimpl */
        virtual void paint( QPainter* );

        /** reimpl */
        AbstractCoordinatePlane* sharedAxisMasterPlane( QPainter* p = 0 );

        /**
         * Returns the currently visible data range. Might be greater than the
         * range of the grid.
         */
        QRectF visibleDataRange() const;

        /**
         * Returns the logical area, i.e., the rectangle defined by the very top
         * left and very bottom right coordinate.
         */
        QRectF logicalArea() const;

        /**
         * Returns the (physical) area occupied by the diagram. Unless zoom is applied
         * (which is also true when a fixed data coordinate / space relation is used),
         * \code diagramArea() == drawingArea() \endcode .
         * \sa setFixedDataCoordinateSpaceRelation
         * \sa drawingArea
         */
        QRectF diagramArea() const;

        /**
         * Returns the visible part of the diagram area, i.e.
         * \code diagramArea().intersected( drawingArea() ) \endcode
         * \sa diagramArea
         */
        QRectF visibleDiagramArea() const;

        /**
         * Sets whether the horizontal range should be reversed or not, i.e.
         * small values to the left and large values to the right (the default)
         * or vice versa.
         * \param reverse Whether the horizontal range should be reversed or not
         */
        void setHorizontalRangeReversed( bool reverse );

        /**
         * \return Whether the horizontal range is reversed or not
         */
        bool isHorizontalRangeReversed() const;

        /**
         * Sets whether the vertical range should be reversed or not, i.e.
         * small values at the bottom and large values at the top (the default)
         * or vice versa.
         * \param reverse Whether the vertical range should be reversed or not
         */
        void setVerticalRangeReversed( bool reverse );

        /**
         * \return Whether the vertical range is reversed or not
         */
        bool isVerticalRangeReversed() const;

        /**
         * reimplement from AbstractCoordinatePlane
         */
        void setGeometry( const QRect& r );

    public Q_SLOTS:
        /**
         * \brief Adjust both, horizontal and vertical range settings to the
         * ranges covered by the model's data values.
         *
         * \sa setHorizontalRange, setVerticalRange
         * \sa adjustHorizontalRangeToData, adjustVerticalRangeToData
         * \sa setAutoAdjustHorizontalRangeToData, setAutoAdjustVerticalRangeToData
         */
        void adjustRangesToData();

        /**
         * Adjust horizontal range settings to the ranges covered by the model's data values.
         * \sa adjustRangesToData
         */
        void adjustHorizontalRangeToData();

        /**
         * Adjust vertical range settings to the ranges covered by the model's data values.
         * \sa adjustRangesToData
         */
        void adjustVerticalRangeToData();


    protected:
        QRectF getRawDataBoundingRectFromDiagrams() const;
        QRectF adjustedToMaxEmptyInnerPercentage(
                const QRectF& r, unsigned int percentX, unsigned int percentY ) const;
        virtual QRectF calculateRawDataBoundingRect() const;
        virtual DataDimensionsList getDataDimensionsList() const;
        // the whole drawing area, includes diagrams and axes, but maybe smaller
        // than (width, height):
        virtual QRectF drawingArea() const;
        const QPointF translateBack( const QPointF& screenPoint ) const;
        void paintEvent ( QPaintEvent* );
        void layoutDiagrams();
        bool doneSetZoomFactorX( double factor );
        bool doneSetZoomFactorY( double factor );
        bool doneSetZoomCenter( const QPointF& center );

        void handleFixedDataCoordinateSpaceRelation( const QRectF& geometry );

    protected Q_SLOTS:
        void slotLayoutChanged( AbstractDiagram* );

    private:
        void setHasOwnGridAttributes(
            Qt::Orientation orientation, bool on );
    };

}

#endif
