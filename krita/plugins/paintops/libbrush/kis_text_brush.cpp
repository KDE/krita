/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_text_brush.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include "kis_gbr_brush.h"

KisTextBrush::KisTextBrush()
    : KisBrush()
    , m_letterIndex(0)
    , m_currentBrush(0)
{
    setBrushType(MASK);
}

KisTextBrush::KisTextBrush(const QString& txt, const QFont& font, bool pipe)
    : KisBrush()
    , m_letterIndex(0)
    , m_currentBrush(0)
{
    setFont(font);
    setText(txt);
    setPipeMode(pipe);
    updateBrush();
}

KisTextBrush::~KisTextBrush()
{
    clearBrushes();
}

void KisTextBrush::setPipeMode(bool pipe)
{
    if (pipe) {
        setBrushType(PIPE_MASK);
    } else
    {
        setBrushType(MASK);
    }
}


void KisTextBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    if (brushType() == MASK){
        KisBrush::generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
    } else /* if (brushType() == PIPE_MASK)*/ {
        if (m_brushes.isEmpty()) {
            kWarning() << "No brush masks to rendered";
            return;
        }
        selectNextBrush(info);
        m_currentBrush->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
    }
}

void KisTextBrush::selectNextBrush(const KisPaintInformation& info) const
{
    Q_UNUSED(info);
    if (m_letterIndex >= m_txt.length()){
        m_letterIndex = 0;
    }
    Q_ASSERT(m_brushes.contains( m_txt.at(m_letterIndex) ));
    m_currentBrush = m_brushes.value( m_txt.at(m_letterIndex) );
    m_letterIndex++;
}


void KisTextBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);

    e.setAttribute("type", "kis_text_brush");
    e.setAttribute("spacing", spacing());
    e.setAttribute("text", m_txt);
    e.setAttribute("font", m_font.toString());
    e.setAttribute("pipe", (brushType() == PIPE_MASK) ? "true" : "false");
    KisBrush::toXML(doc, e);
}

void KisTextBrush::updateBrush()
{
    Q_ASSERT((brushType() == PIPE_MASK) || (brushType() == MASK));
    if (brushType() == PIPE_MASK) {
        init();
    } else {
        setImage( renderChar(m_txt) );
    }
    resetBoundary();
    setValid(true);
}

QImage KisTextBrush::renderChar(const QString& text)
{
    QFontMetrics metric(m_font);
    int w = metric.width(text);
    int h = metric.height();

    // don't crash, if there is no text
    if (w==0) w=1;
    if (h==0) h=1;

    QPixmap px(w, h);
    QPainter p;
    p.begin(&px);
    p.setFont(m_font);
    p.fillRect(0, 0, w, h, Qt::white);
    p.setPen(Qt::black);
    p.drawText(0, metric.ascent(), text);
    p.end();
    return px.toImage();
}


void KisTextBrush::init()
{
    clearBrushes();
    for (int i = 0; i < m_txt.length();i++) {
        QImage brush = renderChar(m_txt.at(i));
        KisGbrBrush * singleLetter = new KisGbrBrush(brush, m_txt.at(i));
        singleLetter->setSpacing(0.1); // support for letter spacing?
        singleLetter->makeMaskImage();
        m_brushes.insert(m_txt.at(i), singleLetter );
    }
    // TODO: think about alternative to set width and height in
    // generateMaskAndApplyMaskOrCreateDab -- KisBrush API is confusing so might
    // give weird results
    setImage( m_brushes.value(m_txt.at(0))->image() ); //set
}


void KisTextBrush::setAngle(qreal _angle)
{
    KisBrush::setAngle(_angle);
    QMap<QString, KisGbrBrush*>::iterator i;
    for (i = m_brushes.begin(); i!= m_brushes.end(); ++i){
            i.value()->setAngle(_angle);
    }
}


void KisTextBrush::setScale(qreal _scale)
{
    KisBrush::setScale(_scale);
    QMap<QString, KisGbrBrush*>::iterator i;
    for (i = m_brushes.begin(); i!= m_brushes.end(); ++i){
            i.value()->setScale(_scale);
    }
}

void KisTextBrush::clearBrushes()
{
    qDeleteAll(m_brushes.begin(), m_brushes.end());
    m_brushes.clear();
}
