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
#include <kis_arcs_constants.h>
#include <resources/KoGamutMask.h>
#include <KisGamutMaskViewConverter.h>

#include "kis_color_selector.h"

//#define DEBUG_ARC_SELECTOR

KisColorSelector::KisColorSelector(QWidget* parent, KisColor::Type type)
    : QWidget(parent)
    , m_colorConverter(KisDisplayColorConverter::dumbConverterInstance())
    , m_colorSpace(type)
    , m_inverseSaturation(false)
    , m_selectedColor(m_colorConverter)
    , m_fgColor(m_colorConverter)
    , m_bgColor(m_colorConverter)
    , m_clickedRing(-1)
    , m_gamutMaskOn(false)
    , m_currentGamutMask(nullptr)
    , m_maskPreviewActive(true)
    , m_widgetUpdatesSelf(false)
{
    m_viewConverter = new KisGamutMaskViewConverter();

    setLumaCoefficients(DEFAULT_LUMA_R, DEFAULT_LUMA_G, DEFAULT_LUMA_B,DEFAULT_LUMA_GAMMA);

    recalculateRings(DEFAULT_SATURATION_STEPS, DEFAULT_HUE_STEPS);
    recalculateAreas(DEFAULT_VALUE_SCALE_STEPS);
    selectColor(KisColor(Qt::red, m_colorConverter, KisColor::HSY, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma));

    using namespace std::placeholders; // For _1 placeholder
    auto function = std::bind(&KisColorSelector::slotUpdateColorAndPreview, this, _1);
    m_updateColorCompressor.reset(new ColorCompressorType(20 /* ms */, function));
}

void KisColorSelector::setColorSpace(KisColor::Type type)
{
    m_colorSpace    = type;
    m_selectedColor = KisColor(m_selectedColor, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);

#ifdef DEBUG_ARC_SELECTOR
        dbgPlugins << "KisColorSelector::setColorSpace: set to:" << m_colorSpace;
#endif

        update();
}

void KisColorSelector::setColorConverter(KisDisplayColorConverter *colorConverter)
{
    m_colorConverter = colorConverter;
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
    m_selectedColor      = KisColor(color, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
    m_selectedPiece      = getHueIndex(m_selectedColor.getH() * PI2);
    m_selectedRing       = getSaturationIndex(m_selectedColor.getS());
    m_selectedLightPiece = getLightIndex(m_selectedColor.getX());
    update();
}

void KisColorSelector::setFgColor(const KoColor& fgColor)
{
    if (!m_widgetUpdatesSelf) {
        m_fgColor = KisColor(fgColor, m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
        m_selectedColor = KisColor(fgColor, m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);

#ifdef DEBUG_ARC_SELECTOR
        dbgPlugins << "KisColorSelector::setFgColor: m_fgColor set to:"
                   << "H:" << m_fgColor.getH()
                   << "S:" << m_fgColor.getS()
                   << "X:" << m_fgColor.getX();

        dbgPlugins << "KisColorSelector::setFgColor: m_selectedColor set to:"
                   << "H:" << m_selectedColor.getH()
                   << "S:" << m_selectedColor.getS()
                   << "X:" << m_selectedColor.getX();
#endif
        update();
    }
}

void KisColorSelector::setBgColor(const KoColor& bgColor)
{
    if (!m_widgetUpdatesSelf) {
        m_bgColor = KisColor(bgColor, m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
#ifdef DEBUG_ARC_SELECTOR
        dbgPlugins << "KisColorSelector::setBgColor: m_bgColor set to:"
                   << "H:" << m_bgColor.getH()
                   << "S:" << m_bgColor.getS()
                   << "X:" << m_bgColor.getX();
#endif
        update();
    }
}

void KisColorSelector::setLight(qreal light)
{
    m_selectedColor.setX(qBound(0.0, light, 1.0));
    m_selectedLightPiece = getLightIndex(m_selectedColor.getX());
    update();
}

void KisColorSelector::setLumaCoefficients(qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;

    if (m_colorSpace == KisColor::HSY) {
        m_lumaGamma = lGamma;
    } else {
        m_lumaGamma = 1.0;
    }

    m_selectedColor = KisColor(m_selectedColor, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);

#ifdef DEBUG_ARC_SELECTOR
        dbgPlugins << "KisColorSelector::setLumaCoefficients: " << m_lumaR
                   << " " << m_lumaG << " " << m_lumaB << " " << m_lumaGamma;
#endif

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

void KisColorSelector::setGamutMask(KoGamutMask* gamutMask)
{
    if (!gamutMask) {
        return;
    }

    m_currentGamutMask = gamutMask;
    m_viewConverter->setViewSize(m_renderAreaSize);
    m_viewConverter->setMaskSize(m_currentGamutMask->maskSize());
    update();
}

KoGamutMask* KisColorSelector::gamutMask()
{
    return m_currentGamutMask;
}

bool KisColorSelector::gamutMaskOn()
{
    return m_gamutMaskOn;
}


void KisColorSelector::setGamutMaskOn(bool gamutMaskOn)
{
    if (m_currentGamutMask) {
        m_gamutMaskOn = gamutMaskOn;
        update();
    }
}

void KisColorSelector::setEnforceGamutMask(bool enforce)
{
    m_enforceGamutMask = enforce;
    update();
}

QPointF KisColorSelector::mapCoordToView(const QPointF& pt, const QRectF& viewRect) const
{
    qreal w = viewRect.width()  / 2.0;
    qreal h = viewRect.height() / 2.0;

    qreal x = pt.x() + 1.0;
    qreal y = (pt.y()) + 1.0;

    return QPointF(x*w, y*h);
}

QPointF KisColorSelector::mapCoordToUnit(const QPointF& pt, const QRectF& viewRect) const
{
    qreal w = viewRect.width()  / 2.0;
    qreal h = viewRect.height() / 2.0;
    qreal x = pt.x() - (viewRect.x() + w);
    qreal y = pt.y() - (viewRect.y() + h);
    return QPointF(x/w, y/h);
}

QPointF KisColorSelector::mapColorToUnit(const KisColor& color, bool invertSaturation) const
{
    qreal radius;
    if (invertSaturation && m_inverseSaturation) {
        radius = 1.0 - color.getS();
    } else {
        radius = color.getS();
    }

    QPointF hueCoord = mapHueToAngle(color.getH());
    qreal x = hueCoord.x()*radius;
    qreal y = hueCoord.y()*radius;

    return QPointF(x,y);
}

KisColorSelector::Radian KisColorSelector::mapCoordToAngle(qreal x, qreal y) const
{
    qreal angle = std::atan2(-y, -x);

#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::mapCoordToAngle: "
               << "X:" << x
               << "Y:" << y
               << "angle:" << angle;
#endif

    return angle;
}

QPointF KisColorSelector::mapHueToAngle(qreal hue) const
{
    qreal angle = hue * 2.0 * M_PI - M_PI;
    qreal x = std::cos(angle);
    qreal y = std::sin(angle);

    return QPointF(x,y);
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

qint8 KisColorSelector::getHueIndex(Radian hue) const
{
    qreal partSize = 1.0 / qreal(getNumPieces());
    return qint8(qRound(hue.scaled(0.0, 1.0) / partSize) % getNumPieces());
}

qreal KisColorSelector::getHue(int hueIdx, Radian shift) const
{
    Radian hue = (qreal(hueIdx) / qreal(getNumPieces())) * PI2;
    hue += shift;
    return hue.scaled(0.0, 1.0);
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

    qreal LIGHT_STRIP_RATIO = 0.075;
    if (m_showValueScaleNumbers) {
        LIGHT_STRIP_RATIO = 0.25;
    }

    int width      = QWidget::width();
    int height     = QWidget::height();
    int size       = qMin(width, height);
    int stripThick = int(size * LIGHT_STRIP_RATIO);

    width -= stripThick;

    size = qMin(width, height);

    int x = (width  - size) / 2;
    int y = (height - size) / 2;

    m_renderAreaSize = QSize(size,size);
    m_viewConverter->setViewSize(m_renderAreaSize);

    m_renderArea     = QRect(x+stripThick, y, size, size);
    m_lightStripArea = QRect(0, 0, stripThick, QWidget::height());

    m_renderBuffer   = QImage(size, size, QImage::Format_ARGB32_Premultiplied);
    m_maskBuffer   = QImage(size, size, QImage::Format_ARGB32_Premultiplied);
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

bool KisColorSelector::colorIsClear(const KisColor &color)
{
    if (m_gamutMaskOn && m_currentGamutMask) {

        QPointF colorCoord = mapCoordToView(mapColorToUnit(color, false), m_renderArea);
        bool isClear = m_currentGamutMask->coordIsClear(colorCoord, *m_viewConverter, m_maskPreviewActive);

        if (isClear) {
            return true;
        } else {
            return false;
        }

    } else {
        return true;
    }

    return false;
}


void KisColorSelector::requestUpdateColorAndPreview(const KisColor &color, Acs::ColorRole role)
{
#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::requestUpdateColorAndPreview: requesting update to: "
               << "H:" << color.getH()
               << "S:" << color.getS()
               << "X:" << color.getX();
#endif
    m_updateColorCompressor->start(qMakePair(color, role));
}

void KisColorSelector::slotUpdateColorAndPreview(QPair<KisColor, Acs::ColorRole> color)
{
    const bool selectAsFgColor = color.second == Acs::Foreground;

    if (selectAsFgColor) { m_fgColor = color.first; }
    else                 { m_bgColor = color.first; }

    m_selectedColor     = color.first;

#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::slotUpdateColorAndPreview: m_selectedColor set to:"
               << "H:" << m_selectedColor.getH()
               << "S:" << m_selectedColor.getS()
               << "X:" << m_selectedColor.getX();
#endif

    if (selectAsFgColor) { emit sigFgColorChanged(m_selectedColor); }
    else                 { emit sigBgColorChanged(m_selectedColor); }
}

void KisColorSelector::drawRing(QPainter& painter, KisColorSelector::ColorRing& ring, const QRect& rect)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.resetTransform();
    painter.translate(rect.width()/2, rect.height()/2);

    if (ring.pieced.size() > 1) {
        QTransform mirror;
        mirror.rotate(180, Qt::YAxis);
        painter.setTransform(mirror, true);
        painter.scale(rect.width()/2, rect.height()/2);
        QPen normalPen = QPen(QBrush(COLOR_NORMAL_OUTLINE), 0.005);
        QPen clearMaskPen = QPen(QBrush(COLOR_MASK_CLEAR), 0.005);
        QBrush brush(Qt::SolidPattern);

        for(int i=0; i<ring.pieced.size(); ++i) {
            qreal hue = qreal(i) / qreal(ring.pieced.size());
            hue = (hue >= 1.0) ? (hue - 1.0) : hue;
            hue = (hue <  0.0) ? (hue + 1.0) : hue;

            KisColor color(hue, m_colorConverter, m_colorSpace);
            color.setS(ring.saturation);
            color.setX(m_selectedColor.getX());

            if(m_gamutMaskOn && m_enforceGamutMask && colorIsClear(color)) {
                painter.setPen(clearMaskPen);
            } else {
                painter.setPen(normalPen);
            }

            if ((m_enforceGamutMask) && (!colorIsClear(color))) {
                brush.setColor(COLOR_MASK_FILL);
            } else {
                brush.setColor(color.toQColor());
            }
            painter.setBrush(brush);

            painter.drawPath(ring.pieced[i]);
        }
    }
    else {
        KisColor colors[7] = {
            KisColor(Qt::cyan   , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::green  , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::yellow , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::red    , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::magenta, m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::blue   , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma),
            KisColor(Qt::cyan   , m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma)
        };

        QConicalGradient gradient(0, 0, 0);

        for(int i=0; i<=6; ++i) {
            qreal hue = qreal(i) / 6.0;
            colors[i].setS(ring.saturation);
            colors[i].setX(m_selectedColor.getX());
            gradient.setColorAt(hue, colors[i].toQColor());
        }

        painter.scale(rect.width()/2, rect.height()/2);
        painter.fillPath(ring.pieced[0], QBrush(gradient));
    }

    painter.restore();
}

void KisColorSelector::drawOutline(QPainter& painter, const QRect& rect)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.resetTransform();
    painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    painter.scale(rect.width()/2, rect.height()/2);

    QPen normalPen = QPen(QBrush(COLOR_NORMAL_OUTLINE), 0.005);
    QPen selectedPen;

    painter.setPen(normalPen);

    if (getNumPieces() > 1) {
        if (m_selectedRing >= 0 && m_selectedPiece >= 0) {
            painter.resetTransform();
            painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
            QTransform mirror;
            mirror.rotate(180, Qt::YAxis);
            painter.setTransform(mirror, true);
            painter.scale(rect.width()/2, rect.height()/2);

            if (m_selectedColor.getX() < 0.55) {
                selectedPen = QPen(QBrush(COLOR_SELECTED_LIGHT), 0.007);
            } else {
                selectedPen = QPen(QBrush(COLOR_SELECTED_DARK), 0.007);
            }

            painter.setPen(selectedPen);
            painter.drawPath(m_colorRings[m_selectedRing].pieced[m_selectedPiece]);
        }
    }
    else {
        for(int i=0; i<getNumRings(); ++i) {
            qreal rad = m_colorRings[i].outerRadius;
            painter.drawEllipse(QRectF(-rad, -rad, rad*2.0, rad*2.0));
        }

        if (m_selectedRing >= 0) {
            qreal iRad = m_colorRings[m_selectedRing].innerRadius;
            qreal oRad = m_colorRings[m_selectedRing].outerRadius;

            if (m_selectedColor.getX() < 0.55) {
                selectedPen = QPen(QBrush(COLOR_SELECTED_LIGHT), 0.005);
            } else {
                selectedPen = QPen(QBrush(COLOR_SELECTED_DARK), 0.005);
            }

            painter.setPen(selectedPen);
            painter.drawEllipse(QRectF(-iRad, -iRad, iRad*2.0, iRad*2.0));
            painter.drawEllipse(QRectF(-oRad, -oRad, oRad*2.0, oRad*2.0));

            QPointF lineCoords = mapHueToAngle(m_selectedColor.getH());
            painter.drawLine(QPointF(lineCoords.x()*iRad, lineCoords.y()*iRad), QPointF(lineCoords.x()*oRad, lineCoords.y()*oRad));
        }
    }
}

void KisColorSelector::drawLightStrip(QPainter& painter, const QRect& rect)
{
    qreal    penSize    = qreal(qMin(QWidget::width(), QWidget::height())) / 200.0;
    qreal    penSizeSmall = penSize / 1.2;
    QPen selectedPen;

    KisColor valueScaleColor(m_selectedColor, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
    KisColor grayScaleColor(Qt::gray, m_colorConverter, m_colorSpace, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
    int rectSize = rect.height();

    painter.resetTransform();
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QTransform matrix;
    matrix.translate(rect.x(), rect.y());
    matrix.scale(rect.width(), rect.height());

    qreal rectColorLeftX;
    qreal rectColorWidth;

    if (m_showValueScaleNumbers) {
        rectColorLeftX = 0.6;
        rectColorWidth = 0.4;
    } else {
        rectColorLeftX = 0.0;
        rectColorWidth = 1.0;
    }

    if (getNumLightPieces() > 1) {
        for(int i=0; i<getNumLightPieces(); ++i) {
            qreal  t1    = qreal(i)   / qreal(getNumLightPieces());
            qreal  t2    = qreal(i+1) / qreal(getNumLightPieces());
            qreal  light = 1.0 - (qreal(i) / qreal(getNumLightPieces()-1));
            qreal  diff  = t2 - t1;// + 0.001;

            QRectF rectColor = QRectF(rectColorLeftX, t1, rectColorWidth, diff);
            rectColor = matrix.mapRect(rectColor);

            valueScaleColor.setX(light);
            painter.fillRect(rectColor, valueScaleColor.toQColor());

            if (i == m_selectedLightPiece) {
                if (light < 0.55) {
                    selectedPen = QPen(QBrush(COLOR_SELECTED_LIGHT), penSize);
                } else {
                    selectedPen = QPen(QBrush(COLOR_SELECTED_DARK), penSize);
                }

                painter.setPen(selectedPen);
                painter.drawRect(rectColor);
            }
        }
    } else {
        painter.setRenderHint(QPainter::Antialiasing, false);

        for(int i=0; i<rectSize; ++i) {
            int   y     = rect.y() + i;
            qreal light = 1.0 - (qreal(i) / qreal(rectSize-1));
            valueScaleColor.setX(light);
            painter.setPen(QPen(QBrush(valueScaleColor.toQColor()), penSize));
            painter.drawLine(rect.left(), y, rect.right(), y);
        }
    }

    // draw color blip
    painter.setRenderHint(QPainter::Antialiasing, false);
    // draw position of fg color value on the strip
    qreal fgColorValue = 1.0 - m_fgColor.getX();

    int y = rect.y() + int(rectSize * fgColorValue);
    painter.setPen(QPen(QBrush(COLOR_SELECTED_LIGHT), penSizeSmall));
    painter.drawLine(rect.left(), y, rect.right(), y);
    painter.setPen(QPen(QBrush(COLOR_SELECTED_DARK), penSizeSmall));
    painter.drawLine(rect.left(), y+2*penSizeSmall, rect.right(), y+2*penSizeSmall);
    // draw color blip

    if (m_showValueScaleNumbers) {
        painter.setRenderHint(QPainter::Antialiasing, true);

        int valueScalePieces = getNumLightPieces();
        if (getNumLightPieces() == 1) {
            valueScalePieces = 11;
        }

        QFont font = painter.font();
        QFontMetrics fm = painter.fontMetrics();
        while (fm.boundingRect("100%").width() > rect.width()*rectColorLeftX) {
            font.setPointSize(font.pointSize() - 1);
            painter.setFont(font);
            fm = painter.fontMetrics();
        }

        for(int i=0; i<valueScalePieces; ++i) {
            qreal  t1    = qreal(i)   / qreal(valueScalePieces);
            qreal  t2    = qreal(i+1) / qreal(valueScalePieces);
            qreal  light = 1.0 - (qreal(i) / qreal(valueScalePieces-1));
            qreal  diff  = t2 - t1;// + 0.001;

            grayScaleColor.setX(light);

            QRectF rectValue = QRectF(0.0, t1, rectColorLeftX, diff);
            rectValue = matrix.mapRect(rectValue);

            painter.fillRect(rectValue, grayScaleColor.toQColor());

            int valueNumber = 0;
            if (m_colorSpace == KisColor::HSY) {
                valueNumber = 100 - round(pow(pow(grayScaleColor.getX(), m_lumaGamma), 1.0/2.2)*100);
            } else {
                valueNumber = 100 - grayScaleColor.getX()*100;
            }

            if (valueNumber < 55) {
                painter.setPen(QPen(QBrush(COLOR_DARK), penSize));
            } else {
                painter.setPen(QPen(QBrush(COLOR_LIGHT), penSize));
            }

            painter.drawText(rectValue, Qt::AlignRight|Qt::AlignBottom, QString("%1%").arg(QString::number(valueNumber)));
        }
    }

    painter.restore();
}

void KisColorSelector::drawBlip(QPainter& painter, const QRect& rect)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.resetTransform();
    painter.translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    painter.scale(rect.width()/2, rect.height()/2);

    QPointF fgColorPos = mapColorToUnit(m_fgColor);

#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::drawBlip: "
               << "colorPoint H:" << m_fgColor.getH() << " S:" << m_fgColor.getS()
               << "-> coord X:" << fgColorPos.x() << " Y:" << fgColorPos.y();
#endif

    painter.setPen(QPen(QBrush(COLOR_SELECTED_DARK), 0.01));
    painter.drawEllipse(fgColorPos, 0.05, 0.05);

    painter.setPen(QPen(QBrush(COLOR_SELECTED_LIGHT), 0.01));
    painter.setBrush(m_fgColor.toQColor());
    painter.drawEllipse(fgColorPos, 0.04, 0.04);
}

void KisColorSelector::drawGamutMaskShape(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.resetTransform();
    painter.translate(rect.width()/2, rect.height()/2);
    painter.scale(rect.width()/2, rect.height()/2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(COLOR_MASK_FILL);

    painter.drawEllipse(QPointF(0,0), 1.0, 1.0);

    painter.resetTransform();

    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    m_currentGamutMask->paint(painter, *m_viewConverter, m_maskPreviewActive);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    m_currentGamutMask->paintStroke(painter, *m_viewConverter, m_maskPreviewActive);

    painter.restore();
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

    // draw the fg and bg color previews
    // QPainter settings must be saved and restored afterwards, otherwise the wheel drawing is totally broken
    wdgPainter.save();
    wdgPainter.setRenderHint(QPainter::Antialiasing, true);

    QRect fgRect(0, 0, QWidget::width(), QWidget::height());
    wdgPainter.fillRect(fgRect, m_fgColor.toQColor());

    int bgSide = qMin(QWidget::width()*0.15,QWidget::height()*0.15);

    if (m_showBgColor) {
        QPointF bgPolyPoints[3] = {
            QPointF(QWidget::width(), QWidget::height()),
            QPointF(QWidget::width()-bgSide, QWidget::height()),
            QPointF(QWidget::width(), QWidget::height()-bgSide)
        };

        wdgPainter.setBrush(m_bgColor.toQColor());
        wdgPainter.setPen(m_bgColor.toQColor());
        wdgPainter.drawPolygon(bgPolyPoints, 3);
    }

    wdgPainter.restore();

    for(int i=0; i<m_colorRings.size(); ++i)
        drawRing(imgPainter, m_colorRings[i], m_renderArea);

    wdgPainter.drawImage(m_renderArea, m_renderBuffer);

    // draw the mask either in continuous mode or in discrete mode when enforcing is turned off
    // if enforcing is turned on in discrete mode,
    // drawRing function takes care of delineating the mask swatches
    if (m_gamutMaskOn
            && ((getNumPieces() == 1) || (!m_enforceGamutMask))) {

        m_maskBuffer.fill(0);
        QPainter maskPainter(&m_maskBuffer);
        drawGamutMaskShape(maskPainter, m_renderArea);

        wdgPainter.drawImage(m_renderArea, m_maskBuffer);
    }

    drawOutline   (wdgPainter, m_renderArea);
    drawLightStrip(wdgPainter, m_lightStripArea);
    drawBlip (wdgPainter, m_renderArea);
}

void KisColorSelector::mousePressEvent(QMouseEvent* event)
{
    m_widgetUpdatesSelf = true;
#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::mousePressEvent: m_widgetUpdatesSelf = true";
#endif

    m_clickPos       = mapCoordToUnit(event->localPos(), m_renderArea);
    m_mouseMoved     = false;
    m_pressedButtons = event->buttons();
    m_clickedRing    = getSaturationIndex(m_clickPos);
    Acs::ColorRole colorRole = Acs::buttonsToRole(Qt::NoButton, m_pressedButtons);

    qint8 clickedLightPiece = getLightIndex(event->localPos());

    if (clickedLightPiece >= 0) {
        setLight(getLight(event->localPos()));
        m_selectedLightPiece = clickedLightPiece;
        requestUpdateColorAndPreview(m_selectedColor, colorRole);
        m_mouseMoved   = true;
    }
    else if (m_clickedRing >= 0) {
        if (getNumPieces() == 1) {
            Radian angle = mapCoordToAngle(m_clickPos.x(), m_clickPos.y());

            KisColor color(m_colorConverter, m_colorSpace);
            color.setHSX(angle.scaled(0.0, 1.0)
                         , getSaturation(m_clickedRing)
                         , m_selectedColor.getX()
                         );

#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::mousePressEvent: picked color: "
               << "H:" << color.getH()
               << "S:" << color.getS()
               << "X:" << color.getX();
#endif

            if ((!m_enforceGamutMask) || colorIsClear(color)) {
                m_selectedColor.setHSX(color.getH(), color.getS(), color.getX());
                requestUpdateColorAndPreview(m_selectedColor, colorRole);
                m_selectedRing = m_clickedRing;
                m_mouseMoved   = true;
                update();
            }
        }
    }
}

void KisColorSelector::mouseMoveEvent(QMouseEvent* event)
{
    QPointF dragPos = mapCoordToUnit(event->localPos(), m_renderArea);
    qint8   clickedLightPiece = getLightIndex(event->localPos());
    Acs::ColorRole colorRole = Acs::buttonsToRole(Qt::NoButton, m_pressedButtons);

    if (clickedLightPiece >= 0) {
        setLight(getLight(event->localPos()));
        m_selectedLightPiece = clickedLightPiece;
        requestUpdateColorAndPreview(m_selectedColor, colorRole);
    }

    if (m_clickedRing < 0)
        return;

    if (getNumPieces() == 1) {
        Radian angle = mapCoordToAngle(dragPos.x(), dragPos.y());
        KisColor color(m_colorConverter, m_colorSpace);
        color.setHSX(angle.scaled(0.0, 1.0)
                     , getSaturation(m_clickedRing)
                     , m_selectedColor.getX()
                     );

        if ((!m_enforceGamutMask) || colorIsClear(color)) {
            m_selectedColor.setHSX(color.getH(), color.getS(), color.getX());
            requestUpdateColorAndPreview(m_selectedColor, colorRole);
        }
    }

    update();
}

void KisColorSelector::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    Acs::ColorRole colorRole = Acs::buttonsToRole(Qt::NoButton, m_pressedButtons);

    if (!m_mouseMoved && m_clickedRing >= 0) {
        Radian angle = mapCoordToAngle(m_clickPos.x(), m_clickPos.y());
        KisColor color(m_colorConverter, m_colorSpace);

        qint8 hueIndex = getHueIndex(angle);

        if (getNumPieces() > 1) {
            color.setH(getHue(hueIndex));
        } else {
            color.setH(angle.scaled(0.0, 1.0));
        }

        color.setS(getSaturation(m_clickedRing));
        color.setX(m_selectedColor.getX());

        if ((!m_enforceGamutMask) || colorIsClear(color)) {
            m_selectedColor.setHSX(color.getH(), color.getS(), color.getX());
            m_selectedPiece = hueIndex;
            m_selectedRing = m_clickedRing;
            requestUpdateColorAndPreview(m_selectedColor, colorRole);
        }
    }
    else if (m_mouseMoved)
        requestUpdateColorAndPreview(m_selectedColor, colorRole);

    m_clickedRing = -1;

    m_widgetUpdatesSelf = false;
#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::ReleasePressEvent: m_widgetUpdatesSelf = false";
#endif

    update();
}

void KisColorSelector::resizeEvent(QResizeEvent* /*event*/)
{
    recalculateAreas(quint8(getNumLightPieces()));
}

void KisColorSelector::leaveEvent(QEvent* /*e*/)
{
    m_widgetUpdatesSelf = false;
#ifdef DEBUG_ARC_SELECTOR
    dbgPlugins << "KisColorSelector::leaveEvent: m_widgetUpdatesSelf = false";
#endif
}

void KisColorSelector::saveSettings()
{
    KisConfig cfg(false);
    cfg.writeEntry("ArtColorSel.ColorSpace" , qint32(m_colorSpace));

    cfg.writeEntry("ArtColorSel.lumaR", qreal(m_lumaR));
    cfg.writeEntry("ArtColorSel.lumaG", qreal(m_lumaG));
    cfg.writeEntry("ArtColorSel.lumaB", qreal(m_lumaB));
    cfg.writeEntry("ArtColorSel.lumaGamma", qreal(m_lumaGamma));

    cfg.writeEntry("ArtColorSel.NumRings"   , m_colorRings.size());
    cfg.writeEntry("ArtColorSel.RingPieces" , qint32(m_numPieces));
    cfg.writeEntry("ArtColorSel.LightPieces", qint32(m_numLightPieces));

    cfg.writeEntry("ArtColorSel.InversedSaturation", m_inverseSaturation);
    cfg.writeEntry("ArtColorSel.Light"             , m_selectedColor.getX());

    cfg.writeEntry("ArtColorSel.SelColorH", m_selectedColor.getH());
    cfg.writeEntry("ArtColorSel.SelColorS", m_selectedColor.getS());
    cfg.writeEntry("ArtColorSel.SelColorX", m_selectedColor.getX());

    cfg.writeEntry("ArtColorSel.defaultHueSteps", quint32(m_defaultHueSteps));
    cfg.writeEntry("ArtColorSel.defaultSaturationSteps", quint32(m_defaultSaturationSteps));
    cfg.writeEntry("ArtColorSel.defaultValueScaleSteps", quint32(m_defaultValueScaleSteps));

    cfg.writeEntry("ArtColorSel.showBgColor", m_showBgColor);
    cfg.writeEntry("ArtColorSel.showValueScale", m_showValueScaleNumbers);
    cfg.writeEntry("ArtColorSel.enforceGamutMask", m_enforceGamutMask);
}

void KisColorSelector::loadSettings()
{
    KisConfig cfg(true);

    m_defaultHueSteps = cfg.readEntry("ArtColorSel.defaultHueSteps", DEFAULT_HUE_STEPS);
    m_defaultSaturationSteps = cfg.readEntry("ArtColorSel.defaultSaturationSteps", DEFAULT_SATURATION_STEPS);
    m_defaultValueScaleSteps = cfg.readEntry("ArtColorSel.defaultValueScaleSteps", DEFAULT_VALUE_SCALE_STEPS);

    setNumLightPieces(cfg.readEntry("ArtColorSel.LightPieces", DEFAULT_VALUE_SCALE_STEPS));

    KisColor::Type colorSpace = KisColor::Type(cfg.readEntry<qint32>("ArtColorSel.ColorSpace" , KisColor::HSY));

    setColorSpace(colorSpace);

    setLumaCoefficients(cfg.readEntry("ArtColorSel.lumaR", DEFAULT_LUMA_R),
                        cfg.readEntry("ArtColorSel.lumaG", DEFAULT_LUMA_G),
                        cfg.readEntry("ArtColorSel.lumaB", DEFAULT_LUMA_B),
                        cfg.readEntry("ArtColorSel.lumaGamma", DEFAULT_LUMA_GAMMA));

    m_selectedColor.setH(cfg.readEntry<qreal>("ArtColorSel.SelColorH", 0.0));
    m_selectedColor.setS(cfg.readEntry<qreal>("ArtColorSel.SelColorS", 0.0));
    m_selectedColor.setX(cfg.readEntry<qreal>("ArtColorSel.SelColorX", 0.0));

    setInverseSaturation(cfg.readEntry<bool>("ArtColorSel.InversedSaturation", false));
    setLight(cfg.readEntry<qreal>("ArtColorSel.Light", 0.5f));

    setNumRings(cfg.readEntry("ArtColorSel.NumRings", DEFAULT_SATURATION_STEPS));
    setNumPieces(cfg.readEntry("ArtColorSel.RingPieces", DEFAULT_HUE_STEPS));

    m_showBgColor = cfg.readEntry("ArtColorSel.showBgColor", true);
    m_showValueScaleNumbers = cfg.readEntry("ArtColorSel.showValueScale", false);
    m_enforceGamutMask = cfg.readEntry("ArtColorSel.enforceGamutMask", false);

    selectColor(m_selectedColor);
    update();
}

void KisColorSelector::setDefaultHueSteps(int num)
{
    num = qBound(MIN_NUM_HUE_PIECES, num, MAX_NUM_HUE_PIECES);
    m_defaultHueSteps = num;
}

void KisColorSelector::setDefaultSaturationSteps(int num)
{
    num = qBound(MIN_NUM_SATURATION_RINGS, num, MAX_NUM_SATURATION_RINGS);
    m_defaultSaturationSteps = num;
}

void KisColorSelector::setDefaultValueScaleSteps(int num)
{
    num = qBound(MIN_NUM_LIGHT_PIECES, num, MAX_NUM_LIGHT_PIECES);
    m_defaultValueScaleSteps = num;
}

void KisColorSelector::setShowBgColor(bool value)
{
    m_showBgColor = value;
    update();
}

void KisColorSelector::setShowValueScaleNumbers(bool value)
{
    m_showValueScaleNumbers = value;
    recalculateAreas(quint8(getNumLightPieces()));
    update();
}

bool KisColorSelector::saturationIsInvertible()
{
    if (!m_gamutMaskOn) return true;
    bool b = (m_enforceGamutMask && getNumPieces() == 1) ? false : true;
    return b;
}

