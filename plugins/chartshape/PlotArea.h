/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008-2010 Inge Wallin    <inge@lysator.liu.se>

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

#ifndef KCHART_PLOTAREA_H
#define KCHART_PLOTAREA_H

// Qt
#include <QObject>
#include <QList>

// ChartShape
#include "ChartShape.h"

namespace KDChart {
    class CartesianCoordinatePlane;
    class PolarCoordinatePlane;
    class RadarCoordinatePlane;
}


class Ko3dScene;


namespace KChart {

/**
 * @brief The PlotArea class is the central chart element. It plots the data and draws the axes.
 * 
 * There always exists exactly one instance of this class, owned by
 * the chart shape. The plot area itself takes ownership of the axes
 * and the chart wall.
 * 
 * 3D support is not yet fully implemented, a chart floor is not
 * supported at all yet.
 * 
 * This class also plays a central role when loading from or saving to
 * ODF. Though it does not handle anything in particular itself, it
 * utilizes the DataSet, ChartTableModel, Axis, Surface, and
 * ChartProxyModel classes to handle ODF data and properties embedded
 * in the <chart:plotarea> element.
 */

class CHARTSHAPELIB_EXPORT PlotArea : public QObject, public KoShape
{
    friend class Surface;
    friend class Axis;
    Q_OBJECT
    
public:
    PlotArea(ChartShape *parent);
    ~PlotArea();
    
    void plotAreaInit();
    
    ChartProxyModel *proxyModel() const;

    ChartType    chartType() const;
    ChartSubtype chartSubType() const;
    void         setChartType(ChartType type);
    void         setChartSubType(ChartSubtype subType);
    
    QList<Axis*>    axes() const;
    QList<DataSet*> dataSets() const;
    int             dataSetCount() const;
    bool            addAxis(Axis *axis);
    bool            removeAxis(Axis *axis);

    // TODO: Rename this into primaryXAxis()
    Axis *xAxis() const;
    // TODO: Rename this into primaryYAxis()
    Axis *yAxis() const;
    Axis *secondaryXAxis() const;
    Axis *secondaryYAxis() const;

    bool isThreeD() const;
    Ko3dScene *threeDScene() const;

    /**
     * Determines from what range of cells the data in this chart
     * comes from. This region also contains the name of the sheet.
     * See table:cell-range-address, ODF v1.2, $18.595
     */
    CellRegion cellRangeAddress() const;

    /**
     * Determines whether x and y axis are swapped. Default is 'false'.
     * See chart:vertical attribute in ODF v1.2, $19.63
     *
     * FIXME: This is exactly the opposite of what ODF defines. ODF says
     * vertical="true" is a regular bar chart, "false" a column chart.
     * So this specifies whether the axis is *not* swapped.
     */
    bool isVertical() const;
    int gapBetweenBars() const;
    int gapBetweenSets() const;

    /**
     * Defines at what angle, relative to the right-most point
     * of a pie or ring chart, the first slice is going to be drawn,
     * going counter-clockwise.
     * See chart:angle-offset property, as defined in ODF v1.2.
     */
    qreal pieAngleOffset() const;

    void setGapBetweenBars(int percent);
    void setGapBetweenSets(int percent);

    /**
     * @see pieAngleOffset
     */
    void setPieAngleOffset(qreal angle);

    bool loadOdf(const KoXmlElement &plotAreaElement, KoShapeLoadingContext &context);
    bool loadOdfSeries(const KoXmlElement &seriesElement, KoShapeLoadingContext &context);
    
    void saveOdf(KoShapeSavingContext &context) const;
    void saveOdfSubType(KoXmlWriter &bodyWriter, KoGenStyle &plotAreaStyle) const;
    
    
    void setThreeD(bool threeD);

    /**
     * @see cellRangeAddress
     */
    void setCellRangeAddress(const CellRegion &region);

    /**
     * @see isVertical
     */
    void setVertical(bool vertical);
    
    ChartShape *parent() const;

    void paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    
    bool registerKdDiagram(KDChart::AbstractDiagram *diagram);
    bool deregisterKdDiagram(KDChart::AbstractDiagram *diagram);
    
    void relayout() const;
    
public slots:
    void requestRepaint() const;
    void proxyModelStructureChanged();
    void plotAreaUpdate() const;
    
signals:
    void gapBetweenBarsChanged(int);
    void gapBetweenSetsChanged(int);
    void pieAngleOffsetChanged(qreal);

private:
    void paintPixmap(QPainter &painter, const KoViewConverter &converter);

    // For class Axis
    KDChart::CartesianCoordinatePlane *kdCartesianPlane(Axis *axis = 0) const;
    KDChart::PolarCoordinatePlane *kdPolarPlane() const;
    KDChart::RadarCoordinatePlane *kdRadarPlane() const;
    KDChart::Chart *kdChart() const;
    
    class Private;
    Private *const d;
};

} // Namespace KChart

#endif // KCHART_PLOTAREA_H

