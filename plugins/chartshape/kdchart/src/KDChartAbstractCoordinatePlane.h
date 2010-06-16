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

#ifndef KDCHARTABSTRACTCOORDINATEPLANE_H
#define KDCHARTABSTRACTCOORDINATEPLANE_H

#include <QObject>
#include <QList>

#include "KDChartAbstractArea.h"
#include "KDChartAbstractDiagram.h"
#include "KDChartEnums.h"

namespace KDChart {

    class Chart;
    class GridAttributes;
    class DataDimension;

    typedef QList<DataDimension> DataDimensionsList;

    /**
      * @brief Base class common for all coordinate planes, CartesianCoordinatePlane, PolarCoordinatePlane, TernaryCoordinatePlane
      */
    class KDCHART_EXPORT AbstractCoordinatePlane : public AbstractArea
    {
        Q_OBJECT

        Q_DISABLE_COPY( AbstractCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( AbstractCoordinatePlane, Chart* )

    friend class AbstractGrid;

    public:
        enum AxesCalcMode { Linear, Logarithmic };

    protected:
        explicit AbstractCoordinatePlane ( Chart* parent = 0 );

    public:
        virtual ~AbstractCoordinatePlane();

        /**
         * Adds a diagram to this coordinate plane.
         * @param diagram The diagram to add.
         *
         * \sa replaceDiagram, takeDiagram
         */
        virtual void addDiagram ( AbstractDiagram* diagram );

        /**
         * Replaces the old diagram, or appends the
         * diagram, it there is none yet.
         *
         * @param diagram The diagram to be used instead of the old diagram.
         * This parameter must not be zero, or the method will do nothing.
         *
         * @param oldDiagram The diagram to be removed by the new diagram. This
         * diagram will be deleted automatically. If the parameter is omitted,
         * the very first diagram will be replaced. In case, there was no
         * diagram yet, the new diagram will just be added.
         *
         * \note If you want to re-use the old diagram, call takeDiagram and
         * addDiagram, instead of using replaceDiagram.
         *
         * \sa addDiagram, takeDiagram
         */
        virtual void replaceDiagram ( AbstractDiagram* diagram, AbstractDiagram* oldDiagram = 0 );

        /**
         * Removes the diagram from the plane, without deleting it.
         *
         * The plane no longer owns the diagram, so it is
         * the caller's responsibility to delete the diagram.
         *
         * \sa addDiagram, replaceDiagram
        */
        virtual void takeDiagram( AbstractDiagram* diagram );

        /**
         * @return The first diagram associated with this coordinate plane.
         */
        AbstractDiagram* diagram();

        /**
         * @return The list of diagrams associated with this coordinate plane.
         */
        AbstractDiagramList diagrams();

        /**
         * @return The list of diagrams associated with this coordinate plane.
         */
        ConstAbstractDiagramList diagrams() const;

        /**
         * Distribute the available space among the diagrams and axes.
         */
        virtual void layoutDiagrams() = 0;

        /**
         * Translate the given point in value space coordinates to a position
         * in pixel space.
         * @param diagramPoint The point in value coordinates.
         * @returns The translated point.
         */
        virtual const QPointF translate ( const QPointF& diagramPoint ) const = 0;

        /** \reimpl */
        virtual QSize minimumSizeHint() const;
        /** \reimpl */
        virtual QSizePolicy sizePolicy() const;

        /**
         * @return Whether zooming with a rubber band using the mouse is enabled.
         */
        bool isRubberBandZoomingEnabled() const;

        /**
         * Enables or disables zooming with a rubber band using the mouse.
         */
        void setRubberBandZoomingEnabled( bool enable );

        /**
         * @return The zoom factor in horizontal direction, that is applied
         * to all coordinate transformations.
         */
        virtual double zoomFactorX() const { return 1.0; }

        /**
         * @return The zoom factor in vertical direction, that is applied
         * to all coordinate transformations.
         */
        virtual double zoomFactorY() const { return 1.0; }

        /**
         * Sets both zoom factors in one go.
         * \sa setZoomFactorX,setZoomFactorY
         */
        virtual void setZoomFactors( double factorX, double factorY ) { Q_UNUSED( factorX ); Q_UNUSED( factorY ); }

        /**
         * Sets the zoom factor in horizontal direction, that is applied
         * to all coordinate transformations.
         * @param factor The new zoom factor
         */
        virtual void setZoomFactorX( double  factor ) { Q_UNUSED( factor ); }

        /**
         * Sets the zoom factor in vertical direction, that is applied
         * to all coordinate transformations.
         * @param factor The new zoom factor
         */
        virtual void setZoomFactorY( double factor ) { Q_UNUSED( factor ); }

        /**
         * @return The center point (in value coordinates) of the
         * coordinate plane, that is used for zoom operations.
         */
        virtual QPointF zoomCenter() const { return QPointF(0.0, 0.0); }

        /**
         * Set the point (in value coordinates) to be used as the
         * center point in zoom operations.
         * @param center The point to use.
         */
        virtual void setZoomCenter( const QPointF& center ) { Q_UNUSED( center ); }

        /**
         * Set the grid attributes to be used by this coordinate plane.
         * To disable grid painting, for example, your code should like this:
         * \code
         * GridAttributes ga = plane->globalGridAttributes();
         * ga.setGlobalGridVisible( false );
         * plane->setGlobalGridAttributes( ga );
         * \endcode
         * \sa globalGridAttributes
         * \sa CartesianCoordinatePlane::setGridAttributes
         */
        void setGlobalGridAttributes( const GridAttributes & );

        /**
         * @return The grid attributes used by this coordinate plane.
         * \sa setGlobalGridAttributes
         * \sa CartesianCoordinatePlane::gridAttributes
         */
        GridAttributes globalGridAttributes() const;

        /**
         * Returns the dimensions used for drawing the grid lines.
         *
         * Returned data is the result of (cached) grid calculations,
         * so - if you need that information for your own tasks - make sure to
         * call again this function after every data modification that has changed
         * the data range, since grid calculation is based upon the data range,
         * thus the grid start/end might have changed if the data was changed.
         *
         * @note Returned list will contain different numbers of DataDimension,
         * depending on the kind of coordinate plane used.
         * For CartesianCoordinatePlane two DataDimension are returned: the first
         * representing grid lines in X direction (matching the Abscissa axes)
         * and the second indicating vertical grid lines (or Ordinate axes, resp.).
         *
         * @return The dimensions used for drawing the grid lines.
         * @sa DataDimension
         */
        DataDimensionsList gridDimensionsList();

        /**
         * Set another coordinate plane to be used as the reference plane
         * for this one.
         * @param plane The coordinate plane to be used the reference plane
         * for this one.
         * @see referenceCoordinatePlane
         */
        void setReferenceCoordinatePlane( AbstractCoordinatePlane * plane );

        /**
         * There are two ways, in which planes can be caused to interact, in
         * where they are put layouting wise: The first is the reference plane. If
         * such a reference plane is set, on a plane, it will use the same cell in the
         * layout as that one. In addition to this, planes can share an axis. In that case
         * they will be laid out in relation to each other as suggested by the position
         * of the axis. If, for example Plane1 and Plane2 share an axis at position Left,
         * that will result in the layout: Axis Plane1 Plane 2, vertically. If Plane1
         * also happens to be Plane2's reference plane, both planes are drawn over each
         * other. The reference plane concept allows two planes to share the same space
         * even if neither has any axis, and in case there are shared axis, it is used
         * to decided, whether the planes should be painted on top of each other or
         * laid out vertically or horizontally next to each other.
         * @return The reference coordinate plane associated with this one.
         */
        AbstractCoordinatePlane * referenceCoordinatePlane() const;

        virtual AbstractCoordinatePlane* sharedAxisMasterPlane( QPainter* p = 0 );


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
        /** pure virtual in QLayoutItem
          *
          * \note Do not call this function directly, unless you know
          * exactly what you are doing.  Geometry management is done
          * by KD Chart's internal layouting measures.
          */
        virtual void setGeometry( const QRect& r );
        /** pure virtual in QLayoutItem */
        virtual QRect geometry() const;

        void mousePressEvent( QMouseEvent* event );
        void mouseDoubleClickEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );

        /**
          * Called internally by KDChart::Chart
          */
        void setParent( Chart* parent );
        Chart* parent();
        const Chart* parent() const;

        /**
         * Tests, if a point is visible on the coordinate plane.
         *
         * \note Before calling this function the point must have been translated into coordinate plane space.
         */
#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
        const bool isVisiblePoint( const QPointF& point ) const;
#else
        bool isVisiblePoint( const QPointF& point ) const;
#endif

    public Q_SLOTS:
        /**
          * Calling update() on the plane triggers the global KDChart::Chart::update()
          */
        void update();
        /**
          * Calling relayout() on the plane triggers the global KDChart::Chart::slotRelayout()
          */
        void relayout();
        /**
          * Calling layoutPlanes() on the plane triggers the global KDChart::Chart::slotLayoutPlanes()
          */
        void layoutPlanes();
        /**
         * Used by the chart to clear the cached grid data.
         */
        void setGridNeedsRecalculate();

    Q_SIGNALS:
        /** Emitted when this coordinate plane is destroyed. */
        void destroyedCoordinatePlane( AbstractCoordinatePlane* );

        /** Emitted when plane needs to update its drawings. */
        void needUpdate();

        /** Emitted when plane needs to trigger the Chart's layouting. */
        void needRelayout();

        /** Emitted when plane needs to trigger the Chart's layouting of the coord. planes. */
        void needLayoutPlanes();

        /** Emitted upon change of a property of the Coordinate Plane or any of its components. */
        void propertiesChanged();

        /** Emitted after the geometry of the Coordinate Plane has been changed.
         *  and control has returned to the event loop.
         *
         * Parameters are the the old geometry, the new geometry.
        */
        void geometryChanged( QRect, QRect );

    private:
    Q_SIGNALS:
        // Emitted from inside the setGeometry()
        // This is connected via QueuedConnection to the geometryChanged() Signal
        // that users can connect to safely then.
        void internal_geometryChanged( QRect, QRect );

    protected:
        virtual DataDimensionsList getDataDimensionsList() const = 0;

        //KDCHART_DECLARE_PRIVATE_DERIVED( AbstractCoordinatePlane )
    };

    /**
     * \brief Helper class for one dimension of data, e.g. for the rows in a data model,
     * or for the labels of an axis, or for the vertical lines in a grid.
     *
     * isCalculated specifies whether this dimension's values are calculated or counted.
     * (counted == "Item 1", "Item 2", "Item 3" ...)
     *
     * sequence is the GranularitySequence, as specified at for the respective
     * coordinate plane.
     *
     * Step width is an optional parameter, to be omitted (or set to Zero, resp.)
     * if the step width is unknown.
     *
     * The default c'tor just gets you counted values from 1..10, using step width 1,
     * used by the CartesianGrid, when showing an empty plane without any diagrams.
     */
    class DataDimension{
    public:
        DataDimension()
            : start(         1.0 )
            , end(          10.0 )
            , isCalculated( false )
            , calcMode( AbstractCoordinatePlane::Linear )
            , sequence( KDChartEnums::GranularitySequence_10_20 )
            , stepWidth(    1.0 )
            , subStepWidth( 0.0 )
        {}
        DataDimension( qreal start_,
                       qreal end_,
                       bool isCalculated_,
                       AbstractCoordinatePlane::AxesCalcMode calcMode_,
                       KDChartEnums::GranularitySequence sequence_,
                       qreal stepWidth_=0.0,
                       qreal subStepWidth_=0.0 )
            : start(        start_ )
            , end(          end_ )
            , isCalculated( isCalculated_ )
            , calcMode(     calcMode_ )
            , sequence(     sequence_ )
            , stepWidth(    stepWidth_ )
            , subStepWidth( subStepWidth_ )
        {}
        /**
          * Returns the size of the distance,
          * equivalent to the width() (or height(), resp.) of a QRectF.
          *
          * Note that this value can be negative, e.g. indicating axis labels
          * going in reversed direction.
          */
        qreal distance() const
        {
            return end-start;
        }

        bool operator==( const DataDimension& r ) const
        {
            return
                (start        == r.start) &&
                (end          == r.end) &&
                (sequence     == r.sequence) &&
                (isCalculated == r.isCalculated) &&
                (calcMode     == r.calcMode) &&
                (stepWidth    == r.stepWidth) &&
                (subStepWidth    == r.subStepWidth);
        }

        bool operator!=( const DataDimension& other ) const
        { return !operator==( other ); }


        qreal start;
        qreal end;
        bool  isCalculated;
        AbstractCoordinatePlane::AxesCalcMode calcMode;
        KDChartEnums::GranularitySequence sequence;
        qreal stepWidth;
        qreal subStepWidth;
    };

#if !defined(QT_NO_DEBUG_STREAM)
    QDebug operator<<( QDebug stream, const DataDimension& r );
#endif

}
#endif
