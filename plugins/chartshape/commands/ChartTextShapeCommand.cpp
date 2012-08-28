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

#include "ChartTextShapeCommand.h"

// KDE
#include <kdebug.h>
#include <klocalizedstring.h>

// KChart
#include "ChartShape.h"
#include "ChartLayout.h"

#include "KoShape.h"

using namespace KChart;

ChartTextShapeCommand::ChartTextShapeCommand(KoShape* textShape, ChartShape *chart, bool isVisible)
    : m_textShape(textShape)
    , m_chart(chart)
    , m_newIsVisible(isVisible)
{
    if (m_newIsVisible) {
        setText(i18nc("(qtundo-format)", "Show Textshape"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Textshape"));
    }
}

ChartTextShapeCommand::~ChartTextShapeCommand()
{
}

void ChartTextShapeCommand::redo()
{
    // save the old type
    m_oldIsVisible = m_textShape->isVisible();

    if (m_oldIsVisible == m_newIsVisible)
        return;

    // Actually do the work
    m_textShape->setVisible(m_newIsVisible);
    m_chart->update();
}

void ChartTextShapeCommand::undo()
{
    if (m_oldIsVisible == m_newIsVisible)
        return;

    m_textShape->setVisible(m_oldIsVisible);
    m_chart->update();
}
