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
#include <kis_vec.h>
#include <cmath>

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
    m_selectedRing(-1),
    m_selectedPiece(-1),
    m_selectedLightPiece(5),
    m_lightStripPos(LSP_RIGHT),
    m_selectedColorIsFgColor(true),
    m_clickedRing(-1)
{
    recalculateAreas(9);
    recalculateRings(9, 12);
    m_selectedColor = KisColor(Qt::red, KisColor::HSY);
    m_selectedColor.setX(0.5f);
}

void KisColorSelector::setColorSpace(KisColor::Type type)
{
    m_colorSpace    = type;
    m_selectedColor = KisColor(m_selectedColor, m_colorSpace);
    update();
}

void KisColorSelector::setLightStripPosition(KisColorSelector::LightStripPos pos)
{
    m_lightStripPos = pos;
    recalculateAreas(quint8(getNumLightPieces()));
    update();
}

void KisColorSelector::setNumLightPieces(int num)
{
    num = qBound(MIN_NUM_LIGHT_PIECES, num, MAX_NUM_LIGHT_PIECES);
    
    if(m_selectedLightPiece >= 0)
        m_selectedLightPiece = getLightIndex(m_selectedColor.getX());
    
    recalculateAreas(quint8(num));
}

void KisColorSelector::setNumPieces(int num)
{
    num = qBound(MIN_NUM_HUE_PIECES, num, MAX_NUM_HUE_PIECES);
    
    if(m_selectedPiece >= 0)
        m_selectedPiece = getHueIndex(m_selectedColor.getH() * Radian::PI2);
    
    recalculateRings(quint8(getNumRings()), quint8(num));
}

void KisColorSelector::setNumRings(int num)
{
    num = qBound(MIN_NUM_SATURATION_RINGS, num, MAX_NUM_SATURATION_RINGS);
    
    if(m_selectedRing >= 0)
        m_selectedRing = qint8((qreal(m_selectedRing) / qreal(getNumRings())) * num);
    
    recalculateRings(quint8(num), quint8(getNumPieces()));
}

void KisColorSelector::selectColor(const KisColor& color)
{
    KisColor col(color, m_colorSpace);
    float hue = col.getH();
    float sat = m_inverseSaturation ? (1.0f - col.getS()) : col.getS();
    
    m_selectedPiece = getHueIndex(hue * Radian::PI2);
    m_selectedRing  = getSaturationIndex(sat);
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
    if(m_selectedRing >= 0) {
        m_colorRings[m_selectedRing].angle = 0.0f;
        update();
    }
}

void KisColorSelector::setLight(float light, bool relative)
{
    m_light         = relative ? qBound(-1.0f, light, 1.0f) : qBound(0.0f, light, 1.0f);
    m_relativeLight = relative;
    update();
}

void KisColorSelector::setInverseSaturation(bool inverse)
{
    if(m_inverseSaturation != inverse) {
        m_selectedRing      = (getNumRings()-1) - m_selectedRing;
        m_inverseSaturation = inverse;
        recalculateRings(quint8(getNumRings()), quint8(getNumPieces()));
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
    if(m_lightStripArea.contains(pt.toPoint(), true)) {
        qreal t = (pt.x() - m_lightStripArea.x()) / qreal(m_lightStripArea.width());
        
        if(m_lightStripPos == LSP_LEFT || m_lightStripPos == LSP_RIGHT)
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

qreal KisColorSelector::getLight(qreal light, qreal hue) const
{
    if(m_relativeLight) {
        KisColor color(hue, 1.0f, m_colorSpace);
        qreal    cl = color.getX();
        light = (light * 2.0f) - 1.0f;
        return (light < 0.0f) ? (cl + cl*light) : (cl + (1.0f-cl)*light);
    }
    
    return light;
}

qreal KisColorSelector::getLight(int lightIdx, qreal hue) const
{
    qreal light = 1.0 - (qreal(lightIdx) / qreal(getNumLightPieces()-1));
    
    if(m_relativeLight) {
        KisColor color(hue, 1.0f, m_colorSpace);
        qreal    cl = color.getX();
        light = (light * 2.0f) - 1.0f;
        return (light < 0.0f) ? (cl + cl*light) : (cl + (1.0f-cl)*light);
    }
    
    return light;
}

qint8 KisColorSelector::getHueIndex(Radian hue, Radian shift) const
{
    hue -= shift;
    qreal partSize = 1.0 / qreal(getNumPieces());
    return qint8(qRound(hue.scaled(0.0f, 1.0f) / partSize) % getNumPieces());
}

qreal KisColorSelector::getHue(int hueIdx, Radian shift) const
{
    Radian hue = (qreal(hueIdx) / qreal(getNumPieces())) * Radian::PI2;
    hue += shift;
    return hue.scaled(0.0f, 1.0f);
}

qint8 KisColorSelector::getSaturationIndex(qreal saturation) const
{
    saturation = qBound(qreal(0.0), saturation, qreal(1.0));
    qreal partSize = 1.0 / qreal(getNumRings());
    return qint8(qRound(saturation / partSize) % getNumRings());
}

qint8 KisColorSelector::getSaturationIndex(const QPointF& pt) const
{
    qreal length = std::sqrt(pt.x()*pt.x() + pt.y()*pt.y());
    
    for(int i=0; i<m_colorRings.size(); ++i) {
        if(length >= m_colorRings[i].innerRadius && length < m_colorRings[i].outerRadius)
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
    
    if(m_lightStripPos == LSP_LEFT || m_lightStripPos == LSP_RIGHT)
        width -= stripThick;
    else
        height -= stripThick;
    
    size = qMin(width, height);
    
    int x = (width  - size) / 2;
    int y = (height - size) / 2;
    
    switch(m_lightStripPos)
    {
    case LSP_LEFT:
        m_renderArea     = QRect(x+stripThick, y, size, size);
        m_lightStripArea = QRect(0, 0, stripThick, QWidget::height());
        break;
        
    case LSP_RIGHT:
        m_renderArea     = QRect(x, y, size, size);
        m_lightStripArea = QRect(QWidget::width()-stripThick, 0, stripThick, QWidget::height());
        break;
        
    case LSP_TOP:
        m_renderArea     = QRect(x, y+stripThick, size, size);
        m_lightStripArea = QRect(0, 0, QWidget::width(), stripThick);
        break;
        
    case LSP_BOTTOM:
        m_renderArea     = QRect(x, y, size, size);
        m_lightStripArea = QRect(0, QWidget::height()-stripThick, QWidget::width(), stripThick);
        break;
    }
    
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
        
        createRing(m_colorRings[i], numPieces, innerRadius, outerRadius+0.001, 0.0);
        m_colorRings[i].saturation = m_inverseSaturation ? (1.0 - saturation) : saturation;
    }
    
    update();
}

void KisColorSelector::createRing(ColorRing& ring, quint8 numPieces, qreal innerRadius, qreal outerRadius, qreal borderSize)
{
    ring.innerRadius = innerRadius;
    ring.outerRadius = outerRadius;
    ring.pieced.resize(numPieces);
    
    int numParts = (ring.pieced.size() <= 1) ? 1 : ring.pieced.size();
    
    borderSize = (numParts == 1) ? 0 : borderSize;
    
    qreal  halfBorder = borderSize / 2.0;
    qreal  partSize   = 360.0 / qreal(numParts);
    QRectF outerRect(-outerRadius, -outerRadius, outerRadius*2.0, outerRadius*2.0);
    QRectF innerRect(-innerRadius, -innerRadius, innerRadius*2.0, innerRadius*2.0);
    
    for(int i=0; i<numParts; ++i) {
        qreal aBeg  = partSize*i + halfBorder;
        qreal aEnd  = aBeg + partSize - borderSize;
        
        aBeg -= partSize / 2.0;
        aEnd -= partSize / 2.0;
        
        ring.pieced[i] = QPainterPath();
        ring.pieced[i].arcMoveTo(innerRect, aBeg);
        ring.pieced[i].arcTo(outerRect, aBeg, partSize - borderSize);
        ring.pieced[i].arcTo(innerRect, aEnd, -(partSize - borderSize));
    }
}

void KisColorSelector::setSelectedColor(const KisColor& color, bool selectAsFgColor, bool emitSignal)
{
    if(selectAsFgColor) { m_fgColor = color; }
    else                { m_bgColor = color; }
    
    m_selectedColor          = color;
    m_selectedColorIsFgColor = selectAsFgColor;
    
    if(emitSignal) {
        if(selectAsFgColor) { emit sigFgColorChanged(m_selectedColor); }
        else                { emit sigBgColorChanged(m_selectedColor); }
    }
}

void KisColorSelector::drawRing(QPainter& painter, KisColorSelector::ColorRing& ring, const QRect& rect)
{
    painter.resetTransform();
    painter.translate(rect.width()/2, rect.height()/2);
    
    if(ring.pieced.size() > 1) {
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
            color.setX(getLight(m_light, hue));
            
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
            qreal hue = float(i)/6.0f;
            colors[i].setS(ring.saturation);
            colors[i].setX(getLight(m_light, hue));
            gradient.setColorAt(hue, colors[i].getQColor());
        }
        
        painter.scale(rect.width()/2, rect.height()/2);
        painter.fillPath(ring.pieced[0], QBrush(gradient));
    }
    
    painter.resetTransform();
}

void KisColorSelector::drawOutline(QPainter& painter, const QRect& rect)
{
    painter.resetTransform();
    painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    painter.scale(rect.width()/2, rect.height()/2);
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QBrush(Qt::gray), 0.005));
    
    for(int i=0; i<getNumRings(); ++i) {
        qreal rad = m_colorRings[i].outerRadius;
        painter.drawEllipse(QRectF(-rad, -rad, rad*2.0, rad*2.0));
    }
    
    painter.setPen(QPen(QBrush(Qt::gray), 0.005));
    
    for(int i=0; i<getNumRings(); ++i) {
        painter.resetTransform();
        painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
        painter.scale(rect.width()/2, rect.height()/2);
        painter.rotate(-m_colorRings[i].getShift().degrees());
        
        for(int j=0; j<getNumPieces(); ++j)
            painter.drawPath(m_colorRings[i].pieced[j]);
    }
    
    if(m_selectedRing >= 0) {
        painter.resetTransform();
        painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
        painter.rotate(-m_colorRings[m_selectedRing].getShift().degrees());
        painter.scale(rect.width()/2, rect.height()/2);
        
        qreal  outerRadius = m_colorRings[m_selectedRing].outerRadius;
        qreal  innerRadius = m_colorRings[m_selectedRing].innerRadius;
        QRectF outerRect(-outerRadius, -outerRadius, outerRadius*2.0, outerRadius*2.0);
        QRectF innerRect(-innerRadius, -innerRadius, innerRadius*2.0, innerRadius*2.0);
        
        painter.setPen(QPen(QBrush(Qt::red), 0.005));
        painter.drawEllipse(outerRect);
        painter.drawEllipse(innerRect);
        
        if(m_selectedPiece >= 0) {
            painter.setPen(QPen(QBrush(Qt::red), 0.01));
            painter.drawPath(m_colorRings[m_selectedRing].pieced[m_selectedPiece]);
        }
    }
    
    painter.resetTransform();
}

void KisColorSelector::drawLightStrip(QPainter& painter, const QRect& rect)
{
    qreal penSize = qreal(qMin(QWidget::width(), QWidget::height())) / 200.0;
    
    painter.resetTransform();
    painter.setPen(QPen(QBrush(Qt::red), penSize));
    
    QTransform matrix;
    matrix.translate(rect.x(), rect.y());
    matrix.scale(rect.width(), rect.height());
    
    KisColor color(m_selectedColor);
    
    for(int i=0; i<m_numLightPieces; ++i) {
        qreal  t1   = qreal(i)   / qreal(m_numLightPieces);
        qreal  t2   = qreal(i+1) / qreal(m_numLightPieces);
        qreal  diff = t2 - t1;// + 0.001;
        QRectF r(t1, 0.0, diff, 1.0);
        
        color.setX(getLight(i, color.getH()));
        
        if(m_lightStripPos == LSP_LEFT || m_lightStripPos == LSP_RIGHT)
            r = QRectF(0.0, t1, 1.0, diff);
        
        r = matrix.mapRect(r);
        painter.fillRect(r, color.getQColor());
        
        if(i == m_selectedLightPiece)
            painter.drawRect(r);
    }
}

void KisColorSelector::paintEvent(QPaintEvent* event)
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
    
//     if(m_clickedRing >= 0) {
//         qreal   value = m_colorRings[m_clickedRing].getShift().value() / m_colorRings[m_clickedRing].getPieceAngle().value();
//         QString num1  = QString::number(value       * 100.0, 'f', 1) + "%";
//         QString num2  = QString::number((1.0-value) * 100.0, 'f', 1) + "%";
//         QFont   font;
//         font.setPixelSize(m_renderArea.width() / 7);
//         
//         wdgPainter.setFont(font);
//         wdgPainter.setPen(QColor(Qt::red));
//         wdgPainter.drawText(m_renderArea, Qt::AlignCenter, num1 + "\n" + num2);
//     }
}

void KisColorSelector::mousePressEvent(QMouseEvent* event)
{
    m_clickPos       = mapCoord(event->posF(), m_renderArea);
    m_mouseMoved     = false;
    m_pressedButtons = event->buttons();
    
    qint8 clickedLightPiece = getLightIndex(event->posF());
    
    if(clickedLightPiece >= 0) {
        qreal light = 1.0 - (qreal(clickedLightPiece) / qreal(getNumLightPieces()-1));
        setLight(light, m_relativeLight);
        
        m_selectedColor.setX(getLight(clickedLightPiece, m_selectedColor.getH()));
        m_selectedLightPiece = clickedLightPiece;
        
        setSelectedColor(m_selectedColor, !(m_pressedButtons & Qt::RightButton));
    }
    else {
        m_clickedRing = getSaturationIndex(m_clickPos);
        
        if(m_clickedRing >= 0) {
            for(int i=0; i<getNumRings(); ++i)
                m_colorRings[i].setTemporaries(m_selectedColor);
        }
    }
}

void KisColorSelector::mouseMoveEvent(QMouseEvent* event)
{
    QPointF dragPos = mapCoord(event->posF(), m_renderArea);
    
    if(m_clickedRing >= 0) {
        float angle     = std::atan2(dragPos.x(), dragPos.y()) - std::atan2(m_clickPos.x(), m_clickPos.y());
        float dist      = std::sqrt(dragPos.x()*dragPos.x() + dragPos.y()*dragPos.y()) * 0.80f;
        float threshold = 5.0f * (1.0f-(dist*dist));
        
        if(qAbs(angle * Radian::TO_DEG) >= threshold || m_mouseMoved) {
            bool selectedRingMoved = true;
            
            if(m_pressedButtons & Qt::RightButton) {
                selectedRingMoved                 = m_clickedRing == m_selectedRing;
                m_colorRings[m_clickedRing].angle = m_colorRings[m_clickedRing].tmpAngle + angle;
            }
            else for(int i=0; i<getNumRings(); ++i)
                m_colorRings[i].angle = m_colorRings[i].tmpAngle + angle;
            
            if(selectedRingMoved) {
                KisColor color = m_colorRings[m_clickedRing].tmpColor;
                Radian   angle = m_colorRings[m_clickedRing].getMovedAngel() + (color.getH()*Radian::PI2);
                color.setH(angle.scaled(0.0f, 1.0f));
                
                m_selectedPiece = getHueIndex(angle, m_colorRings[m_clickedRing].getShift());
                setSelectedColor(color, m_selectedColorIsFgColor, false);
            }
            
            m_mouseMoved = true;
            update();
        }
    }
}

void KisColorSelector::mouseReleaseEvent(QMouseEvent* event)
{
    if(!m_mouseMoved && m_clickedRing >= 0) {
        Radian angle = std::atan2(m_clickPos.x(), m_clickPos.y()) - Radian::RAD_90;
        
        m_selectedRing  = m_clickedRing;
        m_selectedPiece = getHueIndex(angle, m_colorRings[m_clickedRing].getShift());
        
        qreal hue = getHue(m_selectedPiece, m_colorRings[m_clickedRing].getShift());
        qreal sat = getSaturation(m_selectedRing);
        qreal lig = getLight(m_selectedLightPiece, hue);
        
        m_selectedColor.setHSX(hue, sat, lig);
        setSelectedColor(m_selectedColor, !(m_pressedButtons & Qt::RightButton));
    }
    else if(m_mouseMoved)
        setSelectedColor(m_selectedColor, m_selectedColorIsFgColor);
    
    m_clickedRing = -1;
    update();
}

void KisColorSelector::resizeEvent(QResizeEvent* event)
{
    recalculateAreas(quint8(getNumLightPieces()));
}
