/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004, 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_duplicateop.h"
#include "kis_duplicateop_p.h"

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
#include <KoCompositeOpRegistry.h>
#include <KoColorSpaceRegistry.h>

#include <kis_brush.h>
#include <kis_datamanager.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <brushengine/kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_random_sub_accessor.h>
#include <kis_fixed_paint_device.h>
#include <kis_iterator_ng.h>
#include <kis_spacing_information.h>

#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_settings_widget.h"
#include "kis_duplicateop_option.h"

KisDuplicateOp::KisDuplicateOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_image(image)
    , m_node(node)
    , m_settings(static_cast<KisDuplicateOpSettings*>(const_cast<KisPaintOpSettings*>(settings.data())))
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_opacityOption.resetAllSensors();

    m_healing = settings->getBool(DUPLICATE_HEALING);
    m_perspectiveCorrection = settings->getBool(DUPLICATE_CORRECT_PERSPECTIVE);
    m_moveSourcePoint = settings->getBool(DUPLICATE_MOVE_SOURCE_POINT);
    m_cloneFromProjection = settings->getBool(DUPLICATE_CLONE_FROM_PROJECTION);

    m_srcdev = source()->createCompositionSourceDevice();
}

KisDuplicateOp::~KisDuplicateOp()
{
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

KisSpacingInformation KisDuplicateOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return KisSpacingInformation(1.0);

    KisBrushSP brush = m_brush;
    if (!brush)
        return KisSpacingInformation(1.0);

    if (!brush->canPaintFor(info))
        return KisSpacingInformation(1.0);

    if (!m_duplicateStartIsSet) {
        m_duplicateStartIsSet = true;
        m_duplicateStart = info.pos();
    }

    KisPaintDeviceSP realSourceDevice;

    if (m_cloneFromProjection && m_image) {
        realSourceDevice = m_image->projection();
    }
    else {
        KisNodeSP externalSourceNode = m_settings->sourceNode();

        /**
         * The saved layer might have been deleted by then, so check if it
         * still belongs to a graph
         */
        if (!externalSourceNode || !externalSourceNode->graphListener()) {
            externalSourceNode = m_node;
        }

        realSourceDevice = externalSourceNode->projection();
    }

    qreal rotation = m_rotationOption.apply(info);

    qreal opacity = m_opacityOption.apply(painter(),info);

    qreal scale = m_sizeOption.apply(info);

    if (checkSizeTooSmall(scale)) return KisSpacingInformation();
    KisDabShape shape(scale, 1.0, rotation);


    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP dab =
        m_dabCache->fetchDab(cs, color, info.pos(),
                             shape,
                             info, 1.0,
                             &dstRect);

    if (dstRect.isEmpty()) return KisSpacingInformation(1.0);


    QPoint srcPoint;

    if (m_moveSourcePoint) {
        srcPoint = (dstRect.topLeft() - m_settings->offset()).toPoint();
    }
    else {
        QPointF hotSpot = brush->hotSpot(shape, info);
        srcPoint = (m_settings->position() - hotSpot).toPoint();
    }

    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    // Perspective correction ?


    // if (m_perspectiveCorrection && m_image && m_image->perspectiveGrid()->countSubGrids() == 1) {
    //     Matrix3qreal startM = Matrix3qreal::Identity();
    //     Matrix3qreal endM = Matrix3qreal::Identity();

    //     // First look for the grid corresponding to the start point
    //     KisSubPerspectiveGrid* subGridStart = *m_image->perspectiveGrid()->begin();
    //     QRect r = QRect(0, 0, m_image->width(), m_image->height());

    //     if (subGridStart) {
    //         startM = KisPerspectiveMath::computeMatrixTransfoFromPerspective(r, *subGridStart->topLeft(), *subGridStart->topRight(), *subGridStart->bottomLeft(), *subGridStart->bottomRight());
    //     }

    //     // Second look for the grid corresponding to the end point
    //     KisSubPerspectiveGrid* subGridEnd = *m_image->perspectiveGrid()->begin();
    //     if (subGridEnd) {
    //         endM = KisPerspectiveMath::computeMatrixTransfoToPerspective(*subGridEnd->topLeft(), *subGridEnd->topRight(), *subGridEnd->bottomLeft(), *subGridEnd->bottomRight(), r);
    //     }

    //     // Compute the translation in the perspective transformation space:
    //     QPointF positionStartPaintingT = KisPerspectiveMath::matProd(endM, QPointF(m_duplicateStart));
    //     QPointF duplicateStartPositionT = KisPerspectiveMath::matProd(endM, QPointF(m_duplicateStart) - QPointF(m_settings->offset()));
    //     QPointF translat = duplicateStartPositionT - positionStartPaintingT;

    //     KisSequentialIterator dstIt(m_srcdev, QRect(0, 0, sw, sh));
    //     KisRandomSubAccessorSP srcAcc = realSourceDevice->createRandomSubAccessor();

    //     //Action
    //     while (dstIt.nextPixel()) {
    //         QPointF p =  KisPerspectiveMath::matProd(startM, KisPerspectiveMath::matProd(endM, QPointF(dstIt.x() + dstRect.x(), dstIt.y() + dstRect.y())) + translat);
    //         srcAcc->moveTo(p);
    //         srcAcc->sampledOldRawData(dstIt.rawData());
    //     }


    // }
    // else
    {
        KisPainter copyPainter(m_srcdev);
        copyPainter.setCompositeOp(COMPOSITE_COPY);
        copyPainter.bitBltOldData(0, 0, realSourceDevice, srcPoint.x(), srcPoint.y(), sw, sh);
        copyPainter.end();
    }

    // heal ?
    if (m_healing) {
        QRect healRect(dstRect);

        const bool smallWidth = healRect.width() < 3;
        const bool smallHeight = healRect.height() < 3;

        if (smallWidth || smallHeight) {
            healRect.adjust(-smallWidth, -smallHeight, smallWidth, smallHeight);
        }

        const int healSW = healRect.width();
        const int healSH = healRect.height();


        quint16 srcData[4];
        quint16 tmpData[4];
        QScopedArrayPointer<qreal> matrix(new qreal[ 3 * healSW * healSH ]);
        // First divide
        const KoColorSpace* srcCs = realSourceDevice->colorSpace();
        const KoColorSpace* tmpCs = m_srcdev->colorSpace();
        KisHLineConstIteratorSP srcIt = realSourceDevice->createHLineConstIteratorNG(healRect.x(), healRect.y() , healSW);
        KisHLineIteratorSP tmpIt = m_srcdev->createHLineIteratorNG(0, 0, healSW);
        qreal* matrixIt = matrix.data();
        for (int j = 0; j < healSH; j++) {
            for (int i = 0; i < healSW; i++) {
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
            QScopedArrayPointer<qreal> solution(new qreal[ 3 * healSW * healSH ]);

            do {
                err = DuplicateOpUtils::minimizeEnergy(matrix.data(), solution.data(), healSW, healSH);

                solution.swap(matrix);

                iter++;
            } while (err > 0.00001 && iter < 100);
        }

        // Finally multiply
        KisHLineIteratorSP tmpIt2 = m_srcdev->createHLineIteratorNG(0, 0, healSW);
        matrixIt = &matrix[0];
        for (int j = 0; j < healSH; j++) {
            for (int i = 0; i < healSW; i++) {
                tmpCs->toLabA16(tmpIt2->rawData(), (quint8*)tmpData, 1);
                // Multiplication
                for (int k = 0; k < 3; k++) {
                    tmpData[k] = (int)CLAMP(matrixIt[k] * qMax((int) tmpData[k], 1), 0, 65535);
                }
                tmpCs->fromLabA16((quint8*)tmpData, tmpIt2->rawData(), 1);
                tmpIt2->nextPixel();
                matrixIt += 3;
            }
            tmpIt2->nextRow();
        }
    }

    painter()->bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                                        m_srcdev, dab,
                                        dstRect.width(),
                                        dstRect.height());

    painter()->renderMirrorMaskSafe(dstRect, m_srcdev, 0, 0, dab,
                                    !m_dabCache->needSeparateOriginal());

    painter()->setOpacity(opacity);

    return effectiveSpacing(scale);
}

KisSpacingInformation KisDuplicateOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    return effectiveSpacing(m_sizeOption.apply(info));
}
