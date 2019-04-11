/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#include "kis_grid_paintop.h"
#include "kis_grid_paintop_settings.h"

#include <cmath>

#include <QtGlobal>
#include <QRect>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paint_information.h>
#include <kis_cross_device_color_picker.h>
#include <kis_spacing_information.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoMixColorsOp.h>

#include <kis_gridop_option.h>
#include <kis_grid_shape_option.h>
#include <kis_color_option.h>
#include <kis_lod_transform.h>


#ifdef BENCHMARK
#include <QTime>
#endif


KisGridPaintOp::KisGridPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
    , m_settings(static_cast<KisGridPaintOpSettings*>(const_cast<KisPaintOpSettings*>(settings.data())))
    , m_node(node)
{
    m_properties.readOptionSetting(settings);
    m_colorProperties.fillProperties(settings);

    m_xSpacing = m_properties.gridWidth * m_properties.scale;
    m_ySpacing = m_properties.gridHeight * m_properties.scale;
    m_spacing = m_xSpacing;

    m_dab = source()->createCompositionSourceDevice();
    m_painter = new KisPainter(m_dab);
    m_painter->setPaintColor(painter->paintColor());
    m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
#ifdef BENCHMARK
    m_count = m_total = 0;
#endif

}

KisGridPaintOp::~KisGridPaintOp()
{
    delete m_painter;
}

KisSpacingInformation KisGridPaintOp::paintAt(const KisPaintInformation& info)
{
#ifdef BENCHMARK
    QTime time;
    time.start();
#endif

    KisRandomSourceSP randomSource = info.randomSource();
    const qreal additionalScale = KisLodTransform::lodToScale(painter()->device());

    m_dab->clear();

    qreal gridWidth = m_properties.gridWidth * m_properties.scale * additionalScale;
    qreal gridHeight = m_properties.gridHeight * m_properties.scale * additionalScale;

    int divide;
    if (m_properties.pressureDivision) {
        divide = m_properties.divisionLevel * info.pressure();
    }
    else {
        divide = m_properties.divisionLevel;
    }
    divide = qRound(m_properties.scale * divide);

    qreal posX = info.pos().x();
    qreal posY = info.pos().y();
    posX = posX - std::fmod(posX, gridWidth);
    posY = posY - std::fmod(posY, gridHeight);

    const QRectF dabRect(posX, posY, gridWidth, gridHeight);
    const QRect dabRectAligned = dabRect.toAlignedRect();

    divide = qMax(1, divide);
    const qreal yStep = gridHeight / (qreal)divide;
    const qreal xStep = gridWidth / (qreal)divide;

    QRectF tile;
    KoColor color(painter()->paintColor());

    QScopedPointer<KisCrossDeviceColorPicker> colorPicker;
    if (m_node) {
        colorPicker.reset(new KisCrossDeviceColorPicker(m_node->paintDevice(), color));
    }

    qreal vertBorder = m_properties.vertBorder * additionalScale;
    qreal horzBorder = m_properties.horizBorder * additionalScale;
    if (m_properties.randomBorder) {
        if (vertBorder == horzBorder) {
            vertBorder = horzBorder = vertBorder * randomSource->generateNormalized();
        }
        else {
            vertBorder *= randomSource->generateNormalized();
            horzBorder *= randomSource->generateNormalized();
        }
    }

    bool shouldColor = true;
    // fill the tile
    if (m_colorProperties.fillBackground) {
        m_dab->fill(dabRectAligned, painter()->backgroundColor());
    }

    for (int y = 0; y < divide; y++) {
        for (int x = 0; x < divide; x++) {
            // determine the tile size
            tile = QRectF(dabRect.x() + x * xStep, dabRect.y() + y * yStep, xStep, yStep);
            tile.adjust(vertBorder, horzBorder, -vertBorder, -horzBorder);
            tile = tile.normalized();

            // do color transformation
            if (shouldColor) {
                if (colorPicker && m_colorProperties.sampleInputColor) {
                    colorPicker->pickOldColor(tile.center().x(), tile.center().y(), color.data());
                }

                // mix the color with background color
                if (m_colorProperties.mixBgColor) {
                    KoMixColorsOp * mixOp = source()->colorSpace()->mixColorsOp();

                    const quint8 *colors[2];
                    colors[0] = color.data();
                    colors[1] = painter()->backgroundColor().data();

                    qint16 colorWeights[2];
                    int MAX_16BIT = 255;
                    qreal blend = info.pressure();

                    colorWeights[0] = static_cast<quint16>(blend * MAX_16BIT);
                    colorWeights[1] = static_cast<quint16>((1.0 - blend) * MAX_16BIT);
                    mixOp->mixColors(colors, colorWeights, 2, color.data());
                }

                if (m_colorProperties.useRandomHSV) {
                    QHash<QString, QVariant> params;
                    params["h"] = (m_colorProperties.hue / 180.0) * randomSource->generateNormalized();
                    params["s"] = (m_colorProperties.saturation / 100.0) * randomSource->generateNormalized();
                    params["v"] = (m_colorProperties.value / 100.0) * randomSource->generateNormalized();
                    KoColorTransformation* transfo;
                    transfo = m_dab->colorSpace()->createColorTransformation("hsv_adjustment", params);
                    transfo->setParameter(3, 1);//sets the type to HSV. For some reason 0 is not an option.
                    transfo->setParameter(4, false);//sets the colorize to false.
                    transfo->transform(color.data(), color.data() , 1);
                }

                if (m_colorProperties.useRandomOpacity) {
                    const qreal alpha = randomSource->generateNormalized();
                    color.setOpacity(alpha);
                    m_painter->setOpacity(qRound(alpha * OPACITY_OPAQUE_U8));
                }

                if (!m_colorProperties.colorPerParticle) {
                    shouldColor = false;
                }
                m_painter->setPaintColor(color);
            }

            // paint some element
            switch (m_properties.shape) {
            case 0: {
                m_painter->paintEllipse(tile);
                break;
            }
            case 1: {
                // anti-aliased version
                //m_painter->paintRect(tile);
                m_dab->fill(tile.topLeft().x(), tile.topLeft().y(), tile.width(), tile.height(), color.data());
                break;
            }
            case 2: {
                m_painter->drawDDALine(tile.topRight(), tile.bottomLeft());
                break;
            }
            case 3: {
                m_painter->drawLine(tile.topRight(), tile.bottomLeft());
                break;
            }
            case 4: {
                m_painter->drawThickLine(tile.topRight(), tile.bottomLeft() , 1, 10);
                break;
            }
            default: {
                break;
            }
            }

            if (m_colorProperties.colorPerParticle){
                color=painter()->paintColor();//reset color//
            }
        }
    }

    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);

#ifdef BENCHMARK
    int msec = time.elapsed();
    dbgKrita << msec << " ms/dab " << "[average: " << m_total / (qreal)m_count << "]";
    m_total += msec;
    m_count++;
#endif
    return computeSpacing(additionalScale);
}

KisSpacingInformation KisGridPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return computeSpacing(KisLodTransform::lodToScale(painter()->device()));
}

KisSpacingInformation KisGridPaintOp::computeSpacing(qreal lodScale) const
{
    return KisSpacingInformation(m_spacing * lodScale);
}

void KisGridProperties::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    gridWidth = qMax(1, setting->getInt(GRID_WIDTH));
    gridHeight = qMax(1, setting->getInt(GRID_HEIGHT));
    divisionLevel = qMax(1, setting->getInt(GRID_DIVISION_LEVEL));
    pressureDivision =  setting->getBool(GRID_PRESSURE_DIVISION);
    randomBorder = setting->getBool(GRID_RANDOM_BORDER);
    scale = qMax(0.1, setting->getDouble(GRID_SCALE));
    vertBorder  = setting->getDouble(GRID_VERTICAL_BORDER);
    horizBorder = setting->getDouble(GRID_HORIZONTAL_BORDER);

    shape = setting->getInt(GRIDSHAPE_SHAPE);
}
