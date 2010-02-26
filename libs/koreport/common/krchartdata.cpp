/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2010 by Adam Pigg (adam@piggz.co.uk)
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
#include "krchartdata.h"
#include <KoGlobal.h>
#include <KLocale>
#include <KDebug>
#include <kglobalsettings.h>
#include <klocalizedstring.h>

#include <KDChartBarDiagram>
#include <KDChartThreeDBarAttributes>
#include <KDChartLineDiagram>
#include <KDChartThreeDLineAttributes>
#include <KDChartPieDiagram>
#include <KDChartThreeDPieAttributes>
#include <KDChartLegend>
#include <KDChartCartesianAxis>

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <QMotifStyle>

#include <KDChartChart>
#include <KDChartBackgroundAttributes>

typedef QVector<double> datalist;

KRChartData::KRChartData()
{
    m_reportData = 0;
    createProperties();
}

KRChartData::KRChartData(QDomNode & element)
{
    m_reportData = 0;
    createProperties();

    QDomNodeList nl = element.childNodes();

    QString n;
    QDomNode node;
    QDomElement e = element.toElement();
    m_name->setValue(e.attribute("report:name"));
    m_dataSource->setValue(e.attribute("report:data-source"));
    Z = e.attribute("report:z-index").toDouble();
    m_chartType->setValue(e.attribute("report:chart-type").toInt());
    m_chartSubType->setValue(e.attribute("report:chart-sub-type").toInt());
    m_threeD->setValue(e.attribute("report:three-dimensions"));

    m_colorScheme->setValue(e.attribute("report:chart-color-scheme"));
    m_aa->setValue(e.attribute("report:antialiased"));
    m_xTitle->setValue(e.attribute("report:title-x-axis"));
    m_yTitle->setValue(e.attribute("report:title-y-axis"));
    m_backgroundColor->setValue(e.attribute("report:background-color"));
    m_displayLegend->setValue(e.attribute("report:display-legend"));
    m_linkMaster->setValue(e.attribute("report:link-master"));
    m_linkChild->setValue(e.attribute("report:link-child"));

    parseReportRect(e, &m_pos, &m_size);

}


KRChartData::~KRChartData()
{
}

void KRChartData::createProperties()
{
    m_chartWidget = 0;
    m_set = new KoProperty::Set(0, "Chart");

    QStringList strings;
    QList<QVariant> keys;
    QStringList stringkeys;

    m_dataSource = new KoProperty::Property("data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    m_dataSource->setOption("extraValueAllowed", "true");

    m_font = new KoProperty::Property("Font", KGlobalSettings::generalFont(), i18n("Font"), i18n("Field Font"));

    keys << 1 << 2 << 3 << 4 << 5;
    strings << i18n("Bar") << i18n("Line") << i18n("Pie") << i18n("Ring") << i18n("Polar");
    KoProperty::Property::ListData *typeData = new KoProperty::Property::ListData(keys, strings);
    m_chartType = new KoProperty::Property("chart-type", typeData, 1, i18n("Chart Type"));

    keys.clear();
    strings.clear();
    keys << 0 << 1 << 2 << 3;
    strings << i18n("Normal") << i18n("Stacked") << i18n("Percent") << i18n("Rows");

    KoProperty::Property::ListData *subData = new KoProperty::Property::ListData(keys, strings);

    m_chartSubType = new KoProperty::Property("chart-sub-type", subData, 0, i18n("Chart Sub Type"));

    keys.clear();
    strings.clear();
    stringkeys << "default" << "rainbow" << "subdued";
    strings << i18n("Default") << i18n("Rainbow") << i18n("Subdued");
    m_colorScheme = new KoProperty::Property("chart-color-scheme", stringkeys, strings, "default", i18n("Color Scheme"));

    m_threeD = new KoProperty::Property("three-dimensions", QVariant(false),
        i18nc("Three dimensions", "3D"));
    m_aa = new KoProperty::Property("antialiased", QVariant(false), i18n("Antialiased"));

    m_xTitle = new KoProperty::Property("title-x-axis", QString(), i18n("X Axis Title"), i18n("X Axis Title"));
    m_yTitle = new KoProperty::Property("title-y-axis", QString(), i18n("Y Axis Title"), i18n("Y Axis Title"));

    m_displayLegend = new KoProperty::Property("display-legend", true, i18n("Display Legend"), i18n("Display Legend"));

    m_backgroundColor = new KoProperty::Property("background-color", Qt::white,
        i18n("Background Color"), i18n("Background Color"));

    m_linkMaster = new KoProperty::Property("link-master", QString(), i18n("Link Master"),
        i18n("Fields from master data source"));
    m_linkChild = new KoProperty::Property("link-child", QString(), i18n("Link Child"),
        i18n("Fields from child data source"));

    addDefaultProperties();
    m_set->addProperty(m_dataSource);
    m_set->addProperty(m_chartType);
    m_set->addProperty(m_chartSubType);
    m_set->addProperty(m_font);
    m_set->addProperty(m_colorScheme);
    m_set->addProperty(m_threeD);
    m_set->addProperty(m_aa);
    m_set->addProperty(m_xTitle);
    m_set->addProperty(m_yTitle);
    m_set->addProperty(m_backgroundColor);
    m_set->addProperty(m_displayLegend);
    m_set->addProperty(m_linkMaster);
    m_set->addProperty(m_linkChild);

    set3D(false);
    setAA(false);
    setColorScheme("default");
}

void KRChartData::set3D(bool td)
{
    if (m_chartWidget && m_chartWidget->barDiagram()) {
        KDChart::BarDiagram *bar = m_chartWidget->barDiagram();
        bar->setPen(QPen(Qt::black));

        KDChart::ThreeDBarAttributes threed = bar->threeDBarAttributes();
        threed.setEnabled(td);
        threed.setDepth(10);
        threed.setAngle(15);
        threed.setUseShadowColors(true);
        bar->setThreeDBarAttributes(threed);
    }

}
void KRChartData::setAA(bool aa)
{
    if (m_chartWidget && m_chartWidget->diagram()) {
        m_chartWidget->diagram()->setAntiAliasing(aa);
    }
}

void KRChartData::setColorScheme(const QString &cs)
{
    if (m_chartWidget && m_chartWidget->diagram()) {
        if (cs == "rainbow") {
            m_chartWidget->diagram()->useRainbowColors();
        } else if (cs == "subdued") {
            m_chartWidget->diagram()->useSubduedColors();
        } else {
            m_chartWidget->diagram()->useDefaultColors();
        }
    }
}

void KRChartData::setConnection(KoReportData *c)
{
    m_reportData = c;
    populateData();
}

void KRChartData::populateData()
{
    QVector<datalist> data;
    QStringList labels;

    QStringList fn;


    delete m_chartWidget;
    m_chartWidget = 0;

    if (m_reportData) {
        QString src = m_dataSource->value().toString();

        if (!src.isEmpty()) {
            KoReportData *curs = m_reportData->data(src);
            if (curs && curs->open()) {
                fn = curs->fieldNames();
                //resize the data lists to match the number of columns
                int cols = fn.count() - 1;
                if ( cols > 0 ) {
                    data.resize(cols);
                }

                m_chartWidget = new KDChart::Widget();
                //_chartWidget->setStyle ( new QMotifStyle() );

                m_chartWidget->setType((KDChart::Widget::ChartType) m_chartType->value().toInt());
                m_chartWidget->setSubType((KDChart::Widget::SubType) m_chartSubType->value().toInt());
                set3D(m_threeD->value().toBool());
                setAA(m_aa->value().toBool());
                setColorScheme(m_colorScheme->value().toString());
                setBackgroundColor(m_backgroundColor->value().value<QColor>());
                curs->moveFirst();
                //bool status = true;
                do {
                    labels << curs->value(0).toString();
                    for (int i = 1; i <= cols; ++i) {
                        data[i - 1] << curs->value(i).toDouble();
                    }
                } while (curs->moveNext());

                for (int i = 1; i <= cols; ++i) {
                    m_chartWidget->setDataset(i - 1, data[i - 1], fn[i]);
                }

                setLegend(m_displayLegend->value().toBool(), fn);

                //Add the axis
                setAxis(m_xTitle->value().toString(), m_yTitle->value().toString());

                //Add the bottom labels
                if (m_chartWidget->barDiagram() || m_chartWidget->lineDiagram()) {
                    KDChart::AbstractCartesianDiagram *dia = dynamic_cast<KDChart::AbstractCartesianDiagram*>(m_chartWidget->diagram());

                    foreach(KDChart::CartesianAxis* axis, dia->axes()) {
                        if (axis->position() == KDChart::CartesianAxis::Bottom) {
                            axis->setLabels(labels);
                        }
                    }
                }
            } else {
                kDebug() << "Unable to open data set";
            }
            delete curs;
        } else {
            kDebug() << "No source set";
        }
    } else {
        kDebug() << "No connection!";
    }
}

QStringList KRChartData::masterFields()
{
    return m_linkMaster->value().toString().split(',');
}

void KRChartData::setLinkData(QString fld, QVariant val)
{
    kDebug() << "Field: " << fld << "is" << val;
    m_links[fld] = val;
}

void KRChartData::setAxis(const QString& xa, const QString &ya)
{
    Q_ASSERT(m_chartWidget);

    if (m_chartWidget->barDiagram() || m_chartWidget->lineDiagram()) {
        KDChart::AbstractCartesianDiagram *dia = dynamic_cast<KDChart::AbstractCartesianDiagram*>(m_chartWidget->diagram());
        KDChart::CartesianAxis *xAxis = 0;
        KDChart::CartesianAxis *yAxis = 0;

        //delete existing axis
        foreach(KDChart::CartesianAxis* axis, dia->axes()) {
            if (axis->position() == KDChart::CartesianAxis::Bottom) {
                xAxis = axis;
            }
            if (axis->position() == KDChart::CartesianAxis::Left) {
                yAxis = axis;
            }
        }

        if (!xAxis) {
            xAxis =  new KDChart::CartesianAxis(dynamic_cast<KDChart::AbstractCartesianDiagram*>(m_chartWidget->diagram()));
            xAxis->setPosition(KDChart::CartesianAxis::Bottom);
            dia->addAxis(xAxis);
        }

        if (!yAxis) {
            yAxis = new KDChart::CartesianAxis(dynamic_cast<KDChart::AbstractCartesianDiagram*>(m_chartWidget->diagram()));
            yAxis->setPosition(KDChart::CartesianAxis::Left);
            dia->addAxis(yAxis);
        }

        xAxis->setTitleText(xa);
        yAxis->setTitleText(ya);
    }
}

void KRChartData::setBackgroundColor(const QColor&)
{
    //Set the background color
    KDChart::Chart *cht = m_chartWidget->diagram()->coordinatePlane()->parent();

    KDChart::BackgroundAttributes ba;

    ba.setVisible(true);
    ba.setBrush(m_backgroundColor->value().value<QColor>());
    cht->setBackgroundAttributes(ba);
}

void KRChartData::setLegend(bool le, const QStringList &legends)
{
    //Add the legend
    if (m_chartWidget) {
        if (le && ! legends.isEmpty()) {
            m_chartWidget->addLegend(KDChart::Position::East);
            m_chartWidget->legend()->setOrientation(Qt::Horizontal);
            m_chartWidget->legend()->setTitleText("Legend");
            for (uint i = 1; i < (uint)legends.count(); ++i) {
                m_chartWidget->legend()->setText(i - 1, legends[i]);
            }

            m_chartWidget->legend()->setShowLines(true);
        } else {
            if (m_chartWidget->legend()) {
                m_chartWidget->takeLegend(m_chartWidget->legend());
            }
        }
    }
}

// RTTI
int KRChartData::type() const
{
    return RTTI;
}
int KRChartData::RTTI = KRObjectData::EntityChart;
KRChartData * KRChartData::toChart()
{
    return this;
}
