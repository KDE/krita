/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SCRIPTINGKRSCRIPTCHART_H
#define SCRIPTINGKRSCRIPTCHART_H

#include <QObject>
#include <QPointF>
#include <QSizeF>
#include <QColor>

class KRChartData;

namespace Scripting
{

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class Chart : public QObject
{
    Q_OBJECT
public:
    Chart(KRChartData *);

    ~Chart();
public slots:


    /**
    * Get the position of the barcode
    * @return position in points
     */
    QPointF position();

    /**
     * Sets the position of the barcode in points
     * @param Position
     */
    void setPosition(const QPointF&);

    /**
     * Get the size of the barcode
     * @return size in points
     */
    QSizeF size();

    /**
     * Set the size of the barcode in points
     * @param Size
     */
    void setSize(const QSizeF&);

    /**
     * The data source for the chart
     * @return Datasource
     */
    QString dataSource();

    /**
     * Set the data source for the chart
     * @param datasource
     */
    void setDataSource(const QString &);

    /**
     * The 3d status of the chart
     * @return 3d
     */
    bool threeD();

    /**
     * Set the 3d status of the chart
     * @param 3d
     */
    void setThreeD(bool);

    /**
     * The visibility status of the legend
     * @return visibility
     */
    bool legendVisible();

    /**
     * Sets the visibility of the legend
     * @param visible
     */
    void setLegendVisible(bool);

    /**
     * The color scheme used by the chart
     * @return scheme, 0=default, 2=rainbow 3=subdued
     */
    int colorScheme();

    /**
     * Sets the colorscheme of the chart
     * @param scheme 0=default, 2=rainbow 3=subdued
     */
    void setColorScheme(int);

    /**
     * The background color of the chart
     * @return backgroundcolor
     */
    QColor backgroundColor();

    /**
     * Sets the background color of the chart
     * @param backgroundcolor
     */
    void setBackgroundColor(const QColor &);

    /**
     * The title of the X Axis
     * @return xTitle
     */
    QString xAxisTitle();

    /**
     * Set the title of the X Axis
     * @param xTitle
     */
    void setXAxisTitle(const QString &);

    /**
     * The title of the Y Axis
     * @return yTitle
     */
    QString yAxisTitle();

    /**
     * Set the title of the Y Axis
     * @param yTitle
     */
    void setYAxisTitle(const QString &);

private:
    KRChartData* m_chart;

};

}

#endif
