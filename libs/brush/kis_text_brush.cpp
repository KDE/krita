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
#include <kis_dom_utils.h>
#include <kis_threaded_text_rendering_workaround.h>

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
#include <QApplication>
#include <QWidget>
#include <QThread>
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */


class KisTextBrushesPipe : public KisBrushesPipe<KisGbrBrush>
{
public:
    KisTextBrushesPipe() {
        m_charIndex = 0;
        m_currentBrushIndex = 0;
    }

    KisTextBrushesPipe(const KisTextBrushesPipe &rhs)
        : KisBrushesPipe<KisGbrBrush>(), // no copy here!
          m_text(rhs.m_text),
          m_charIndex(rhs.m_charIndex),
          m_currentBrushIndex(rhs.m_currentBrushIndex)
    {
        m_brushesMap.clear();

        QMapIterator<QChar, KisGbrBrushSP> iter(rhs.m_brushesMap);
        while (iter.hasNext()) {
            iter.next();
            KisGbrBrushSP brush(new KisGbrBrush(*iter.value()));
            m_brushesMap.insert(iter.key(), brush);
            KisBrushesPipe<KisGbrBrush>::addBrush(brush);
        }
    }

    void setText(const QString &text, const QFont &font) {
        m_text = text;

        m_charIndex = 0;

        clear();

        for (int i = 0; i < m_text.length(); i++) {

            const QChar letter = m_text.at(i);

            // skip letters that are already present in the brushes pipe
            if (m_brushesMap.contains(letter)) continue;

            QImage image = renderChar(letter, font);
            KisGbrBrushSP brush(new KisGbrBrush(image, letter));
            brush->setSpacing(0.1); // support for letter spacing?
            brush->makeMaskImage();

            m_brushesMap.insert(letter, brush);
            KisBrushesPipe<KisGbrBrush>::addBrush(brush);
        }
    }

    static QImage renderChar(const QString& text, const QFont &font) {
#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
        QWidget *focusWidget = qApp->focusWidget();
        if (focusWidget) {
            QThread *guiThread = focusWidget->thread();
            if (guiThread != QThread::currentThread()) {
                warnKrita << "WARNING: Rendering text in non-GUI thread!"
                           << "That may lead to hangups and crashes on some"
                           << "versions of X11/Qt!";
            }
        }
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

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

    void clear() override {
        m_brushesMap.clear();
        KisBrushesPipe<KisGbrBrush>::clear();
    }

    KisGbrBrushSP firstBrush() const {
        Q_ASSERT(m_text.size() > 0);
        Q_ASSERT(m_brushesMap.size() > 0);
        return m_brushesMap.value(m_text.at(0));
    }

    void notifyStrokeStarted() override {
        m_charIndex = 0;
        updateBrushIndexesImpl();
    }

protected:

    int chooseNextBrush(const KisPaintInformation& info) override {
        Q_UNUSED(info);
        return m_currentBrushIndex;
    }

    int currentBrushIndex() override {
        return m_currentBrushIndex;
    }

    void updateBrushIndexes(const KisPaintInformation& info, int seqNo) override {
        Q_UNUSED(info);

        if (m_text.size()) {
            m_charIndex = (seqNo >= 0 ? seqNo : (m_charIndex + 1)) % m_text.size();
        } else {
            m_charIndex = 0;
        }

        updateBrushIndexesImpl();
    }

private:
    void updateBrushIndexesImpl() {
        if (m_text.isEmpty()) return;

        if (m_charIndex >= m_text.size()) {
            m_charIndex = 0;
        }

        QChar letter = m_text.at(m_charIndex);
        Q_ASSERT(m_brushesMap.contains(letter));

        m_currentBrushIndex = m_brushes.indexOf(m_brushesMap.value(letter));
    }

private:
    QMap<QChar, KisGbrBrushSP> m_brushesMap;
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
    : KisScalingSizeBrush(rhs),
      m_font(rhs.m_font),
      m_text(rhs.m_text),
      m_brushesPipe(new KisTextBrushesPipe(*rhs.m_brushesPipe))
{
}

KisTextBrush::~KisTextBrush()
{
    delete m_brushesPipe;
}

KisTextBrush &KisTextBrush::operator=(const KisTextBrush &rhs)
{
    if (*this != rhs) {
        m_font = rhs.m_font;
        m_text = rhs.m_text;
        m_brushesPipe = new KisTextBrushesPipe(*rhs.m_brushesPipe);
    }
    return *this;
}

KoResourceSP KisTextBrush::clone() const
{
    return KisBrushSP(new KisTextBrush(*this));
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

QString KisTextBrush::text(void) const
{
    return m_text;
}

void KisTextBrush::setFont(const QFont& font)
{
    m_font = font;
}

QFont KisTextBrush::font()
{
    return m_font;
}

void KisTextBrush::notifyStrokeStarted()
{
    m_brushesPipe->notifyStrokeStarted();
}

void KisTextBrush::notifyCachedDabPainted(const KisPaintInformation& info)
{
    m_brushesPipe->notifyCachedDabPainted(info);
}

void KisTextBrush::prepareForSeqNo(const KisPaintInformation &info, int seqNo)
{
    m_brushesPipe->prepareForSeqNo(info, seqNo);
}

void KisTextBrush::generateMaskAndApplyMaskOrCreateDab(
    KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
    KisDabShape const& shape,
    const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    if (brushType() == MASK) {
        KisBrush::generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, shape, info, subPixelX, subPixelY, softnessFactor);
    }
    else { /* if (brushType() == PIPE_MASK)*/
        m_brushesPipe->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, shape, info, subPixelX, subPixelY, softnessFactor);
    }
}

KisFixedPaintDeviceSP KisTextBrush::paintDevice(const KoColorSpace * colorSpace,
    KisDabShape const& shape,
    const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    if (brushType() == MASK) {
        return KisBrush::paintDevice(colorSpace, shape, info, subPixelX, subPixelY);
    }
    else { /* if (brushType() == PIPE_MASK)*/
        return m_brushesPipe->paintDevice(colorSpace, shape, info, subPixelX, subPixelY);
    }
}

void KisTextBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);

    e.setAttribute("type", "kis_text_brush");
    e.setAttribute("spacing", KisDomUtils::toString(spacing()));
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
        setBrushTipImage(m_brushesPipe->firstBrush()->brushTipImage());
    }
    else { /* if (brushType() == MASK)*/
        setBrushTipImage(KisTextBrushesPipe::renderChar(m_text, m_font));
    }

    resetBoundary();
    setValid(true);
}

quint32 KisTextBrush::brushIndex(const KisPaintInformation& info) const
{
    return brushType() == MASK ? 0 : 1 + m_brushesPipe->brushIndex(info);
}

qint32 KisTextBrush::maskWidth(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return brushType() == MASK ?
           KisBrush::maskWidth(shape, subPixelX, subPixelY, info) :
           m_brushesPipe->maskWidth(shape, subPixelX, subPixelY, info);
}

qint32 KisTextBrush::maskHeight(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return brushType() == MASK ?
           KisBrush::maskHeight(shape, subPixelX, subPixelY, info) :
           m_brushesPipe->maskHeight(shape, subPixelX, subPixelY, info);
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

