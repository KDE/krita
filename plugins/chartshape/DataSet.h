/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010 Carlos Licea    <carlos@kdab.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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
 * Boston, MA 02110-1301, USA.
 */


#ifndef KCHART_DATASET_H
#define KCHART_DATASET_H


// Qt
#include <QPen>

// KChart
#include "ChartShape.h"
#include "CellRegion.h"

namespace KDChart {
    class DataValueAttributes;
    class PieAttributes;
    class MarkerAttributes;
}

class KoShapeLoadingContext;

namespace KChart {

class KDChartModel;


/**
 * @brief The DataSet class stores properties of a single data series.
 * 
 * A global chart type can be overridden by setting a specific type
 * on a data series.
 * 
 * To change properties of a single data point inside a data series,
 * use section's like for example \a brush( int section ) where the
 * section refers to the data-point number.
 */

class CHARTSHAPELIB_EXPORT DataSet
{
public:
    DataSet( ChartProxyModel *model, int dataSetNr );
    ~DataSet();

    // Getter methods
    QString       title() const;
    ChartType     chartType() const;
    ChartSubtype  chartSubType() const;
    Axis         *attachedAxis() const;

    ChartProxyModel   *model() const;

    /**
     * Describes ODF attribute chart:data-label-number from §15.32.3
     */
    enum ValueLabelType {
        /// No value will be displayed
        NoValueLabel,
        /// The actual value will be displayed
        RealValueLabel,
        /// A percentage in e.g. respect to the sum of all values in a series
        /// will be shown.
        PercentageValueLabel
    };

    /**
     * Sets the value label type. \see ValueLabelType
     *
     * \param section The data point to set this type for. -1 will set
     * a series-wide value
     */
    void setValueLabelType( ValueLabelType type, int section = -1 );    

    /**
     * \return the value label type.
     * \see ValueLabelType
     *
     * \param section The data point to return the type for. -1 will return
     * the series-wide value
     */
    ValueLabelType valueLabelType( int section = -1 ) const;
    
    /**
     * Sets the marker attributes of the series
     *
     * \param section The data point to set this type for. -1 will set
     * a series-wide value
     */
    void setMarkerAttributes( const KDChart::MarkerAttributes& attribs, int section = -1 );
    
    /**
     * \return the MarkerAttributes.
     * \see ValueLabelType
     *
     * \param section The data point to return the MarkerAttributes for. -1 will return
     * the series-wide value
     */
    KDChart::MarkerAttributes getMarkerAttributes( int section = -1, bool* success = NULL ) const;

    // Graphics properties for the visualization of this dataset.
    QPen   pen() const;
    QBrush brush() const;
    KDChart::PieAttributes pieAttributes() const;
    QPen   pen( int section ) const;
    QBrush brush( int section ) const;
    KDChart::PieAttributes pieAttributes( int section ) const;
    KDChart::DataValueAttributes dataValueAttributes( int section = -1 ) const;
    QColor color() const;
    int    number() const;

    bool showMeanValue() const;
    QPen meanValuePen() const;

    bool showLowerErrorIndicator() const;
    bool showUpperErrorIndicator() const;
    QPen errorIndicatorPen() const;
    ErrorCategory errorCategory() const;
    qreal errorPercentage() const;
    qreal errorMargin() const;
    qreal lowerErrorLimit() const;
    qreal upperErrorLimit() const;

    // Setter methods
    void setChartType( ChartType type );
    void setChartSubType( ChartSubtype type );
    void setAttachedAxis( Axis *axis );

    /**
     * \return Whether to display categories as labels or not
     * See ODF's chart:data-label-text attribute from §15.32.3
     */
    bool showLabels( int section = -1 ) const;

    /**
     * \see showLabels
     */
    void setShowLabels( bool showLabels, int section = -1 );

    void setPen( const QPen &pen );
    void setBrush( const QBrush &brush );
    void setPen( int section, const QPen &pen );
    void setBrush( int section, const QBrush &brush );
    void setColor( const QColor &color );
    void setNumber( int num );

    void setPieExplodeFactor( int factor );
    void setPieExplodeFactor( int section, int factor );

    void setShowMeanValue( bool b );
    void setMeanValuePen( const QPen &pen );

    void setShowLowerErrorIndicator( bool b );
    void setShowUpperErrorIndicator( bool b );
    void setShowErrorIndicators( bool lower, bool upper );
    void setErrorIndicatorPen( const QPen &pen );
    void setErrorCategory( ErrorCategory category );
    void setErrorPercentage( qreal percentage );
    void setErrorMargin( qreal margin );
    void setLowerErrorLimit( qreal limit );
    void setUpperErrorLimit( qreal limit );

    QVariant xData( int index ) const;
    QVariant yData( int index ) const;
    QVariant customData( int index ) const;
    QVariant categoryData( int index ) const;
    QVariant labelData() const;

    CellRegion xDataRegion() const;
    CellRegion yDataRegion() const;
    CellRegion customDataRegion() const;
    CellRegion categoryDataRegion() const;
    CellRegion labelDataRegion() const;
    // TODO: Region for custom colors

    void setXDataRegion( const CellRegion &region );
    void setYDataRegion( const CellRegion &region );
    void setCustomDataRegion( const CellRegion &region );
    void setCategoryDataRegion( const CellRegion &region );
    void setLabelDataRegion( const CellRegion &region );

    int size() const;
    int dimension() const;

    void setKdDiagram( KDChart::AbstractDiagram *diagram );
    void setKdDataSetNumber( int number );

    KDChart::AbstractDiagram *kdDiagram() const;

    // Called by the proxy model
    void yDataChanged( int start, int end ) const;
    void xDataChanged( int start, int end ) const;
    void customDataChanged( int start, int end ) const;
    void labelDataChanged() const;
    void categoryDataChanged( int start, int end ) const;

    // FIXME: The parameter 'region' is unused in the methods below
    void yDataChanged( const QRect &region ) const;
    void xDataChanged( const QRect &region ) const;
    void customDataChanged( const QRect &region ) const;
    void labelDataChanged( const QRect &region ) const;
    void categoryDataChanged( const QRect &region ) const;

    void setKdChartModel( KDChartModel *model );
    KDChartModel *kdChartModel() const;
    
    void blockSignals( bool block );

    bool loadOdf( const KoXmlElement &n,
                  KoShapeLoadingContext &context );
    /**
     * Saves a series to ODF. Creates a new chart:series element.
     */
    void saveOdf( KoShapeSavingContext &context ) const;

private:
    class Private;
    Private *const d;
};

} // Namespace KChart

#endif // KCHART_DATASET_H

