/*
    Copyright (C) 2011 Silvio Heinrich <plassy@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <QPaintDevice>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTransform>
#include <QList>
#include <cmath>

#include <kis_config.h>

#include "kis_color_selector.h"

static const int MIN_NUM_HUE_PIECES       = 1;
static const int MAX_NUM_HUE_PIECES       = 48;
static const int MIN_NUM_LIGHT_PIECES     = 1;
static const int MAX_NUM_LIGHT_PIECES     = 30;
static const int MIN_NUM_SATURATION_RINGS = 1;
static const int MAX_NUM_SATURATION_RINGS = 20;

KisColorSelector::KisColorSelector(QWidget* parent, KisColor::Type type):
    QWidget(parent),
    m_colorSpace(type),
    m_inverseSaturation(false),
    m_relativeLight(false),
    m_light(0.5f),
    m_selectedColorRole(Acs::Foreground),
    m_clickedRing(-1)
{
    recalculateRings(9, 12);
    recalculateAreas(9);
    selectColor(KisColor(Qt::red, KisColor::HSY));

    using namespace std::placeholders; // For _1 placeholder
    auto function = std::bind(&KisColorSelector::slotUpdateColorAndPreview, this, _1);
    m_updateColorCompressor.reset(new ColorCompressorType(20 /* ms */, function));
}

void KisColorSelector::setColorSpace(KisColor::Type type)
{
    m_colorSpace    = type;
    m_selectedColor = KisColor(m_selectedColor, m_colorSpace);
    update();
}

void KisColorSelector::setNumLightPieces(int num)
{
    num = qBound(MIN_NUM_LIGHT_PIECES, num, MAX_NUM_LIGHT_PIECES);

    recalculateAreas(quint8(num));

    if (m_selectedLightPiece >= 0)
        m_selectedLightPiece = getLightIndex(m_selectedColor.getX());

    update();
}

void KisColorSelector::setNumPieces(int num)
{
    num = qBound(MIN_NUM_HUE_PIECES, num, MAX_NUM_HUE_PIECES);

    recalculateRings(quint8(getNumRings()), quint8(num));

    if (m_selectedPiece >= 0)
        m_selectedPiece = getHueIndex(m_selectedColor.getH() * PI2);

    update();
}

void KisColorSelector::setNumRings(int num)
{
    num = qBound(MIN_NUM_SATURATION_RINGS, num, MAX_NUM_SATURATION_RINGS);

    recalculateRings(quint8(num), quint8(getNumPieces()));

    if (m_selectedRing >= 0)
        m_selectedRing = getSaturationIndex(m_selectedColor.getS());

    update();
}

void KisColorSelector::selectColor(const KisColor& color)
{
    m_selectedColor      = KisColor(color, m_colorSpace);
    m_selectedPiece      = getHueIndex(m_selectedColor.getH() * PI2);
    m_selectedRing       = getSaturationIndex(m_selectedColor.getS());
    m_selectedLightPiece = getLightIndex(m_selectedColor.getX());
    update();
}

void KisColorSelector::setFgColor(const KisColor& fgColor)
{
    m_fgColor = KisColor(fgColor, m_colorSpace);
    update();
}

void KisColorSelector::setBgColor(const KisColor& bgColor)
{
    m_bgColor = KisColor(bgColor, m_colorSpace);
    update();
}

void KisColorSelector::resetRings()
{
    for(int i=0; i<m_colorRings.size(); ++i)
        m_colorRings[i].angle = 0.0f;

    update();
}

void KisColorSelector::resetLight()
{
    m_light              = (m_colorSpace == KisColor::HSV) ? 1.0f : 0.5f;
    m_selectedLightPiece = getLightIndex(m_light);
    update();
}

void KisColorSelector::resetSelectedRing()
{
    if (m_selectedRing >= 0) {
        m_colorRings[m_selectedRing].angle = 0.0f;
        update();
    }
}

void KisColorSelector::setLight(float light, bool relative)
{
    m_light = qBound(0.0f, light, 1.0f);

    m_selectedColor.setX(getLight(m_light, m_selectedColor.getH(), relative));
    m_relativeLight      = relative;
    m_selectedLightPiece = getLightIndex(m_selectedColor.getX());
    update();
}

void KisColorSelector::setInverseSaturation(bool inverse)
{
    if (m_inverseSaturation != inverse) {
        m_selectedRing      = (getNumRings()-1) - m_selectedRing;
        m_inverseSaturation = inverse;
        recalculateRings(quint8(getNumRings()), quint8(getNumPieces()));
        update();
    }
}

QPointF KisColorSelector::mapCoord(const QPointF& pt, const QRectF& rect) const
{
    qreal w = rect.width()  / 2.0;
    qreal h = rect.height() / 2.0;
    qreal x = pt.x() - (rect.x() + w);
    qreal y = pt.y() - (rect.y() + h);
    return QPointF(x/w, y/h);
}

qint8 KisColorSelector::getLightIndex(const QPointF& pt) const
{
    if (m_lightStripArea.contains(pt.toPoint(), true)) {
        qreal t = (pt.x() - m_lightStripArea.x()) / qreal(m_lightStripArea.width());
        t = (pt.y() - m_lightStripArea.y()) / qreal(m_lightStripArea.height());

        return qint8(t * getNumLightPieces());
    }

    return -1;
}

qint8 KisColorSelector::getLightIndex(qreal light) const
{
    light = qreal(1) - qBound(qreal(0), light, qreal(1));
    return qint8(qRound(light * (getNumLightPieces()-1)));
}

qreal KisColorSelector::getLight(qreal light, qreal hue, bool relative) const
{
    if (relative) {
        KisColor color(hue, 1.0f, m_colorSpace);
        qreal    cl = color.getX();
        light = (light * 2.0f) - 1.0f;
        return (light < 0.0f) ? (cl + cl*light) : (cl + (1.0f-cl)*light);
    }

    return light;
}

qreal KisColorSelector::getLight(const QPointF& pt) const
{
    qint8 clickedLightPiece = getLightIndex(pt);

    if (clickedLightPiece >= 0) {
        if (getNumLightPieces() > 1) {
            return 1.0 - (qreal(clickedLightPiece) / qreal(getNumLightPieces()-1));
        }
        return 1.0 - (qreal(pt.y()) / qreal(m_lightStripArea.height()));
    }

    return qreal(0);
}

qint8 KisColorSelector::getHueIndex(Radian hue, Radian shift) const
{
    hue -= shift;
    qreal partSize = 1.0 / qreal(getNumPieces());
    return qint8(qRound(hue.scaled(0.0f, 1.0f) / partSize) % getNumPieces());
}

qreal KisColorSelector::getHue(int hueIdx, Radian shift) const
{
    Radian hue = (qreal(hueIdx) / qreal(getNumPieces())) * PI2;
    hue += shift;
    return hue.scaled(0.0f, 1.0f);
}

qint8 KisColorSelector::getSaturationIndex(qreal saturation) const
{
    saturation = qBound(qreal(0), saturation, qreal(1));
    saturation = m_inverseSaturation ? (qreal(1) - saturation) : saturation;
    return qint8(saturation * qreal(getNumRings() - 1));
}

qint8 KisColorSelector::getSaturationIndex(const QPointF& pt) const
{
    qreal length = std::sqrt(pt.x()*pt.x() + pt.y()*pt.y());

    for(int i=0; i<m_colorRings.size(); ++i) {
        if (length >= m_colorRings[i].innerRadius && length < m_colorRings[i].outerRadius)
            return qint8(i);
    }

    return -1;
}

qreal KisColorSelector::getSaturation(int saturationIdx) const
{
    qreal sat = qreal(saturationIdx) / qreal(getNumRings()-1);
    return m_inverseSaturation ? (1.0 - sat) : sat;
}

void KisColorSelector::recalculateAreas(quint8 numLightPieces)
{
    const qreal LIGHT_STRIP_RATIO = 0.075;

    int width      = QWidget::width();
    int height     = QWidget::height();
    int size       = qMin(width, height);
    int stripThick = int(size * LIGHT_STRIP_RATIO);

    width -= stripThick;

    size = qMin(width, height);

    int x = (width  - size) / 2;
    int y = (height - size) / 2;

    m_renderArea     = QRect(x+stripThick, y, size, size);
    m_lightStripArea = QRect(0, 0, stripThick, QWidget::height());

    m_renderBuffer   = QImage(size, size, QImage::Format_ARGB32);
    m_numLightPieces = numLightPieces;
}

void KisColorSelector::recalculateRings(quint8 numRings, quint8 numPieces)
{
    m_colorRings.resize(numRings);
    m_numPieces = numPieces;

    for(int i=0; i<numRings; ++i) {
        qreal innerRadius = qreal(i)   / qreal(numRings);
        qreal outerRadius = qreal(i+1) / qreal(numRings);
        qreal saturation  = qreal(i)   / qreal(numRings-1);

        createRing(m_colorRings[i], numPieces, innerRadius, outerRadius+0.001);
        m_colorRings[i].saturation = m_inverseSaturation ? (1.0 - saturation) : saturation;
    }
}

void KisColorSelector::createRing(ColorRing& ring, quint8 numPieces, qreal innerRadius, qreal outerRadius)
{
    int numParts = qMax<int>(numPieces, 1);

    ring.innerRadius = innerRadius;
    ring.outerRadius = outerRadius;
    ring.pieced.resize(numParts);

    qreal  partSize = 360.0 / qreal(numParts);
    QRectF outerRect(-outerRadius, -outerRadius, outerRadius*2.0, outerRadius*2.0);
    QRectF innerRect(-innerRadius, -innerRadius, innerRadius*2.0, innerRadius*2.0);

    for(int i=0; i<numParts; ++i) {
        qreal aBeg  = partSize*i;
        qreal aEnd  = aBeg + partSize;

        aBeg -= partSize / 2.0;
        aEnd -= partSize / 2.0;

        ring.pieced[i] = QPainterPath();
        ring.pieced[i].arcMoveTo(innerRect, aBeg);
        ring.pieced[i].arcTo(outerRect, aBeg, partSize);
        ring.pieced[i].arcTo(innerRect, aEnd,-partSize);
    }
}

void KisColorSelector::requestUpdateColorAndPreview(const KisColor &color, Acs::ColorRole role)
{
    m_updateColorCompressor->start(qMakePair(color, role));
}

void KisColorSelector::slotUpdateColorAndPreview(QPair<KisColor, Acs::ColorRole> color)
{
    const bool selectAsFgColor = color.second == Acs::Foreground;

    if (selectAsFgColor) { m_fgColor = color.first; }
    else                 { m_bgColor = color.first; }

    m_selectedColor          = color.first;
    m_selectedColorRole = color.second;

    if (selectAsFgColor) { emit sigFgColorChanged(m_selectedColor); }
    else                 { emit sigBgColorChanged(m_selectedColor); }
}

void KisColorSelector::drawRing(QPainter& painter, KisColorSelector::ColorRing& ring, const QRect& rect)
{
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.resetTransform();
    painter.translate(rect.width()/2, rect.height()/2);

    if (ring.pieced.size() > 1) {
        painter.rotate(-ring.getShift().degrees());
        painter.scale(rect.width()/2, rect.height()/2);
        painter.setPen(Qt::NoPen);

        QBrush brush(Qt::SolidPattern);

        for(int i=0; i<ring.pieced.size(); ++i) {
            float hue = float(i) / float(ring.pieced.size()) + ring.getShift().scaled(0.0f, 1.0f);
            hue = (hue >= 1.0f) ? (hue - 1.0f) : hue;
            hue = (hue <  0.0f) ? (hue + 1.0f) : hue;

            KisColor color(hue, 1.0f, m_colorSpace);
            color.setS(ring.saturation);
            color.setX(getLight(m_light, hue, m_relativeLight));

            brush.setColor(color.getQColor());
            painter.fillPath(ring.pieced[i], brush);
        }
    }
    else {
        KisColor colors[7] = {
            KisColor(Qt::red    , m_colorSpace),
            KisColor(Qt::yellow , m_colorSpace),
            KisColor(Qt::green  , m_colorSpace),
            KisColor(Qt::cyan   , m_colorSpace),
            KisColor(Qt::blue   , m_colorSpace),
            KisColor(Qt::magenta, m_colorSpace),
            KisColor(Qt::red    , m_colorSpace)
        };

        QConicalGradient gradient(0, 0, 0);

        for(int i=0; i<=6; ++i) {
            qreal hue = float(i) / 6.0f;
            colors[i].setS(ring.saturation);
            colors[i].setX(getLight(m_light, hue, m_relativeLight));
            gradient.setColorAt(hue, colors[i].getQColor());
        }

        painter.scale(rect.width()/2, rect.height()/2);
        painter.fillPath(ring.pieced[0], QBrush(gradient));
    }

    painter.resetTransform();
}

void KisColorSelector::drawOutline(QPainter& painter, const QRect& rect)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.resetTransform();
    painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    painter.scale(rect.width()/2, rect.height()/2);
    painter.setPen(QPen(QBrush(Qt::gray), 0.005));

    if (getNumPieces() > 1) {
        for(int i=0; i<getNumRings(); ++i) {
            painter.resetTransform();
            painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
            painter.scale(rect.width()/2, rect.height()/2);
            painter.rotate(-m_colorRings[i].getShift().degrees());

            for(int j=0; j<m_colorRings[i].pieced.size(); ++j)
                painter.drawPath(m_colorRings[i].pieced[j]);
        }

        if (m_selectedRing >= 0 && m_selectedPiece >= 0) {
            painter.resetTransform();
            painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
            painter.rotate(-m_colorRings[m_selectedRing].getShift().degrees());
            painter.scale(rect.width()/2, rect.height()/2);

            painter.setPen(QPen(QBrush(Qt::red), 0.01));
            painter.drawPath(m_colorRings[m_selectedRing].pieced[m_selectedPiece]);
        }
    }
    else {
        for(int i=0; i<getNumRings(); ++i) {
            qreal rad = m_colorRings[i].outerRadius;
            painter.drawEllipse(QRectF(-rad, -rad, rad*2.0, rad*2.0));
        }
    }

    if (m_selectedRing >= 0) {
        qreal iRad = m_colorRings[m_selectedRing].innerRadius;
        qreal oRad = m_colorRings[m_selectedRing].outerRadius;

        painter.setPen(QPen(QBrush(Qt::red), 0.005));
        painter.drawEllipse(QRectF(-iRad, -iRad, iRad*2.0, iRad*2.0));
        painter.drawEllipse(QRectF(-oRad, -oRad, oRad*2.0, oRad*2.0));

        if (getNumPieces() <= 1) {
            float c = std::cos(-m_selectedColor.getH() * PI2);
            float s = std::sin(-m_selectedColor.getH() * PI2);
            painter.drawLine(QPointF(c*iRad, s*iRad), QPointF(c*oRad, s*oRad));
        }
    }
}

void KisColorSelector::drawLightStrip(QPainter& painter, const QRect& rect)
{
    bool     isVertical = true;
    qreal    penSize    = qreal(qMin(QWidget::width(), QWidget::height())) / 200.0;
    KisColor color(m_selectedColor);

    painter.resetTransform();

    if (getNumLightPieces() > 1) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QBrush(Qt::red), penSize));

        QTransform matrix;
        matrix.translate(rect.x(), rect.y());
        matrix.scale(rect.width(), rect.height());

        for(int i=0; i<getNumLightPieces(); ++i) {
            float  t1    = float(i)   / float(getNumLightPieces());
            float  t2    = float(i+1) / float(getNumLightPieces());
            float  light = 1.0f - (float(i) / float(getNumLightPieces()-1));
            float  diff  = t2 - t1;// + 0.001;
            QRectF r     = isVertical ? QRectF(0.0, t1, 1.0, diff) : QRect(t1, 0.0, diff, 1.0);

            color.setX(getLight(light, color.getH(), m_relativeLight));

            r = matrix.mapRect(r);
            painter.fillRect(r, color.getQColor());

            if (i == m_selectedLightPiece)
                painter.drawRect(r);
        }
    }
    else {
        int size = isVertical ? rect.height() : rect.width();
        painter.setRenderHint(QPainter::Antialiasing, false);

        if (isVertical) {
            for(int i=0; i<size; ++i) {
                int   y     = rect.y() + i;
                float light = 1.0f - (float(i) / float(size-1));
                color.setX(getLight(light, color.getH(), m_relativeLight));
                painter.setPen(color.getQColor());
                painter.drawLine(rect.left(), y, rect.right(), y);
            }
        }
        else {
            for(int i=0; i<size; ++i) {
                int   x     = rect.x() + i;
                float light = 1.0f - (float(i) / float(size-1));
                color.setX(getLight(light, color.getH(), m_relativeLight));
                painter.setPen(color.getQColor());
                painter.drawLine(x, rect.top(), x, rect.bottom());
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QBrush(Qt::red), penSize));
        float t = 1.0f - m_light;

        if (isVertical) {
            int y = rect.y() + int(size * t);
            painter.drawLine(rect.left(), y, rect.right(), y);
        }
        else {
            int x = rect.x() + int(size * t);
            painter.drawLine(x, rect.top(), x, rect.bottom());
        }
    }
}

void KisColorSelector::paintEvent(QPaintEvent* /*event*/)
{
    // 0 red    -> (1,0,0)
    // 1 yellow -> (1,1,0)
    // 2 green  -> (0,1,0)
    // 3 cyan   -> (0,1,1)
    // 4 blue   -> (0,0,1)
    // 5 maenta -> (1,0,1)
    // 6 red    -> (1,0,0)

    m_renderBuffer.fill(0);

    QPainter imgPainter(&m_renderBuffer);
    QPainter wdgPainter(this);

    QRect fgRect(0, 0                  , QWidget::width(), QWidget::height()/2);
    QRect bgRect(0, QWidget::height()/2, QWidget::width(), QWidget::height()/2);
    wdgPainter.fillRect(fgRect, m_fgColor.getQColor());
    wdgPainter.fillRect(bgRect, m_bgColor.getQColor());

    for(int i=0; i<m_colorRings.size(); ++i)
        drawRing(imgPainter, m_colorRings[i], m_renderArea);

    wdgPainter.drawImage(m_renderArea, m_renderBuffer);

    drawOutline   (wdgPainter, m_renderArea);
    drawLightStrip(wdgPainter, m_lightStripArea);
}

void KisColorSelector::mousePressEvent(QMouseEvent* event)
{
    m_clickPos       = mapCoord(event->localPos(), m_renderArea);
    m_mouseMoved     = false;
    m_pressedButtons = event->buttons();
    m_clickedRing    = getSaturationIndex(m_clickPos);

    qint8 clickedLightPiece = getLightIndex(event->localPos());

    if (clickedLightPiece >= 0) {
        setLight(getLight(event->localPos()), m_relativeLight);
        m_selectedLightPiece = clickedLightPiece;
        requestUpdateColorAndPreview(m_selectedColor, Acs::buttonsToRole(Qt::NoButton, m_pressedButtons));
        m_mouseMoved   = true;
    }
    else if (m_clickedRing >= 0) {
        if (getNumPieces() > 1) {
            for(int i=0; i<getNumRings(); ++i)
                m_colorRings[i].setTemporaries(m_selectedColor);
        }
        else {
            Radian angle = std::atan2(m_clickPos.x(), m_clickPos.y()) - RAD_90;
            m_selectedColor.setH(angle.scaled(0.0f, 1.0f));
            m_selectedColor.setS(getSaturation(m_clickedRing));
            m_selectedColor.setX(getLight(m_light, m_selectedColor.getH(), m_relativeLight));
            requestUpdateColorAndPreview(m_selectedColor, Acs::buttonsToRole(Qt::NoButton, m_pressedButtons));
            m_selectedRing = m_clickedRing;
            m_mouseMoved   = true;
            update();
        }
    }
}

void KisColorSelector::mouseMoveEvent(QMouseEvent* event)
{
    QPointF dragPos = mapCoord(event->localPos(), m_renderArea);
    qint8   clickedLightPiece = getLightIndex(event->localPos());

    if (clickedLightPiece >= 0) {
        setLight(getLight(event->localPos()), m_relativeLight);
        m_selectedLightPiece = clickedLightPiece;
        requestUpdateColorAndPreview(m_selectedColor, m_selectedColorRole);
    }

    if (m_clickedRing < 0)
        return;

    if (getNumPieces() > 1) {
        float angle     = std::atan2(dragPos.x(), dragPos.y()) - std::atan2(m_clickPos.x(), m_clickPos.y());
        float dist      = std::sqrt(dragPos.x()*dragPos.x() + dragPos.y()*dragPos.y()) * 0.80f;
        float threshold = 5.0f * (1.0f-(dist*dist));

        if (qAbs(angle * TO_DEG) >= threshold || m_mouseMoved) {
            bool selectedRingMoved = true;

            if (m_pressedButtons & Qt::RightButton) {
                selectedRingMoved                 = m_clickedRing == m_selectedRing;
                m_colorRings[m_clickedRing].angle = m_colorRings[m_clickedRing].tmpAngle + angle;
            }
            else for(int i=0; i<getNumRings(); ++i)
                m_colorRings[i].angle = m_colorRings[i].tmpAngle + angle;

            if (selectedRingMoved) {
                KisColor color = m_colorRings[m_clickedRing].tmpColor;
                Radian   angle = m_colorRings[m_clickedRing].getMovedAngel() + (color.getH()*PI2);
                color.setH(angle.scaled(0.0f, 1.0f));
                color.setX(getLight(m_light, color.getH(), m_relativeLight));

                m_selectedPiece = getHueIndex(angle, m_colorRings[m_clickedRing].getShift());
                requestUpdateColorAndPreview(color, m_selectedColorRole);
            }

            m_mouseMoved = true;
        }
    }
    else {
        Radian angle = std::atan2(dragPos.x(), dragPos.y()) - RAD_90;
        m_selectedColor.setH(angle.scaled(0.0f, 1.0f));
        m_selectedColor.setX(getLight(m_light, m_selectedColor.getH(), m_relativeLight));
        requestUpdateColorAndPreview(m_selectedColor, m_selectedColorRole);
    }

    update();
}

void KisColorSelector::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (!m_mouseMoved && m_clickedRing >= 0) {
        Radian angle = std::atan2(m_clickPos.x(), m_clickPos.y()) - RAD_90;

        m_selectedRing  = m_clickedRing;
        m_selectedPiece = getHueIndex(angle, m_colorRings[m_clickedRing].getShift());

        if (getNumPieces() > 1)
            m_selectedColor.setH(getHue(m_selectedPiece, m_colorRings[m_clickedRing].getShift()));
        else
            m_selectedColor.setH(angle.scaled(0.0f, 1.0f));

        m_selectedColor.setS(getSaturation(m_selectedRing));
        m_selectedColor.setX(getLight(m_light, m_selectedColor.getH(), m_relativeLight));

        requestUpdateColorAndPreview(m_selectedColor, Acs::buttonsToRole(Qt::NoButton, m_pressedButtons));
    }
    else if (m_mouseMoved)
        requestUpdateColorAndPreview(m_selectedColor, m_selectedColorRole);

    m_clickedRing = -1;
    update();
}

void KisColorSelector::resizeEvent(QResizeEvent* /*event*/)
{
    recalculateAreas(quint8(getNumLightPieces()));
}

void KisColorSelector::saveSettings()
{
    KisConfig cfg(false);
    cfg.writeEntry("ArtColorSel.ColorSpace" , qint32(m_colorSpace));
    cfg.writeEntry("ArtColorSel.NumRings"   , m_colorRings.size());
    cfg.writeEntry("ArtColorSel.RingPieces" , qint32(m_numPieces));
    cfg.writeEntry("ArtColorSel.LightPieces", qint32(m_numLightPieces));

    cfg.writeEntry("ArtColorSel.InversedSaturation", m_inverseSaturation);
    cfg.writeEntry("ArtColorSel.RelativeLight"     , m_relativeLight);
    cfg.writeEntry("ArtColorSel.Light"             , m_light);

    cfg.writeEntry("ArtColorSel.SelColorH", m_selectedColor.getH());
    cfg.writeEntry("ArtColorSel.SelColorS", m_selectedColor.getS());
    cfg.writeEntry("ArtColorSel.SelColorX", m_selectedColor.getX());
    cfg.writeEntry("ArtColorSel.SelColorA", m_selectedColor.getA());

    QList<float> angles;

    for(int i=0; i<m_colorRings.size(); ++i)
        angles.push_back(m_colorRings[i].angle.value());

    cfg.writeList("ArtColorSel.RingAngles", angles);
}

void KisColorSelector::loadSettings()
{
    KisConfig cfg(true);
    setColorSpace(KisColor::Type(cfg.readEntry<qint32>("ArtColorSel.ColorSpace" , KisColor::HSY)));

    setNumLightPieces(cfg.readEntry("ArtColorSel.LightPieces", 19));

    m_selectedColor.setH(cfg.readEntry<float>("ArtColorSel.SelColorH", 0.0f));
    m_selectedColor.setS(cfg.readEntry<float>("ArtColorSel.SelColorS", 0.0f));
    m_selectedColor.setX(cfg.readEntry<float>("ArtColorSel.SelColorX", 0.0f));
    m_selectedColor.setA(1.0f);

    setInverseSaturation(cfg.readEntry<bool>("ArtColorSel.InversedSaturation", false));
    setLight(cfg.readEntry<float>("ArtColorSel.Light", 0.5f), cfg.readEntry<bool>("ArtColorSel.RelativeLight", false));

    recalculateRings(
                cfg.readEntry("ArtColorSel.NumRings"  , 11),
                cfg.readEntry("ArtColorSel.RingPieces", 12)
                );

    QList<float> angles = cfg.readList<float>("ArtColorSel.RingAngles");

    for (int i = 0; i < m_colorRings.size(); ++i) {
        if (i < angles.size() && i < m_colorRings.size()) {
            m_colorRings[i].angle = angles[i];
        }
    }

    selectColor(m_selectedColor);
    update();
}
