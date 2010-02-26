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
#ifndef KRCHARTDATA_H
#define KRCHARTDATA_H

#include "krobjectdata.h"
#include <QRect>
#include <qdom.h>
#include "krsize.h"
#include "krpos.h"
#include <KDChartWidget>
#include "KoReportData.h"

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/

namespace Scripting
{
class Chart;
}

class KRChartData : public KRObjectData
{
public:
    KRChartData();
    KRChartData(QDomNode & element);
    ~KRChartData();
    virtual KRChartData * toChart();
    virtual int type() const;
    KDChart::Widget *widget() {
        return m_chartWidget;
    }

    /**
    @brief Perform the query for the chart and set the charts data
    */
    void populateData();
    void setConnection(KoReportData*);

    /**
    @brief Set the value of a field from the master (report) data set
    This data will be used when retrieving the data for the chart
    as the values in a 'where' clause.
    */
    void setLinkData(QString, QVariant);

    /**
    @brief Return the list of master fields
    The caller will use this to set the current value for each field
    at that stage in the report
    */
    QStringList masterFields();

protected:

    KoProperty::Property * m_dataSource;
    KoProperty::Property * m_font;
    KoProperty::Property * m_chartType;
    KoProperty::Property * m_chartSubType;
    KoProperty::Property * m_threeD;
    KoProperty::Property * m_colorScheme;
    KoProperty::Property * m_aa;
    KoProperty::Property * m_xTitle;
    KoProperty::Property * m_yTitle;

    KoProperty::Property *m_backgroundColor;
    KoProperty::Property *m_displayLegend;

    KoProperty::Property *m_linkMaster;
    KoProperty::Property *m_linkChild;

    KDChart::Widget *m_chartWidget;

    void set3D(bool);
    void setAA(bool);
    void setColorScheme(const QString &);
    void setAxis(const QString&, const QString&);
    void setBackgroundColor(const QColor&);
    void setLegend(bool, const QStringList &legends = QStringList());

private:
    virtual void createProperties();
    static int RTTI;
    KoReportData *m_reportData;

    friend class KoReportPreRendererPrivate;
    friend class Scripting::Chart;

    QMap<QString, QVariant> m_links; //Map of field->value for child/master links

};

#endif
