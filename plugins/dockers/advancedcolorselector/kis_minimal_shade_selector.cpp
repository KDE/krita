/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_minimal_shade_selector.h"

#include <QColor>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "KoCanvasResourceProvider.h"

#include "kis_shade_selector_line.h"

#include "kis_color_selector_base_proxy.h"


KisMinimalShadeSelector::KisMinimalShadeSelector(QWidget *parent)
    : KisColorSelectorBase(parent)
    , m_canvas(0)
    , m_proxy(new KisColorSelectorBaseProxyObject(this))
{
    setAcceptDrops(true);

    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(0);

    updateSettings();

    setMouseTracking(true);
}

KisMinimalShadeSelector::~KisMinimalShadeSelector()
{
}

void KisMinimalShadeSelector::unsetCanvas()
{
    KisColorSelectorBase::unsetCanvas();
    m_canvas = 0;
}

void KisMinimalShadeSelector::setCanvas(KisCanvas2 *canvas)
{
    KisColorSelectorBase::setCanvas(canvas);
    m_canvas = canvas;
}

void KisMinimalShadeSelector::setColor(const KoColor& color)
{
    m_lastRealColor = color;

    for(int i=0; i<m_shadingLines.size(); i++) {
        m_shadingLines.at(i)->setColor(color);
    }
}

void KisMinimalShadeSelector::updateSettings()
{
    KisColorSelectorBase::updateSettings();
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    QString stri = cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0");
    QStringList strili = stri.split(';', QString::SkipEmptyParts);

    int lineCount = strili.size();
    while(lineCount-m_shadingLines.size() > 0) {
        KisShadeSelectorLine *line = new KisShadeSelectorLine(m_proxy.data(), this);
        m_shadingLines.append(line);
        m_shadingLines.last()->setLineNumber(m_shadingLines.size()-1);
        layout()->addWidget(m_shadingLines.last());
    }
    while(lineCount-m_shadingLines.size() < 0) {
        layout()->removeWidget(m_shadingLines.last());
        delete m_shadingLines.takeLast();
    }

    for(int i=0; i<strili.size(); i++) {
        m_shadingLines.at(i)->fromString(strili.at(i));
    }

    int lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);
    setMinimumHeight(lineCount*lineHeight+2*lineCount);
    setMaximumHeight(lineCount*lineHeight+2*lineCount);

    for(int i=0; i<m_shadingLines.size(); i++)
        m_shadingLines.at(i)->updateSettings();

    setPopupBehaviour(false, false);
}

void KisMinimalShadeSelector::mousePressEvent(QMouseEvent * e)
{
    Q_FOREACH (KisShadeSelectorLine* line, m_shadingLines) {
        QMouseEvent newEvent(e->type(),
                                          line->mapFromGlobal(e->globalPos()),
                                          e->button(),
                                          e->buttons(),
                                          e->modifiers());
        if(line->rect().contains(newEvent.pos()))
            line->mousePressEvent(&newEvent);
    }
    KisColorSelectorBase::mousePressEvent(e);
}

void KisMinimalShadeSelector::mouseMoveEvent(QMouseEvent * e)
{
    Q_FOREACH (KisShadeSelectorLine* line, m_shadingLines) {
        QMouseEvent newEvent(e->type(),
                                          line->mapFromGlobal(e->globalPos()),
                                          e->button(),
                                          e->buttons(),
                                          e->modifiers());
        if(line->rect().contains(newEvent.pos()))
            line->mouseMoveEvent(&newEvent);
    }
    KisColorSelectorBase::mouseMoveEvent(e);
}

void KisMinimalShadeSelector::mouseReleaseEvent(QMouseEvent * e)
{
    Q_FOREACH (KisShadeSelectorLine* line, m_shadingLines) {
        QMouseEvent newEvent(e->type(),
                                          line->mapFromGlobal(e->globalPos()),
                                          e->button(),
                                          e->buttons(),
                                          e->modifiers());

        if(line->rect().contains(newEvent.pos()))
            line->mouseReleaseEvent(&newEvent);
    }
    KisColorSelectorBase::mouseReleaseEvent(e);
}

void KisMinimalShadeSelector::canvasResourceChanged(int key, const QVariant &v)
{
    if(m_colorUpdateAllowed==false)
        return;

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    bool onForeground = cfg.readEntry("shadeSelectorUpdateOnForeground", false);
    bool onBackground = cfg.readEntry("shadeSelectorUpdateOnBackground", true);

    if ((key == KoCanvasResourceProvider::ForegroundColor && onForeground)
        || (key == KoCanvasResourceProvider::BackgroundColor && onBackground)) {

        setColor(v.value<KoColor>());
    }
}

void KisMinimalShadeSelector::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0,0,width(), height(), QColor(128,128,128));
}

KisColorSelectorBase* KisMinimalShadeSelector::createPopup() const
{
    KisMinimalShadeSelector* popup = new KisMinimalShadeSelector(0);
    popup->setColor(m_lastRealColor);
    return popup;
}
