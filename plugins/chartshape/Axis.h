/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_AXIS_H
#define KCHART_AXIS_H


// Qt
#include <QObject>

// KChart
#include "ChartShape.h"


namespace KChart {

enum OdfGridClass {
    OdfMajorGrid,
    OdfMinorGrid
};

/**
 * @brief The Axis class handles axis as well as grid settings.
 * 
 * Data series can be attached to axes that represent an
 * ordinate. This is done to customize the scaling, i.e., the relation
 * in which the data of a series is visualized.
 */

class CHARTSHAPELIB_EXPORT Axis : public QObject
{
    Q_OBJECT
    
public:
    Axis( PlotArea *parent );
    ~Axis();
	
    PlotArea *plotArea() const;
    AxisPosition position() const;
    KoShape *title() const;
    QString titleText() const;
    bool showLabels() const;
    QString id() const;
    AxisDimension dimension() const;
    QList<DataSet*> dataSets() const;
    qreal majorInterval() const;
    qreal minorInterval() const;
    int minorIntervalDivisor() const;
    bool useAutomaticMajorInterval() const;
    bool useAutomaticMinorInterval() const;
    bool showInnerMinorTicks() const;
    bool showOuterMinorTicks() const;
    bool showInnerMajorTicks() const;
    bool showOuterMajorTicks() const;
    bool scalingIsLogarithmic() const;
    bool showMajorGrid() const;
    bool showMinorGrid() const;
    Qt::Orientation orientation();
    QFont font() const;
    bool isVisible() const;
    
    QString categoryDataRegionString() const;
    void setCategoryDataRegionString( const QString &region );
	
    void setPosition( AxisPosition position );
    void setTitleText( const QString &text );
    void setShowLabels( bool show );
    void setDimension( AxisDimension dimension );

    /**
     * Attaches a data set to this axis, adding it to a diagram
     * of its chart type, creating it if necessary
     */
    bool attachDataSet( DataSet *dataSet, bool silent = false );

    /**
     * Detaches a data set from this axis, removing it from the diagram of
     * its chart type, and deleting it if it was the last data set in this diagram.
     */
    bool detachDataSet( DataSet *dataSet, bool silent = false );

    /**
     * Detaches all data sets in this axis, deleting any diagram
     * that this axis might have owned.
     */
    void clearDataSets();

    void setMajorInterval( qreal interval );
    void setMinorInterval( qreal interval );
    void setMinorIntervalDivisor( int divisor );
    void setUseAutomaticMajorInterval( bool automatic );
    void setUseAutomaticMinorInterval( bool automatic );
    void setShowInnerMinorTicks( bool showTicks );
    void setShowOuterMinorTicks( bool showTicks );
    void setShowInnerMajorTicks( bool showTicks );
    void setShowOuterMajorTicks( bool showTicks );
    void setScalingLogarithmic( bool logarithmicScaling );
    void setShowMajorGrid( bool showGrid );
    void setShowMinorGrid( bool showGrid );
    void setThreeD( bool threeD );
    void setFont( const QFont &font );
    void setVisible( bool visible );
    
    bool loadOdf( const KoXmlElement &axisElement, KoShapeLoadingContext &context);
    bool loadOdfChartSubtypeProperties( const KoXmlElement &axisElement,
                                        KoShapeLoadingContext &context );
    void saveOdf( KoShapeSavingContext &context );
    void saveOdfGrid( KoShapeSavingContext &context, OdfGridClass gridClass );

    // KDChart stuff
    KDChart::CartesianAxis *kdAxis() const;
    KDChart::AbstractCoordinatePlane *kdPlane() const;
    
    void plotAreaChartTypeChanged( ChartType chartType );
    void plotAreaChartSubTypeChanged( ChartSubtype chartSubType );
    
    void registerKdAxis( KDChart::CartesianAxis *axis );
    void deregisterKdAxis( KDChart::CartesianAxis *axis );
    
    void update() const;
    void requestRepaint() const;
    void layoutPlanes();
    
public slots:
    void setGapBetweenBars( int percent );
    void setGapBetweenSets( int percent );
    void setPieExplodeFactor( DataSet *dataSet, int percent );
    void setPieAngleOffset( qreal angle );
    
private:
    class Private;
    Private *const d;
};

} // Namespace KChart

#endif
