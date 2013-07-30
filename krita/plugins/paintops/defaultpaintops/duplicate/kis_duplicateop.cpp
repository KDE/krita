/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_duplicateop.h"

#include <string.h>

#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDomElement>
#include <QHBoxLayout>
#include <QToolButton>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoInputDevice.h>
#include <KoColorSpaceRegistry.h>

#include <kis_brush.h>
#include <kis_datamanager.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_perspective_grid.h>
#include <kis_random_sub_accessor.h>
#include <kis_fixed_paint_device.h>
#include <kis_iterator_ng.h>

#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_settings_widget.h"
#include "kis_duplicateop_option.h"

KisDuplicateOp::KisDuplicateOp(const KisDuplicateOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(settings, painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_healing = settings->getBool(DUPLICATE_HEALING);
    m_perspectiveCorrection = settings->getBool(DUPLICATE_CORRECT_PERSPECTIVE);
    m_moveSourcePoint = settings->getBool(DUPLICATE_MOVE_SOURCE_POINT);

    m_srcdev = source()->createCompositionSourceDevice();
}

KisDuplicateOp::~KisDuplicateOp()
{
}

qreal KisDuplicateOp::minimizeEnergy(const qreal* m, qreal* sol, int w, int h)
{
    int rowstride = 3 * w;
    qreal err = 0;
    memcpy(sol, m, 3* sizeof(qreal) * w);
    m += rowstride;
    sol += rowstride;
    for (int i = 1; i < h - 1; i++) {
        memcpy(sol, m, 3* sizeof(qreal));
        m += 3; sol += 3;
        for (int j = 3; j < rowstride - 3; j++) {
            qreal tmp = *sol;
            *sol = ((*(m - 3) + *(m + 3) + *(m - rowstride) + *(m + rowstride)) + 2 * *m) / 6;
            qreal diff = *sol - tmp;
            err += diff * diff;
            m ++; sol ++;
        }
        memcpy(sol, m, 3* sizeof(qreal));
        m += 3; sol += 3;
    }
    memcpy(sol, m, 3* sizeof(qreal) * w);
    return err;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))


KisSpacingInformation KisDuplicateOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return 1.0;

    if (!m_duplicateStartIsSet) {
        m_duplicateStartIsSet = true;
        m_duplicateStart = info.pos();
    }

    if (!source()) return 1.0;

    KisBrushSP brush = m_brush;
    if (!brush)
        return 1.0;

    if (! brush->canPaintFor(info))
        return 1.0;

    qreal scale = m_sizeOption.apply(info);
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return spacing(info.pressure());

    QPointF hotSpot = brush->hotSpot(scale, scale, 0, info);
    QPointF pt = info.pos() - hotSpot;

    setCurrentScale(scale);

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x, y;
    qreal xFraction, yFraction; // will not be used
    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    QPoint srcPoint;

    if(m_moveSourcePoint)
    {
        srcPoint = QPoint(x - static_cast<qint32>(settings->offset().x()),
                          y - static_cast<qint32>(settings->offset().y()));
    } else {
        srcPoint = QPoint(static_cast<qint32>(settings->position().x() - hotSpot.x()),
                          static_cast<qint32>(settings->position().y() - hotSpot.y()));
    }

    qint32 sw = brush->maskWidth(scale, 0.0, xFraction, yFraction, info);
    qint32 sh = brush->maskHeight(scale, 0.0, xFraction, yFraction, info);

    if (srcPoint.x() < 0)
        srcPoint.setX(0);

    if (srcPoint.y() < 0)
        srcPoint.setY(0);

    // Perspective correction ?
    KisImageWSP image = settings->m_image;
    if (m_perspectiveCorrection && image && image->perspectiveGrid()->countSubGrids() == 1) {
        Matrix3qreal startM = Matrix3qreal::Identity();
        Matrix3qreal endM = Matrix3qreal::Identity();

        // First look for the grid corresponding to the start point
        KisSubPerspectiveGrid* subGridStart = *image->perspectiveGrid()->begin();
        QRect r = QRect(0, 0, image->width(), image->height());

#if 1
        if (subGridStart) {
            startM = KisPerspectiveMath::computeMatrixTransfoFromPerspective(r, *subGridStart->topLeft(), *subGridStart->topRight(), *subGridStart->bottomLeft(), *subGridStart->bottomRight());
        }
#endif
#if 1
        // Second look for the grid corresponding to the end point
        KisSubPerspectiveGrid* subGridEnd = *image->perspectiveGrid()->begin();
        if (subGridEnd) {
            endM = KisPerspectiveMath::computeMatrixTransfoToPerspective(*subGridEnd->topLeft(), *subGridEnd->topRight(), *subGridEnd->bottomLeft(), *subGridEnd->bottomRight(), r);
        }
#endif

        // Compute the translation in the perspective transformation space:
        QPointF positionStartPaintingT = KisPerspectiveMath::matProd(endM, QPointF(m_duplicateStart));
        QPointF duplicateStartPositionT = KisPerspectiveMath::matProd(endM, QPointF(m_duplicateStart) - QPointF(settings->offset()));
        QPointF translat = duplicateStartPositionT - positionStartPaintingT;

        KisRectIteratorSP dstIt = m_srcdev->createRectIteratorNG(0, 0, sw, sh);
        KisRandomSubAccessorSP srcAcc = source()->createRandomSubAccessor();
        //Action
        do {
            QPointF p =  KisPerspectiveMath::matProd(startM, KisPerspectiveMath::matProd(endM, QPointF(dstIt->x() + x, dstIt->y() + y)) + translat);
            srcAcc->moveTo(p);
            srcAcc->sampledOldRawData(dstIt->rawData());
        } while (dstIt->nextPixel());


    } else {
        KisPainter copyPainter(m_srcdev);
        copyPainter.setCompositeOp(COMPOSITE_COPY);
        copyPainter.bitBltOldData(0, 0, source(), srcPoint.x(), srcPoint.y(), sw, sh);
        copyPainter.end();
    }

    // heal ?

    if (m_healing) {
        quint16 srcData[4];
        quint16 tmpData[4];
        qreal* matrix = new qreal[ 3 * sw * sh ];
        // First divide
        const KoColorSpace* srcCs = source()->colorSpace();
        const KoColorSpace* tmpCs = source()->colorSpace();
        KisHLineConstIteratorSP srcIt = source()->createHLineConstIteratorNG(x, y, sw);
        KisHLineIteratorSP tmpIt = m_srcdev->createHLineIteratorNG(0, 0, sw);
        qreal* matrixIt = &matrix[0];
        for (int j = 0; j < sh; j++) {
            for (int i = 0; i < sw; i++) {
                srcCs->toLabA16(srcIt->oldRawData(), (quint8*)srcData, 1);
                tmpCs->toLabA16(tmpIt->rawData(), (quint8*)tmpData, 1);
                // Division
                for (int k = 0; k < 3; k++) {
                    matrixIt[k] = srcData[k] / (qreal)qMax((int)tmpData [k], 1);
                }
                srcIt->nextPixel();
                tmpIt->nextPixel();
                matrixIt += 3;
            }
            srcIt->nextRow();
            tmpIt->nextRow();
        }
        // Minimize energy
        {
            int iter = 0;
            qreal err;
            qreal* solution = new qreal [ 3 * sw * sh ];
            do {
                err = minimizeEnergy(&matrix[0], &solution[0], sw, sh);

                // swap pointers
                qreal *tmp = matrix;
                matrix = solution;
                solution = tmp;

                iter++;
            } while (err > 0.00001 && iter < 100);
            delete [] solution;
        }

        // Finaly multiply
        KisHLineIteratorSP srcIt2 = source()->createHLineIteratorNG(x, y, sw);
        KisHLineIteratorSP tmpIt2 = m_srcdev->createHLineIteratorNG(0, 0, sw);
        matrixIt = &matrix[0];
        for (int j = 0; j < sh; j++) {
            for (int i = 0; i < sw; i++) {
                srcCs->toLabA16(srcIt2->rawData(), (quint8*)srcData, 1);
                tmpCs->toLabA16(tmpIt2->rawData(), (quint8*)tmpData, 1);
                // Multiplication
                for (int k = 0; k < 3; k++) {
                    tmpData[k] = (int)CLAMP(matrixIt[k] * qMax((int) tmpData[k], 1), 0, 65535);
                }
                tmpCs->fromLabA16((quint8*)tmpData, tmpIt2->rawData(), 1);
                srcIt2->nextPixel();
                tmpIt2->nextPixel();
                matrixIt += 3;
            }
            srcIt2->nextRow();
            tmpIt2->nextRow();
        }
        delete [] matrix;
    }

    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    KisFixedPaintDeviceSP dab =
        m_dabCache->fetchDab(cs, color, scale, scale,
                             0.0, info);

    QRect dstRect = QRect(x, y, dab->bounds().width(), dab->bounds().height());
    if (dstRect.isEmpty()) return 1.0;

    painter()->bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                                        m_srcdev, dab,
                                        dstRect.width(),
                                        dstRect.height());

    painter()->renderMirrorMaskSafe(dstRect, m_srcdev, 0, 0, dab,
                                    !m_dabCache->needSeparateOriginal());

    return effectiveSpacing(dstRect.width(), dstRect.height());
}
