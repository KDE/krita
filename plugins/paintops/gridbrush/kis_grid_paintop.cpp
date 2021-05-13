/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_cross_device_color_sampler.h>
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


KisGridPaintOp::KisGridPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP /*image*/)
    : KisPaintOp(painter)
    , m_settings(static_cast<KisGridPaintOpSettings*>(const_cast<KisPaintOpSettings*>(settings.data())))
    , m_node(node)
{

    m_properties.readOptionSetting(settings);
    m_colorProperties.fillProperties(settings);

    m_xSpacing = m_properties.grid_width * m_properties.grid_scale;
    m_ySpacing = m_properties.grid_height * m_properties.grid_scale;
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

    qreal gridWidth = m_properties.diameter * m_properties.grid_scale * additionalScale;
    qreal gridHeight = m_properties.diameter * m_properties.grid_scale * additionalScale;

    qreal cellWidth = m_properties.grid_width * m_properties.grid_scale * additionalScale;
    qreal cellHeight = m_properties.grid_height * m_properties.grid_scale * additionalScale;

    qreal horizontalOffset = m_properties.horizontal_offset;
    qreal verticalOffset = m_properties.vertical_offset;

    int divide;
    if (m_properties.grid_pressure_division) {
        divide = m_properties.grid_division_level * info.pressure();
    }
    else {
        divide = m_properties.grid_division_level;
    }

    divide = qRound(m_properties.grid_scale * divide);

    //Adjust the start position of the drawn grid to the top left of the brush instead of in the center
    qreal posX = info.pos().x() - (gridWidth/2) + (cellWidth/2) - horizontalOffset;
    qreal posY = info.pos().y() - (gridHeight/2) + (cellHeight/2) - verticalOffset;

    //Lock the grid alignment
    posX = posX - std::fmod(posX, cellWidth) + horizontalOffset;
    posY = posY - std::fmod(posY, cellHeight) + verticalOffset;
    const QRectF dabRect(posX , posY , cellWidth, cellHeight);
    const QRect dabRectAligned = dabRect.toAlignedRect();

    divide = qMax(1, divide);
    const qreal yStep = cellHeight / (qreal)divide;
    const qreal xStep = cellWidth / (qreal)divide;

    QRectF tile;
    KoColor color(painter()->paintColor());

    QScopedPointer<KisCrossDeviceColorSampler> colorSampler;
    if (m_node) {
        colorSampler.reset(new KisCrossDeviceColorSampler(m_node->paintDevice(), color));
    }

    qreal vertBorder = m_properties.grid_vertical_border * additionalScale;
    qreal horzBorder = m_properties.grid_horizontal_border * additionalScale;
    if (m_properties.grid_random_border) {
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
    for (int y = 0; y < (gridHeight)/yStep; y++) {
        for (int x = 0; x < (gridWidth)/xStep; x++) {
            // determine the tile size
            tile = QRectF(dabRect.x() + x * xStep, dabRect.y() + y * yStep, xStep, yStep);
            tile.adjust(vertBorder, horzBorder, -vertBorder, -horzBorder);
            tile = tile.normalized();

            // do color transformation
            if (shouldColor) {
                if (colorSampler && m_colorProperties.sampleInputColor) {
                    colorSampler->sampleOldColor(tile.center().x(), tile.center().y(), color.data());
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
            switch (m_properties.grid_shape) {
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
