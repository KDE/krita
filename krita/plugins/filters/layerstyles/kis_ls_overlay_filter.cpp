/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_ls_overlay_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <KoUpdater.h>
#include <KoPattern.h>

#include <KoAbstractGradient.h>

#include "psd.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_gradient_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"

#include "kis_psd_layer_style.h"
#include "kis_layer_style_filter_environment.h"

#include "kis_ls_utils.h"



KisLsOverlayFilter::KisLsOverlayFilter(Mode mode)
    : KisLayerStyleFilter(KoID("lsoverlay", i18n("Overlay (style)"))),
      m_mode(mode)
{
}

void KisLsOverlayFilter::applyOverlay(KisPaintDeviceSP srcDevice,
                                      KisPaintDeviceSP dstDevice,
                                      const QRect &applyRect,
                                      const psd_layer_effects_shadow_base *config,
                                      KisPSDLayerStyleSP style,
                                      KisLayerStyleFilterEnvironment *env) const
{
    if (applyRect.isEmpty()) return;

    KisPaintDeviceSP fillDevice = new KisPaintDevice(dstDevice->colorSpace());

    if (m_mode == Color) {
        const psd_layer_effects_color_overlay *co = style->colorOverlay();
        KoColor color(co->color(), dstDevice->colorSpace());
        fillDevice->setDefaultPixel(color.data());

    } else if (m_mode == Pattern) {
        const psd_layer_effects_pattern_overlay *po = style->patternOverlay();

        if (po->scale() != 100) {
            qWarning() << "KisLsOverlayFilter::applyOverlay(): Pattern scaling is NOT implemented!";
        }

        QSize psize(po->pattern()->width(), po->pattern()->height());

        QPoint patternOffset(qreal(psize.width()) * po->horizontalPhase() / 100,
                             qreal(psize.height()) * po->horizontalPhase() / 100);

        const QRect boundsRect = po->linkWithLayer() ?
            env->layerBounds() : env->defaultBounds();

        patternOffset += boundsRect.topLeft();

        patternOffset.rx() %= psize.width();
        patternOffset.ry() %= psize.height();

        QRect fillRect = applyRect | applyRect.translated(patternOffset);

        KisFillPainter gc(fillDevice);
        gc.fillRect(fillRect.x(), fillRect.y(),
                    fillRect.width(), fillRect.height(), po->pattern());
        gc.end();

        fillDevice->setX(-patternOffset.x());
        fillDevice->setY(-patternOffset.y());

    } else if (m_mode == Gradient) {
        const psd_layer_effects_gradient_overlay *go = style->gradientOverlay();

        const QRect boundsRect = go->alignWithLayer() ?
            env->layerBounds() : env->defaultBounds();

        QPoint center = boundsRect.center();
        center += QPoint(boundsRect.width() * go->gradientXOffset() / 100,
                         boundsRect.height() * go->gradientYOffset() / 100);

        int width = (boundsRect.width() * go->scale() + 100) / 200;
        int height = (boundsRect.height() * go->scale() + 100) / 200;

        /* copy paste from libpsd */

        int angle = go->angle();
        int corner_angle = (int)(atan((qreal)boundsRect.height() / boundsRect.width()) * 180 / M_PI + 0.5);
        int sign_x = 1;
        int sign_y = 1;


	if(angle < 0) {
            angle += 360;
        }

        if (angle >= 90 && angle < 180) {
            angle = 180 - angle;
            sign_x = -1;
        } else if (angle >= 180 && angle < 270) {
            angle = angle - 180;
            sign_x = -1;
            sign_y = -1;
        } else if (angle >= 270 && angle <= 360) {
            angle = 360 - angle;
            sign_y = -1;
        }

        int radius_x = 0;
        int radius_y = 0;

        if (angle <= corner_angle) {
            radius_x = width;
            radius_y = (int)(radius_x * tan(kisDegreesToRadians(qreal(angle))) + 0.5);
        } else {
            radius_y = height;
            radius_x = (int)(radius_y / tan(kisDegreesToRadians(qreal(angle))) + 0.5);
        }

        int radius_corner = (int)(std::sqrt((qreal)(radius_x * radius_x + radius_y * radius_y)) + 0.5);

        /* end of copy paste from libpsd */

        KisGradientPainter gc(fillDevice);
        gc.setGradient(go->gradient());
        QPointF gradStart;
        QPointF gradEnd;
        KisGradientPainter::enumGradientRepeat repeat =
            KisGradientPainter::GradientRepeatNone;

        QPoint rectangularOffset(sign_x * radius_x, -sign_y * radius_y);


        switch(go->style())
	{
        case psd_gradient_style_linear:
            gc.setGradientShape(KisGradientPainter::GradientShapeLinear);
            repeat = KisGradientPainter::GradientRepeatNone;
            gradStart = center - rectangularOffset;
            gradEnd = center + rectangularOffset;
            break;

        case psd_gradient_style_radial:
            gc.setGradientShape(KisGradientPainter::GradientShapeRadial);
            repeat = KisGradientPainter::GradientRepeatNone;
            gradStart = center;
            gradEnd = center + QPointF(radius_corner, 0);
            break;

        case psd_gradient_style_angle:
            gc.setGradientShape(KisGradientPainter::GradientShapeConical);
            repeat = KisGradientPainter::GradientRepeatNone;
            gradStart = center;
            gradEnd = center + rectangularOffset;
            break;

        case psd_gradient_style_reflected:
            gc.setGradientShape(KisGradientPainter::GradientShapeLinear);
            repeat = KisGradientPainter::GradientRepeatAlternate;
            gradStart = center - rectangularOffset;
            gradEnd = center;
            break;

        case psd_gradient_style_diamond:
            gc.setGradientShape(KisGradientPainter::GradientShapeBiLinear);
            repeat = KisGradientPainter::GradientRepeatNone;
            gradStart = center - rectangularOffset;
            gradEnd = center + rectangularOffset;
            break;
        default:
            qFatal("Gradient Overlay: unknown switch case!");
            break;
	}

        gc.paintGradient(gradStart, gradEnd,
                         repeat, 0.0,
                         go->reverse(),
                         applyRect);
    }

    KisPainter gc(dstDevice);
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(applyRect.topLeft(), srcDevice, applyRect);

    const QString compositeOp = config->blendMode();
    const quint8 opacityU8 = 255.0 / 100.0 * config->opacity();

    gc.setCompositeOp(compositeOp);
    gc.setOpacity(opacityU8);

    QBitArray channelFlags = dstDevice->colorSpace()->channelFlags(true, false);
    gc.setChannelFlags(channelFlags);
    gc.bitBlt(applyRect.topLeft(), fillDevice, applyRect);

    gc.end();
}

const psd_layer_effects_shadow_base*
KisLsOverlayFilter::getOverlayStruct(KisPSDLayerStyleSP style) const
{
    const psd_layer_effects_shadow_base *config = 0;

    if (m_mode == Color) {
        config = style->colorOverlay();
    } else if (m_mode == Gradient) {
        config = style->gradientOverlay();
    } else if (m_mode == Pattern) {
        config = style->patternOverlay();
    }

    return config;
}

void KisLsOverlayFilter::processDirectly(KisPaintDeviceSP src,
                                         KisPaintDeviceSP dst,
                                         const QRect &applyRect,
                                         KisPSDLayerStyleSP style,
                                         KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(env);
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_shadow_base *config = getOverlayStruct(style);
    if (!config->effectEnabled()) return;

    applyOverlay(src, dst, applyRect, config, style, env);
}

QRect KisLsOverlayFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    Q_UNUSED(style);
    return rect;
}

QRect KisLsOverlayFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style) const
{
    Q_UNUSED(style);
    return rect;
}
