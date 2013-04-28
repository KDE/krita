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

#include "LegendCommand.h"

// KDE
#include <kdebug.h>
#include <klocalizedstring.h>

// KChart
#include "Legend.h"

using namespace KChart;
using namespace KDChart;


LegendCommand::LegendCommand(KChart::Legend* legend)
    : m_legend(legend)
{
    m_newFont = legend->font();
    m_newTitle = legend->title();
    m_newFontSize = legend->fontSize();
    m_newExpansion = legend->expansion();
    m_newShowFrame = legend->showFrame();
}

LegendCommand::~LegendCommand()
{
}

void LegendCommand::redo()
{
    // save the old type
    m_oldTitle = m_legend->title();
    m_oldFont = m_legend->font();
    m_oldFontSize = m_legend->fontSize();
    m_oldExpansion = m_legend->expansion();
    m_oldShowFrame = m_legend->showFrame();
    if (m_oldTitle == m_newTitle && m_oldFont == m_newFont && m_oldFontSize == m_newFontSize
            && m_oldExpansion == m_newExpansion && m_oldShowFrame == m_newShowFrame)
        return;

    // Actually do the work
    m_legend->setTitle(m_newTitle);
    m_legend->setFont(m_newFont);
    m_legend->setFontSize(m_newFontSize);
    m_legend->setExpansion(m_newExpansion);
    m_legend->setShowFrame(m_newShowFrame);
    m_legend->update();
}

void LegendCommand::undo()
{
    if (m_oldTitle == m_newTitle && m_oldFont == m_newFont && m_oldFontSize == m_newFontSize
            && m_oldExpansion == m_newExpansion && m_oldShowFrame == m_newShowFrame)
        return;

    m_legend->setTitle(m_oldTitle);
    m_legend->setFont(m_oldFont);
    m_legend->setFontSize(m_oldFontSize);
    m_legend->setExpansion(m_oldExpansion);
    m_legend->setShowFrame(m_oldShowFrame);
    m_legend->update();
}

void LegendCommand::setLegendTitle(const QString &title)
{
    m_newTitle = title;

    setText(i18nc("(qtundo-format)", "Legend Title"));
}

void LegendCommand::setLegendFont(const QFont &font)
{
    m_newFont = font;
    m_newFontSize = font.pointSize();

    setText(i18nc("(qtundo-format)", "Legend Font"));
}

void LegendCommand::setLegendFontSize(int size)
{
    m_newFontSize = size;

    setText(i18nc("(qtundo-format)", "Legend Font size"));
}

void LegendCommand::setLegendExpansion(LegendExpansion expansion)
{
    m_newExpansion = expansion;

    setText(i18nc("(qtundo-format)", "Legend Orientation"));
}

void LegendCommand::setLegendShowFrame(bool show)
{
    m_newShowFrame = show;

    if (show) {
        setText(i18nc("(qtundo-format)", "Show Legend Frame"));
    } else {
        setText(i18nc("(qtundo-format)", "Hide Legend Frame"));
    }
}
