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

#include "AxisCommand.h"

// KDE
#include <kdebug.h>
#include <klocalizedstring.h>

// KChart
#include "Axis.h"

using namespace KChart;
using namespace KDChart;


AxisCommand::AxisCommand(Axis* axis, ChartShape* chart)
    : m_chart(chart)
    , m_axis(axis)
{
    m_newShowTitle = m_axis->title()->isVisible();
    m_newTitleText = m_axis->titleText();
    m_newShowGridLines = m_axis->showMajorGrid();
    m_newUseLogarithmicScaling = m_axis->scalingIsLogarithmic();
    m_newLabelsFont = m_axis->font();

}

AxisCommand::~AxisCommand()
{
}

void AxisCommand::redo()
{
    // save the old type
    m_oldShowTitle = m_axis->title()->isVisible();
    m_oldTitleText = m_axis->titleText();
    m_oldShowGridLines = m_axis->showMajorGrid();
    m_oldUseLogarithmicScaling = m_axis->scalingIsLogarithmic();
    m_oldLabelsFont = m_axis->font();
    /*m_oldStepWidth = m_axis->majorInterval();
    m_oldSubStepWidth = m_axis->minorInterval();
    m_oldUseAutomaticStepWidth = m_axis->useAutomaticMajorInterval();
    m_oldUseAutomaticSubStepWidth = m_axis->useAutomaticMinorInterval();*/

    if (m_oldShowTitle == m_newShowTitle && m_oldTitleText == m_newTitleText && m_oldShowGridLines == m_newShowGridLines
            && m_oldUseLogarithmicScaling == m_newUseLogarithmicScaling && m_oldLabelsFont == m_newLabelsFont)
        return;

    // Actually do the work
    m_axis->title()->setVisible(m_newShowTitle);
    m_axis->setTitleText(m_newTitleText);
    m_axis->setShowMajorGrid(m_newShowGridLines);
    m_axis->setShowMinorGrid(m_newShowGridLines);
    m_axis->setScalingLogarithmic(m_oldUseLogarithmicScaling);
    m_axis->setFont(m_newLabelsFont);
    m_axis->setFontSize(m_newLabelsFont.pointSize());
    /*m_axis->setMajorInterval(m_newStepWidth);
    m_axis->setMinorInterval(m_newSubStepWidth);
    m_axis->setUseAutomaticMajorInterval(m_newUseAutomaticStepWidth);
    m_axis->setUseAutomaticMinorInterval(m_newUseAutomaticSubStepWidth);*/
    m_chart->update();
}

void AxisCommand::undo()
{
    if (m_oldShowTitle == m_newShowTitle && m_oldTitleText == m_newTitleText && m_oldShowGridLines == m_newShowGridLines
            && m_oldUseLogarithmicScaling == m_newUseLogarithmicScaling && m_oldLabelsFont == m_newLabelsFont)
        return;

    m_axis->title()->setVisible(m_oldShowTitle);
    m_axis->setTitleText(m_oldTitleText);
    m_axis->setShowMajorGrid(m_oldShowGridLines);
    m_axis->setShowMinorGrid(m_oldShowGridLines);
    m_axis->setScalingLogarithmic(m_oldUseLogarithmicScaling);
    m_axis->setFont(m_oldLabelsFont);
    m_axis->setFontSize(m_oldLabelsFont.pointSize());
    /*m_axis->setMajorInterval(m_oldStepWidth);
    m_axis->setMinorInterval(m_oldSubStepWidth);
    m_axis->setUseAutomaticMajorInterval(m_oldUseAutomaticStepWidth);
    m_axis->setUseAutomaticMinorInterval(m_oldUseAutomaticSubStepWidth);*/
    m_chart->update();
}

void AxisCommand::setAxisShowTitle(bool show)
{
    m_newShowTitle = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Axis Title"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Axis Title"));
    }
}

void AxisCommand::setAxisTitle(const QString &title)
{
    m_newTitleText = title;

    setText(i18nc("(qtundo-format)", "Axis Title"));
}

void AxisCommand::setAxisShowGridLines(bool show)
{
    m_newShowGridLines = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Axis Gridlines"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Axis Gridlines"));
    }
}

void AxisCommand::setAxisUseLogarithmicScaling(bool b)
{
    m_newUseLogarithmicScaling = b;

    if (b) {
        setText(i18nc("(qtundo-format)", "Logarithmic Scaling"));
    } else {
        setText(i18nc("(qtundo-format)", "Linear Scaling"));
    }
}

void AxisCommand::setAxisStepWidth(qreal width)
{
    m_newStepWidth = width;

    setText(i18nc("(qtundo-format)", "Axis step width"));
}

void AxisCommand::setAxisSubStepWidth(qreal width)
{
    m_newSubStepWidth = width;

    setText(i18nc("(qtundo-format)", "Axis substep width"));
}

void AxisCommand::setAxisUseAutomaticStepWidth(bool automatic)
{
    m_newShowGridLines = automatic;

    if (automatic) {
        setText(i18nc("(qtundo-format)", "Automatic step width"));
    } else {
        setText(i18nc("(qtundo-format)", "Manual step width"));
    }
}

void AxisCommand::setAxisUseAutomaticSubStepWidth(bool automatic)
{
    m_newShowGridLines = automatic;

    if (automatic) {
        setText(i18nc("(qtundo-format)", "Automatic substep width"));
    } else {
        setText(i18nc("(qtundo-format)", "Manual substep width"));
    }
}

void AxisCommand::setAxisLabelsFont(const QFont &font)
{
    m_newLabelsFont = font;

    setText(i18nc("(qtundo-format)", "Axis Label Font"));
}
