/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_grid_decoration.h"

#include <QPainter>
#include <QPen>
#include <QtMath>
#include <klocalizedstring.h>

#include <KoUnit.h>

#include "kis_grid_config.h"
#include "kis_coordinates_converter.h"

struct KisGridDecoration::Private
{
    KisGridConfig config;
};

KisGridDecoration::KisGridDecoration(KisView* parent)
    : KisCanvasDecoration("grid", parent),
      m_d(new Private)
{
    setPriority(0);
}

KisGridDecoration::~KisGridDecoration()
{

}

void KisGridDecoration::setGridConfig(const KisGridConfig &config)
{
    m_d->config = config;
}

void KisGridDecoration::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    if (!m_d->config.showGrid()) return;

    Q_UNUSED(canvas);

    QTransform transform = converter->imageToWidgetTransform();

    const qreal scale = KoUnit::approxTransformScale(transform);
    const int minWidgetSize = 3;
    const int effectiveSize = qMin(m_d->config.spacing().x(), m_d->config.spacing().y());

    int scaleCoeff = 1;
    quint32 subdivision = m_d->config.subdivision();

    while (qFloor(scale * scaleCoeff * effectiveSize) <= minWidgetSize) {
        if (subdivision > 1) {
            scaleCoeff = subdivision;
            subdivision = 1;
        } else {
            scaleCoeff *= 2;
        }

        if (scaleCoeff > 32768) {
            qWarning() << "WARNING: Grid Scale Coeff is too high! That is surely a bug!";
            return;
        }
    }

    QPen mainPen = m_d->config.penMain();
    QPen subdivisionPen = m_d->config.penSubdivision();

    gc.save();
    gc.setTransform(transform);
    gc.setRenderHints(QPainter::Antialiasing, false);
    gc.setRenderHints(QPainter::Antialiasing, false);


    const QRect imageRectInImagePixels = converter->imageRectInImagePixels();
    const QRectF updateRectInImagePixels =
        converter->documentToImage(updateArea) &
        imageRectInImagePixels;

    // for angles. This will later be a combobox to select different types of options
    // also add options to hide specific lines (vertical, horizontal, angle 1, etc
    KisGridConfig::GridType gridType = m_d->config.gridType();

    if (gridType == KisGridConfig::GRID_RECTANGULAR) {
        qreal x1, y1, x2, y2;
        updateRectInImagePixels.getCoords(&x1, &y1, &x2, &y2);

        // compensate the fact the getCoordt returns off-by-one pixel
        // at the bottom right of the rect.
        x2++;
        y2++;

        {
            // vertical lines
            const int offset = m_d->config.offset().x();
            const int step = scaleCoeff * m_d->config.spacing().x();
            const int lineIndexFirst = qCeil((x1 - offset) / step);
            const int lineIndexLast = qFloor((x2 - offset) / step);

            if (mainPen.style() != Qt::SolidLine) {
                mainPen.setDashOffset(y1 * scale);
            }
            if (subdivisionPen.style() != Qt::SolidLine) {
                subdivisionPen.setDashOffset(y1 * scale);
            }

            for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
                int w = offset + i * step;

                gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
                // we adjusted y2 to draw the grid correctly, clip it now...
                gc.drawLine(QPointF(w, y1),QPointF(w, qMin(y2, qreal(imageRectInImagePixels.bottom() + 1))));
            }
        }

        {
            // horizontal lines
            const int offset = m_d->config.offset().y();
            const int step = scaleCoeff * m_d->config.spacing().y();
            const int lineIndexFirst = qCeil((y1 - offset) / step);
            const int lineIndexLast = qFloor((y2 - offset) / step);

            if (mainPen.style() != Qt::SolidLine) {
                mainPen.setDashOffset(x1 * scale);
            }
            if (subdivisionPen.style() != Qt::SolidLine) {
                subdivisionPen.setDashOffset(x1 * scale);
            }

            for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
                int w = offset + i * step;

                gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
                // we adjusted x2 to draw the grid correctly, clip it now...
                gc.drawLine(QPointF(x1, w),QPointF(qMin(x2, qreal(imageRectInImagePixels.right() + 1)), w));
            }
        }
    }

    if (gridType == KisGridConfig::GRID_ISOMETRIC)  {
        qreal x1, y1, x2, y2;

        // get true coordinates, not just the updateArea
        QRectF trueImageRect = converter->imageRectInImagePixels();
        trueImageRect.getCoords(&x1, &y1, &x2, &y2);

        // compensate the fact the getCoordt returns off-by-one pixel
        // at the bottom right of the rect.
        x2++;
        y2++;

        const int offset = m_d->config.offset().x();
        const int offsetY = m_d->config.offset().y();
        const int cellSpacing = m_d->config.cellSpacing();

        gc.setClipping(true);
        gc.setClipRect(updateRectInImagePixels, Qt::IntersectClip);

        // left angle
        {
            const qreal gridXAngle = m_d->config.angleLeft();
            const qreal bottomRightOfImageY = y2; // this should be the height of the image
            qreal finalY = 0.0;

            // figure out the spacing based off the angle. The spacing needs to be perpendicular to the angle,
            // so we need to do a bit of trig to get the correct spacing.
            qreal correctedAngleSpacing = cellSpacing;
            if (gridXAngle > 0.0) {
                correctedAngleSpacing = cellSpacing / qCos(qDegreesToRadians(gridXAngle));
            }

            qreal counter = qFloor((-(offset + offsetY)) / correctedAngleSpacing);

            while (finalY < bottomRightOfImageY) {

                const qreal w = (counter * correctedAngleSpacing) + offsetY + offset;
                gc.setPen(mainPen);

                // calculate where the ending point will be based off the angle
                const qreal startingY = w;
                const qreal horizontalDistance = x2;

                // qTan takes radians, so convert first before sending it
                const qreal length2 = qTan(qDegreesToRadians(gridXAngle)) * x2;

                finalY = startingY - length2;
                gc.drawLine(QPointF(x1, w), QPointF(horizontalDistance, finalY));

                counter = counter + 1.0;
            }
        }

        // right angle (almost the same thing, except starting the lines on the right side)
        {
            const qreal gridXAngle = m_d->config.angleRight(); // TODO: add another angle property
            const qreal bottomLeftOfImageY = y2;

            // figure out the spacing based off the angle
            qreal correctedAngleSpacing = cellSpacing;
            if (gridXAngle > 0.0) {
                correctedAngleSpacing = cellSpacing / qCos(qDegreesToRadians(gridXAngle));
            }

            // distance is the same (width of the image)
            const qreal horizontalDistance = x2;
            // qTan takes radians, so convert first before sending it
            const qreal length2 = qTan(qDegreesToRadians(gridXAngle)) * horizontalDistance;

            // let's get x, y of the line that starts in the top right corder
            const qreal yLower = 0.0;
            const qreal yHigher = yLower - length2;

            const qreal yLeftFirst = qCeil(yHigher / correctedAngleSpacing) * correctedAngleSpacing;
            const qreal additionalOffset = yLeftFirst - yHigher;
            qreal finalY = 0.0;
            qreal counter = qFloor((-(offsetY - offset)) / correctedAngleSpacing);

            while (finalY < bottomLeftOfImageY) {

                const qreal w = (counter * correctedAngleSpacing) + offsetY - offset + additionalOffset;
                gc.setPen(mainPen);

                // calculate where the ending point will be based off the angle
                const qreal startingY = w;

                finalY = startingY - length2;
                gc.drawLine(QPointF(x2, w), QPointF(0.0, finalY));

                counter = counter + 1.0;
            }
        }


    }

    gc.restore();
}
