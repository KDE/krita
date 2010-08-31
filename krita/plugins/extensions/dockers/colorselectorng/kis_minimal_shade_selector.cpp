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
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "kis_shade_selector_line.h"


KisMinimalShadeSelector::KisMinimalShadeSelector(QWidget *parent) :
    KisColorSelectorBase(parent), m_canvas(0)
{
    setAcceptDrops(true);

    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(0);

    updateSettings();
}

void KisMinimalShadeSelector::setCanvas(KisCanvas2 *canvas)
{
    KisColorSelectorBase::setCanvas(canvas);
    m_canvas=canvas;
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
    KisColorSelectorBase::updateSettings();
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    QString stri = cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0");
    QStringList strili = stri.split(';', QString::SkipEmptyParts);

    int lineCount = strili.size();
    while(lineCount-m_shadingLines.size() > 0) {
        m_shadingLines.append(new KisShadeSelectorLine(this));
        m_shadingLines.last()->setLineNumber(m_shadingLines.size()-1);
        layout()->addWidget(m_shadingLines.last());
    }
    while(lineCount-m_shadingLines.size() < 0) {
        layout()->removeWidget(m_shadingLines.last());
        delete m_shadingLines.takeLast();
    }

    for(int i=0; i<strili.size(); i++) {
        m_shadingLines.at(i)->fromString(strili.at(i));
        if(m_canvas!=0)
            m_shadingLines.at(i)->setCanvas(m_canvas);
    }

    int lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);
    setMinimumHeight(lineCount*lineHeight+2*lineCount);
    setMaximumHeight(lineCount*lineHeight+2*lineCount);

    for(int i=0; i<m_shadingLines.size(); i++)
        m_shadingLines.at(i)->updateSettings();

    setPopupBehaviour(false, false);
}

void KisMinimalShadeSelector::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0,0,width(), height(), QColor(128,128,128));
}

KisColorSelectorBase* KisMinimalShadeSelector::createPopup() const
{
    KisMinimalShadeSelector* popup = new KisMinimalShadeSelector(0);
    popup->resize(350,350);
    return popup;
}
