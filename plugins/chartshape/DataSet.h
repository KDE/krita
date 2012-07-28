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
 * use section's like for example \a brush(int section) where the
 * section refers to the data-point number.
 */

class CHARTSHAPELIB_EXPORT DataSet
{
public:
    DataSet(int dataSetNr);
    ~DataSet();

    // Getter methods
    QString       title() const;
    ChartType     chartType() const;
    ChartSubtype  chartSubType() const;
    Axis         *attachedAxis() const;

    /**
     * Describes ODF attribute chart:data-label-number from §15.32.3
     */
    class ValueLabelType {
    public:
        /// Show value as number.
        bool number;
        /// Show value as percentage.
        bool percentage;
        /// Show category.
        bool category;
        /// Show legend key.
        bool symbol;
        /// Constructor.
        explicit ValueLabelType(bool number = false, bool percentage = false, bool category = false, bool symbol = false) : number(number), percentage(percentage), category(category), symbol(symbol) {}
        /// Returns true if no label will be displayed.
        bool noLabel() const { return !number && !percentage && !category && !symbol; }
    };

    /**
     * Sets the value label type. \see ValueLabelType
     *
     * \param section The data point to set this type for. -1 will set
     * a series-wide value
     */
    void setValueLabelType(const ValueLabelType &type, int section = -1);

    /**
     * \return the value label type.
     * \see ValueLabelType
     *
     * \param section The data point to return the type for. -1 will return
     * the series-wide value
     */
    ValueLabelType valueLabelType(int section = -1) const;
    
    /**
     * Sets the marker attributes of the series
     *
     * \param section The data point to set this type for. -1 will set
     * a series-wide value
     */
    void setMarkerAttributes(const KDChart::MarkerAttributes& attribs, int section = -1);
    
    /**
     * \return the MarkerAttributes.
     * \see ValueLabelType
     *
     * \param section The data point to return the MarkerAttributes for. -1 will return
     * the series-wide value
     */
    KDChart::MarkerAttributes getMarkerAttributes(int section = -1, bool* success = NULL) const;

    // Graphics properties for the visualization of this dataset.
    QPen   pen() const;
    QBrush brush() const;
    OdfMarkerStyle markerStyle() const;
    QIcon markerIcon(OdfMarkerStyle markerStyle);
    KDChart::PieAttributes pieAttributes() const;
    QPen   pen(int section) const;
    QBrush brush(int section) const;
    KDChart::PieAttributes pieAttributes(int section) const;
    KDChart::DataValueAttributes dataValueAttributes(int section = -1) const;
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
    bool markerAutoSet() const;

    // Setter methods
    void setChartType(ChartType type);
    void setChartSubType(ChartSubtype type);
    void setAttachedAxis(Axis *axis);

    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setPen(int section, const QPen &pen);
    void setBrush(int section, const QBrush &brush);
    void setMarkerStyle(OdfMarkerStyle style);
    void setAutoMarker(bool isAuto);

    void setPieExplodeFactor(int factor);
    void setPieExplodeFactor(int section, int factor);

    void setShowMeanValue(bool b);
    void setMeanValuePen(const QPen &pen);

    void setShowLowerErrorIndicator(bool b);
    void setShowUpperErrorIndicator(bool b);
    void setShowErrorIndicators(bool lower, bool upper);
    void setErrorIndicatorPen(const QPen &pen);
    void setErrorCategory(ErrorCategory category);
    void setErrorPercentage(qreal percentage);
    void setErrorMargin(qreal margin);
    void setLowerErrorLimit(qreal limit);
    void setUpperErrorLimit(qreal limit);

    /**
     * Returns the x-data.
     *
     * \param index the unique index that identifies the cell.
     * \param role either Qt::DisplayRole if the content displayed
     * in the cell (aka the displayText()) should be returned or
     * Qt::EditRole if the actual data of the cell (which can be
     * different from what is displayed) should be returned.
     * \return the x-data value.
     */
    QVariant xData(int index, int role = Qt::EditRole) const;

    /**
     * Returns the y-data aka value-data.
     *
     * \param index the unique index that identifies the cell.
     * \param role either Qt::DisplayRole if the content displayed
     * in the cell (aka the displayText()) should be returned or
     * Qt::EditRole if the actual data of the cell (which can be
     * different from what is displayed) should be returned.
     * \return the y-data value.
     */
    QVariant yData(int index, int role = Qt::EditRole) const;

    /**
     * Used for bubble width in bubble charts. May also be referred to as
     * 'z data' in some cases.
     */
    QVariant customData(int index, int role = Qt::EditRole) const;

    QVariant categoryData(int index, int role = Qt::EditRole) const;

    QVariant labelData() const;
    QString defaultLabelData() const;

    CellRegion xDataRegion() const;
    CellRegion yDataRegion() const;
    CellRegion customDataRegion() const;
    CellRegion categoryDataRegion() const;
    CellRegion labelDataRegion() const;
    // TODO: Region for custom colors

    void setXDataRegion(const CellRegion &region);
    void setYDataRegion(const CellRegion &region);
    void setCustomDataRegion(const CellRegion &region);
    void setCategoryDataRegion(const CellRegion &region);
    void setLabelDataRegion(const CellRegion &region);

    int size() const;
    int dimension() const;

    // Called by the proxy model
    void yDataChanged(const QRect &region) const;
    void xDataChanged(const QRect &region) const;
    void customDataChanged(const QRect &region) const;
    void labelDataChanged(const QRect &region) const;
    void categoryDataChanged(const QRect &region) const;

    void setKdChartModel(KDChartModel *model);
    KDChartModel *kdChartModel() const;

    bool loadOdf(const KoXmlElement &n, KoShapeLoadingContext &context);
    bool loadSeriesIntoDataset(const KoXmlElement &n, KoShapeLoadingContext &context);
    /**
     * Saves a series to ODF. Creates a new chart:series element.
     */
    void saveOdf(KoShapeSavingContext &context) const;

private:
    class Private;
    Private *const d;
};

} // Namespace KChart

#endif // KCHART_DATASET_H

