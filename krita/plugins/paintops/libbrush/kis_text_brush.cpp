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

#include "kis_gbr_brush.h"
#include "kis_brushes_pipe.h"

class KisTextBrushesPipe : public KisBrushesPipe<KisGbrBrush>
{
public:
    KisTextBrushesPipe() {
        m_charIndex = 0;
        m_currentBrushIndex = 0;
    }

    KisTextBrushesPipe(const KisTextBrushesPipe &rhs)
        : KisBrushesPipe<KisGbrBrush>(rhs)
    {
        m_brushesMap.clear();

        QMapIterator<QChar, KisGbrBrush*> iter(rhs.m_brushesMap);
        while (iter.hasNext()) {
            iter.next();
            m_brushesMap.insert(iter.key(), iter.value());
        }
    }

    void setText(const QString &text, const QFont &font) {
        m_text = text;
        m_charIndex = 0;

        clear();

        for (int i = 0; i < m_text.length(); i++) {
            QChar letter = m_text.at(i);
            QImage image = renderChar(letter, font);
            KisGbrBrush *brush = new KisGbrBrush(image, letter);
            brush->setSpacing(0.1); // support for letter spacing?
            brush->makeMaskImage();

            m_brushesMap.insert(letter, brush);
            KisBrushesPipe<KisGbrBrush>::addBrush(brush);
        }
    }

    static QImage renderChar(const QString& text, const QFont &font) {
        QFontMetrics metric(font);
        QRect rect = metric.boundingRect(text);

        if (rect.isEmpty()) {
            rect = QRect(0, 0, 1, 1); // paint at least something
        }

        QRect paintingRect = rect.translated(-rect.x(), -rect.y());

        QImage renderedChar(paintingRect.size(), QImage::Format_ARGB32);
        QPainter p;
        p.begin(&renderedChar);
        p.setFont(font);
        p.fillRect(paintingRect, Qt::white);
        p.setPen(Qt::black);
        p.drawText(-rect.x(), -rect.y(), text);
        p.end();
        return renderedChar;
    }

    void clear() {
        m_brushesMap.clear();
        KisBrushesPipe<KisGbrBrush>::clear();
    }

    KisGbrBrush* firstBrush() const{
        return m_brushesMap.value(m_text.at(0));
    }

protected:
    int chooseNextBrush(const KisPaintInformation& info) const {
        Q_UNUSED(info);
        return m_currentBrushIndex;
    }
    void updateBrushIndexes() {
        m_charIndex++;
        if (m_charIndex >= m_text.size()) {
            m_charIndex = 0;
        }

        QChar letter = m_text.at(m_charIndex);
        Q_ASSERT(m_brushesMap.contains(letter));

        m_currentBrushIndex = m_brushes.indexOf(m_brushesMap.value(letter));
    }

private:
    QMap<QChar, KisGbrBrush*> m_brushesMap;
    QString m_text;
    int m_charIndex;
    int m_currentBrushIndex;
};


KisTextBrush::KisTextBrush()
    : m_brushesPipe(new KisTextBrushesPipe())
{
    setPipeMode(false);
}

KisTextBrush::KisTextBrush(const KisTextBrush &rhs)
    : KisBrush(rhs),
      m_brushesPipe(new KisTextBrushesPipe(*rhs.m_brushesPipe))
{
}

KisTextBrush::~KisTextBrush()
{
    delete m_brushesPipe;
}

void KisTextBrush::setPipeMode(bool pipe)
{
    setBrushType(pipe ? PIPE_MASK : MASK);
}

bool KisTextBrush::pipeMode() const
{
    return brushType() == PIPE_MASK;
}

void KisTextBrush::setText(const QString& txt)
{
    m_text = txt;
}

void KisTextBrush::setFont(const QFont& font)
{
    m_font = font;
}

QFont KisTextBrush::font()
{
    return m_font;
}

void KisTextBrush::notifyCachedDabPainted()
{
    m_brushesPipe->notifyCachedDabPainted();
}

void KisTextBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    if (brushType() == MASK){
        KisBrush::generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
    } else /* if (brushType() == PIPE_MASK)*/ {
        m_brushesPipe->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
    }
}

KisFixedPaintDeviceSP KisTextBrush::paintDevice(const KoColorSpace * colorSpace, double scale, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    if (brushType() == MASK){
        return KisBrush::paintDevice(colorSpace, scale, angle, info, subPixelX, subPixelY);
    } else /* if (brushType() == PIPE_MASK)*/ {
        return m_brushesPipe->paintDevice(colorSpace, scale, angle, info, subPixelX, subPixelY);
    }
}

void KisTextBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);

    e.setAttribute("type", "kis_text_brush");
    e.setAttribute("spacing", spacing());
    e.setAttribute("text", m_text);
    e.setAttribute("font", m_font.toString());
    e.setAttribute("pipe", (brushType() == PIPE_MASK) ? "true" : "false");
    KisBrush::toXML(doc, e);
}

void KisTextBrush::updateBrush()
{
    Q_ASSERT((brushType() == PIPE_MASK) || (brushType() == MASK));
    if (brushType() == PIPE_MASK) {
        m_brushesPipe->setText(m_text, m_font);
        setImage(m_brushesPipe->firstBrush()->image());
    } else /* if (brushType() == MASK)*/ {
        setImage(KisTextBrushesPipe::renderChar(m_text, m_font));
    }

    resetBoundary();
    setValid(true);
}

quint32 KisTextBrush::brushIndex(const KisPaintInformation& info) const
{
    return brushType() == MASK ? 0 : 1 + m_brushesPipe->brushIndex(info);
}

qint32 KisTextBrush::maskWidth(double scale, double angle, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return brushType() == MASK ?
        KisBrush::maskWidth(scale, angle, subPixelX, subPixelY, info) :
        m_brushesPipe->maskWidth(scale, angle, subPixelX, subPixelY, info);
}

qint32 KisTextBrush::maskHeight(double scale, double angle, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return brushType() == MASK ?
        KisBrush::maskHeight(scale, angle, subPixelX, subPixelY, info) :
        m_brushesPipe->maskHeight(scale, angle, subPixelX, subPixelY, info);
}

void KisTextBrush::setAngle(qreal _angle)
{
    KisBrush::setAngle(_angle);
    m_brushesPipe->setAngle(_angle);
}

void KisTextBrush::setScale(qreal _scale)
{
    KisBrush::setScale(_scale);
    m_brushesPipe->setScale(_scale);
}

void KisTextBrush::setSpacing(double _spacing)
{
    KisBrush::setSpacing(_spacing);
    m_brushesPipe->setSpacing(_spacing);
}

KisBrush* KisTextBrush::clone() const
{
    return new KisTextBrush(*this);
}
