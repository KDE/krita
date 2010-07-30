/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_minimal_shade_selector.h"

#include <QColor>
#include <QVBoxLayout>
//#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "kis_shade_selector_line.h"


KisMinimalShadeSelector::KisMinimalShadeSelector(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    KisShadeSelectorLine* kisssl = new KisShadeSelectorLine(0.2, 0.0, 0.0, this);
    l->addWidget(kisssl);
    m_shadingLines.append(kisssl);

    kisssl = new KisShadeSelectorLine(0.0, 1., 0.0, this);
    l->addWidget(kisssl);
    m_shadingLines.append(kisssl);

    kisssl = new KisShadeSelectorLine(0.0, 0.0, 1., this);
    l->addWidget(kisssl);
    m_shadingLines.append(kisssl);

    l->setSpacing(0);
    l->setMargin(0);
}

void KisMinimalShadeSelector::setCanvas(KisCanvas2 *canvas)
{
    for(int i=0; i<m_shadingLines.size(); i++)
        m_shadingLines.at(i)->setCanvas(canvas);
}

void KisMinimalShadeSelector::setColor(const QColor& color)
{
    for(int i=0; i<m_shadingLines.size(); i++)
        m_shadingLines.at(i)->setColor(color);
}

void KisMinimalShadeSelector::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    int lineCount = cfg.readEntry("minimalShadeSelectorLineCount", 3);
    int lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);

    setMinimumHeight(lineCount*lineHeight+2*lineCount);
    setMaximumHeight(lineCount*lineHeight+2*lineCount);

    for(int i=0; i<m_shadingLines.size(); i++)
        m_shadingLines.at(i)->updateSettings();
}

