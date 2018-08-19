/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_multihand_helper.h"

#include <QTransform>

#include "kis_painter.h"
#include <strokes/KisFreehandStrokeInfo.h>

struct KisToolMultihandHelper::Private
{
    QVector<QTransform> transformations;
};

KisToolMultihandHelper::KisToolMultihandHelper(KisPaintingInformationBuilder *infoBuilder,
                                               const KUndo2MagicString &transactionText)
    : KisToolFreehandHelper(infoBuilder, transactionText)
    , d(new Private)
{
}

KisToolMultihandHelper::~KisToolMultihandHelper()
{
    delete d;
}

void KisToolMultihandHelper::setupTransformations(const QVector<QTransform> &transformations)
{
    d->transformations = transformations;
}

void KisToolMultihandHelper::createPainters(QVector<KisFreehandStrokeInfo*> &strokeInfos,
                                            const KisDistanceInformation &startDist)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        strokeInfos << new KisFreehandStrokeInfo(startDist);
    }
}

void KisToolMultihandHelper::paintAt(const KisPaintInformation &pi)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];

        KisPaintInformation __pi = pi;
        QLineF rotateme(QPointF (0.0,0.0), QPointF (10.0,10.0));
        rotateme.setAngle(__pi.canvasRotation());
        QLineF rotated = transform.map(rotateme);

        __pi.setPos(transform.map(__pi.pos()));
        __pi.setCanvasRotation(rotated.angle());

        if (__pi.canvasMirroredH()) {
            __pi.setCanvasRotation(180-__pi.canvasRotation());
            __pi.setCanvasRotation(__pi.canvasRotation()+180);
        }

        paintAt(i, __pi);
    }
}

void KisToolMultihandHelper::paintLine(const KisPaintInformation &pi1,
                                       const KisPaintInformation &pi2)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        QLineF rotateme(QPointF (0.0,0.0), QPointF (10.0,10.0));
        rotateme.setAngle(__pi1.canvasRotation());
        QLineF rotated = transform.map(rotateme);
        __pi1.setCanvasRotation(rotated.angle());

        rotateme.setAngle(__pi2.canvasRotation());
        rotated = transform.map(rotateme);
        __pi2.setCanvasRotation(rotated.angle());

        //check mirroring
        if (__pi2.canvasMirroredH()) {
            __pi1.setCanvasRotation(180-__pi1.canvasRotation());
            __pi1.setCanvasRotation(__pi1.canvasRotation()+180);
            __pi2.setCanvasRotation(180-__pi2.canvasRotation());
            __pi2.setCanvasRotation(__pi2.canvasRotation()+180);
        }


        paintLine(i, __pi1, __pi2);
    }
}

void KisToolMultihandHelper::paintBezierCurve(const KisPaintInformation &pi1,
                                              const QPointF &control1,
                                              const QPointF &control2,
                                              const KisPaintInformation &pi2)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        QLineF rotateme(QPointF (0.0,0.0), QPointF (10.0,10.0));
        rotateme.setAngle(__pi1.canvasRotation());
        QLineF rotated = transform.map(rotateme);
        __pi1.setCanvasRotation(rotated.angle());

        rotateme.setAngle(__pi2.canvasRotation());
        rotated = transform.map(rotateme);
        __pi2.setCanvasRotation(rotated.angle());

        if (__pi2.canvasMirroredH()) {
            __pi1.setCanvasRotation(180-__pi1.canvasRotation());
            __pi1.setCanvasRotation(__pi1.canvasRotation()+180);
            __pi2.setCanvasRotation(180-__pi2.canvasRotation());
            __pi2.setCanvasRotation(__pi2.canvasRotation()+180);
        }


        QPointF __control1 = transform.map(control1);
        QPointF __control2 = transform.map(control2);

        paintBezierCurve(i, __pi1, __control1, __control2, __pi2);
    }
}
