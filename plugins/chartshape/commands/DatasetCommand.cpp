/* This file is part of the KDE project
   Copyright 2012 Brijesh Patel <brijesh3105@gmail.com>

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

#include "DatasetCommand.h"

// KDE
#include <kdebug.h>
#include <klocalizedstring.h>

// KChart
#include "DataSet.h"

using namespace KChart;
using namespace KDChart;


DatasetCommand::DatasetCommand(DataSet* dataSet, ChartShape* chart)
    : m_dataSet(dataSet)
    , m_chart(chart)
{
    m_newType = dataSet->chartType();
    m_newSubtype = dataSet->chartSubType();
    m_newShowCategory = dataSet->valueLabelType().category;
    m_newShowNumber = dataSet->valueLabelType().number;
    m_newShowPercent = dataSet->valueLabelType().percentage;
    m_newShowSymbol = dataSet->valueLabelType().symbol;
    m_newBrushColor = dataSet->brush().color();
    m_newPenColor = dataSet->pen().color();
    m_newMarkerStyle = dataSet->markerStyle();
    m_newAxis = dataSet->attachedAxis();
}

DatasetCommand::~DatasetCommand()
{
}

void DatasetCommand::redo()
{
    // save the old type
    /*m_oldType = m_dataSet->chartType();
    m_oldSubtype = m_dataSet->chartSubType();*/
    m_oldShowCategory = m_dataSet->valueLabelType().category;
    m_oldShowNumber = m_dataSet->valueLabelType().number;
    m_oldShowPercent = m_dataSet->valueLabelType().percentage;
    m_oldShowSymbol = m_dataSet->valueLabelType().symbol;
    m_oldBrushColor = m_dataSet->brush().color();
    m_oldPenColor = m_dataSet->pen().color();
    m_oldMarkerStyle = m_dataSet->markerStyle();
    m_oldAxis = m_dataSet->attachedAxis();

    if (m_oldShowCategory == m_newShowCategory && m_oldShowNumber == m_newShowNumber
            && m_oldShowPercent == m_newShowPercent && m_oldShowSymbol == m_newShowSymbol
            && m_oldBrushColor == m_newBrushColor && m_oldPenColor == m_newPenColor && m_oldMarkerStyle == m_newMarkerStyle)
        return;

    // Actually do the work
    DataSet::ValueLabelType valueLabelType = m_dataSet->valueLabelType();
    valueLabelType.category = m_newShowCategory;
    valueLabelType.number = m_newShowNumber;
    valueLabelType.percentage = m_newShowPercent;
    valueLabelType.symbol = m_newShowSymbol;
    m_dataSet->setValueLabelType(valueLabelType);

    m_dataSet->setBrush(QBrush(m_newBrushColor));
    m_dataSet->setPen(QPen(m_newPenColor));
    m_dataSet->setMarkerStyle(m_newMarkerStyle);
    m_dataSet->setAttachedAxis(m_newAxis);
    m_chart->update();
}

void DatasetCommand::undo()
{
    if (m_oldShowCategory == m_newShowCategory && m_oldShowNumber == m_newShowNumber
            && m_oldShowPercent == m_newShowPercent && m_oldShowSymbol == m_newShowSymbol
            && m_oldBrushColor == m_newBrushColor && m_oldPenColor == m_newPenColor && m_oldMarkerStyle == m_newMarkerStyle)
        return;

    DataSet::ValueLabelType valueLabelType = m_dataSet->valueLabelType();
    valueLabelType.category = m_oldShowCategory;
    valueLabelType.number = m_oldShowNumber;
    valueLabelType.percentage = m_oldShowPercent;
    valueLabelType.symbol = m_oldShowSymbol;
    m_dataSet->setValueLabelType(valueLabelType);

    m_dataSet->setBrush(QBrush(m_oldBrushColor));
    m_dataSet->setPen(QPen(m_oldPenColor));
    m_dataSet->setMarkerStyle(m_oldMarkerStyle);
    m_dataSet->setAttachedAxis(m_oldAxis);
    m_chart->update();
}

void DatasetCommand::setDataSetChartType(ChartType type, ChartSubtype subtype)
{
    m_newType    = type;
    m_newSubtype = subtype;

    setText(i18nc("(qtundo-format)", "Dataset Chart Type"));
}

void DatasetCommand::setDataSetShowCategory(bool show)
{
    m_newShowCategory = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Dataset Category"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Dataset Category"));
    }
}

void DatasetCommand::setDataSetShowNumber(bool show)
{
    m_newShowNumber = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Dataset Number"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Dataset Number"));
    }
}

void DatasetCommand::setDataSetShowPercent(bool show)
{
    m_newShowPercent = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Dataset Percent"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Dataset Percent"));
    }
}

void DatasetCommand::setDataSetShowSymbol(bool show)
{
    m_newShowSymbol = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Dataset Symbol"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Dataset Symbol"));
    }
}

void DatasetCommand::setDataSetBrush(const QColor &color)
{
    m_newBrushColor = color;

    setText(i18nc("(qtundo-format)", "Dataset Brush color"));
}

void DatasetCommand::setDataSetPen(const QColor &color)
{
    m_newPenColor = color;

    setText(i18nc("(qtundo-format)", "Dataset Pen color"));
}

void DatasetCommand::setDataSetMarker(OdfMarkerStyle style)
{
    m_newMarkerStyle = style;

    setText(i18nc("(qtundo-format)", "Dataset Marker style"));
}

void DatasetCommand::setDataSetAxis(Axis *axis)
{
    m_newAxis = axis;

    setText(i18nc("(qtundo-format)", "Dataset Axis"));
}
