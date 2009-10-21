/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <qtoolbutton.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoInputDevice.h>

#include <kis_brush.h>
#include <kis_datamanager.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_perspective_grid.h>
#include <kis_random_sub_accessor.h>
#include <kis_fixed_paint_device.h>

#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_settings_widget.h"


KisDuplicateOp::KisDuplicateOp(const KisDuplicateOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    if (settings && settings->m_options) {
        Q_ASSERT(settings->m_options->m_brushOption);
        m_brush = settings->m_options->m_brushOption->brush();
    }
}

KisDuplicateOp::~KisDuplicateOp()
{
}

double KisDuplicateOp::minimizeEnergy(const double* m, double* sol, int w, int h)
{
    int rowstride = 3 * w;
    double err = 0;
    memcpy(sol, m, 3* sizeof(double) * w);
    m += rowstride;
    sol += rowstride;
    for (int i = 1; i < h - 1; i++) {
        memcpy(sol, m, 3* sizeof(double));
        m += 3; sol += 3;
        for (int j = 3; j < rowstride - 3; j++) {
            double tmp = *sol;
            *sol = ((*(m - 3) + *(m + 3) + *(m - rowstride) + *(m + rowstride)) + 2 * *m) / 6;
            double diff = *sol - tmp;
            err += diff * diff;
            m ++; sol ++;
        }
        memcpy(sol, m, 3* sizeof(double));
        m += 3; sol += 3;
    }
    memcpy(sol, m, 3* sizeof(double) * w);
    return err;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))


void KisDuplicateOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return;

    if (!m_duplicateStartIsSet) {
        m_duplicateStartIsSet = true;
        m_duplicateStart = info.pos();
    }

    bool heal = settings->healing();

    if (!source()) return;

    KisBrushSP brush = m_brush;
    if (!m_brush) {
        if (settings->m_options) {
            m_brush = settings->m_options->m_brushOption->brush();
            brush = m_brush;
        } else {
            return;
        }
    }

    if (! brush->canPaintFor(info))
        return;

    double scale = KisPaintOp::scaleForPressure(info.pressure());
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);
    xFraction = yFraction = 0.0;

    QPointF srcPointF = pt - settings->offset();
    QPoint srcPoint = QPoint(x - static_cast<qint32>(settings->offset().x()),
                             y - static_cast<qint32>(settings->offset().y()));


    qint32 sw = brush->maskWidth(scale, 0.0);
    qint32 sh = brush->maskHeight(scale, 0.0);

    if (srcPoint.x() < 0)
        srcPoint.setX(0);

    if (srcPoint.y() < 0)
        srcPoint.setY(0);
    if (!(m_srcdev && !(*m_srcdev->colorSpace() == *source()->colorSpace()))) {
        m_srcdev = new KisPaintDevice(source()->colorSpace());
    }
    Q_CHECK_PTR(m_srcdev);

    // Perspective correction ?
    KisImageWSP image = settings->m_image;
    if (settings->perspectiveCorrection() && image && image->perspectiveGrid()->countSubGrids() == 1) {
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
        KisRectIteratorPixel dstIt = m_srcdev->createRectIterator(0, 0, sw, sh);
        KisRandomSubAccessorPixel srcAcc = source()->createRandomSubAccessor();
        //Action
        while (!dstIt.isDone()) {
            if (dstIt.isSelected()) {
                QPointF p =  KisPerspectiveMath::matProd(startM, KisPerspectiveMath::matProd(endM, QPointF(dstIt.x() + x, dstIt.y() + y)) + translat);
                srcAcc.moveTo(p);
                srcAcc.sampledOldRawData(dstIt.rawData());
            }
            ++dstIt;
        }


    } else {
        // TODO make it a mode to get the behaviour where the source is rawData
        // Or, copy the source data on the temporary device:
//         KisPainter copyPainter(m_srcdev);
//         copyPainter.setCompositeOp(COMPOSITE_COPY);
//         copyPainter.bitBlt(0, 0, source(), srcPoint.x(), srcPoint.y(), sw, sh);
//         copyPainter.end();
        // Do the copy manually to access old raw data
        KisHLineIteratorPixel dstIt = m_srcdev->createHLineIterator(0, 0, sw);
        KisHLineConstIteratorPixel srcIt = source()->createHLineConstIterator(srcPoint.x(), srcPoint.y(), sw);
        int pixelSize = m_srcdev->pixelSize();
        for (int i = 0; i < sh; ++i) {
            while (!dstIt.isDone()) {
                memcpy(dstIt.rawData(), srcIt.oldRawData(), pixelSize);
                ++dstIt;
                ++srcIt;
            }
            dstIt.nextRow();
            srcIt.nextRow();
        }
    }

    // heal ?

    if (heal) {
        quint16 dataDevice[4];
        quint16 dataSrcDev[4];
        double* matrix = new double[ 3 * sw * sh ];
        // First divide
        const KoColorSpace* deviceCs = source()->colorSpace();
        KisHLineConstIteratorPixel deviceIt = source()->createHLineConstIterator(x, y, sw);
        KisHLineIteratorPixel srcDevIt = m_srcdev->createHLineIterator(0, 0, sw);
        double* matrixIt = &matrix[0];
        for (int j = 0; j < sh; j++) {
            for (int i = 0; !srcDevIt.isDone(); i++) {
                deviceCs->toLabA16(deviceIt.rawData(), (quint8*)dataDevice, 1);
                deviceCs->toLabA16(srcDevIt.rawData(), (quint8*)dataSrcDev, 1);
                // Division
                for (int k = 0; k < 3; k++) {
                    matrixIt[k] = dataDevice[k] / (double)qMax((int)dataSrcDev [k], 1);
                }
                ++deviceIt;
                ++srcDevIt;
                matrixIt += 3;
            }
            deviceIt.nextRow();
            srcDevIt.nextRow();
        }
        // Minimize energy
        {
            int iter = 0;
            double err;
            double* solution = new double [ 3 * sw * sh ];
            do {
                err = minimizeEnergy(&matrix[0], &solution[0], sw, sh);
                memcpy(&matrix[0], &solution[0], sw * sh * 3 * sizeof(double));
                iter++;
            } while (err < 0.00001 && iter < 100);
            delete [] solution;
        }

        // Finaly multiply
        deviceIt = source()->createHLineIterator(x, y, sw);
        srcDevIt = m_srcdev->createHLineIterator(0, 0, sw);
        matrixIt = &matrix[0];
        for (int j = 0; j < sh; j++) {
            for (int i = 0; !srcDevIt.isDone(); i++) {
                deviceCs->toLabA16(deviceIt.rawData(), (quint8*)dataDevice, 1);
                deviceCs->toLabA16(srcDevIt.rawData(), (quint8*)dataSrcDev, 1);
                // Multiplication
                for (int k = 0; k < 3; k++) {
                    dataSrcDev[k] = (int)CLAMP(matrixIt[k] * qMax((int) dataSrcDev[k], 1), 0, 65535);
                }
                deviceCs->fromLabA16((quint8*)dataSrcDev, srcDevIt.rawData(), 1);
                ++deviceIt;
                ++srcDevIt;
                matrixIt += 3;
            }
            deviceIt.nextRow();
            srcDevIt.nextRow();
        }
        delete [] matrix;
    }

    KisFixedPaintDeviceSP fixedDab = new KisFixedPaintDevice(m_srcdev->colorSpace());
    fixedDab->setRect(QRect(0, 0, sw, sh));
    fixedDab->initialize();

    m_srcdev->readBytes(fixedDab->data(), fixedDab->bounds());
    brush->mask(fixedDab, scale, scale, 0.0, info, xFraction, yFraction);
    m_srcdev->writeBytes(fixedDab->data(), fixedDab->bounds());

    QRect dabRect = QRect(0, 0, brush->maskWidth(scale, 0.0), brush->maskHeight(scale, 0.0));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();

    painter()->bitBlt(dstRect.x(), dstRect.y(), m_srcdev, sx, sy, sw, sh);


}
