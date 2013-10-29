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

#ifndef __KIS_PAINTING_INFORMATION_BUILDER_H
#define __KIS_PAINTING_INFORMATION_BUILDER_H

#include <QObject>
#include <QVector>

#include "kis_types.h"
#include "krita_export.h"
#include "kis_paint_information.h"

class KoPointerEvent;
class KisToolFreehand;


class KRITAUI_EXPORT KisPaintingInformationBuilder : public QObject
{
    Q_OBJECT

public:
    KisPaintingInformationBuilder();

    KisPaintInformation startStroke(KoPointerEvent *event, int timeElapsed);

    KisPaintInformation continueStroke(KoPointerEvent *event,
                                       int timeElapsed);
protected slots:
    void updateSettings();

protected:
    virtual QPointF adjustDocumentPoint(const QPointF &point, const QPointF &startPoint);
    virtual QPointF documentToImage(const QPointF &point);
    virtual qreal calculatePerspective(const QPointF &documentPoint);

private:

    KisPaintInformation createPaintingInformation(KoPointerEvent *event,
                                                  int timeElapsed);

    /**
     * Defines how many descret samples are stored in a precomputed array
     * of different pressures.
     */
    static const int LEVEL_OF_PRESSURE_RESOLUTION;
    qreal pressureToCurve(qreal pressure);

private:
    QVector<qreal> m_pressureSamples;
    QPointF m_startPoint;
};


class KRITAUI_EXPORT KisToolPaintingInformationBuilder : public KisPaintingInformationBuilder
{
    Q_OBJECT

public:
    KisToolPaintingInformationBuilder(KisToolFreehand *tool);

protected:
    virtual QPointF adjustDocumentPoint(const QPointF &point, const QPointF &startPoint);
    virtual QPointF documentToImage(const QPointF &point);
    virtual qreal calculatePerspective(const QPointF &documentPoint);

private:
    KisToolFreehand *m_tool;
};

#endif /* __KIS_PAINTING_INFORMATION_BUILDER_H */
