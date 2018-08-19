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
#include "kritaui_export.h"
#include <brushengine/kis_paint_information.h>

class KoPointerEvent;
class KisToolFreehand;
class KisCoordinatesConverter;
class KisSpeedSmoother;
class KoCanvasResourceManager;

class KRITAUI_EXPORT KisPaintingInformationBuilder : public QObject
{
    Q_OBJECT

public:
    KisPaintingInformationBuilder();
    ~KisPaintingInformationBuilder() override;

    KisPaintInformation startStroke(KoPointerEvent *event, int timeElapsed, const KoCanvasResourceManager *manager);

    KisPaintInformation continueStroke(KoPointerEvent *event,
                                       int timeElapsed);

    KisPaintInformation hover(const QPointF &imagePoint,
                              const KoPointerEvent *event);

    qreal pressureToCurve(qreal pressure);

protected Q_SLOTS:
    void updateSettings();

protected:
    virtual QPointF adjustDocumentPoint(const QPointF &point, const QPointF &startPoint);
    virtual QPointF documentToImage(const QPointF &point);
    virtual QPointF imageToView(const QPointF &point);
    virtual qreal calculatePerspective(const QPointF &documentPoint);

private:

    KisPaintInformation createPaintingInformation(KoPointerEvent *event,
                                                  int timeElapsed);

    /**
     * Defines how many discrete samples are stored in a precomputed array
     * of different pressures.
     */
    static const int LEVEL_OF_PRESSURE_RESOLUTION;

private:
    QVector<qreal> m_pressureSamples;
    QPointF m_startPoint;
    QScopedPointer<KisSpeedSmoother> m_speedSmoother;
    bool m_pressureDisabled;
};

class KRITAUI_EXPORT KisConverterPaintingInformationBuilder : public KisPaintingInformationBuilder
{
    Q_OBJECT

public:
    KisConverterPaintingInformationBuilder(const KisCoordinatesConverter *converter);

protected:
    QPointF documentToImage(const QPointF &point) override;
    QPointF imageToView(const QPointF &point) override;

private:
    const KisCoordinatesConverter *m_converter;
};

class KRITAUI_EXPORT KisToolFreehandPaintingInformationBuilder : public KisPaintingInformationBuilder
{
    Q_OBJECT

public:
    KisToolFreehandPaintingInformationBuilder(KisToolFreehand *tool);

protected:
    QPointF documentToImage(const QPointF &point) override;
    QPointF imageToView(const QPointF &point) override;
    QPointF adjustDocumentPoint(const QPointF &point, const QPointF &startPoint) override;
    qreal calculatePerspective(const QPointF &documentPoint) override;

private:
    KisToolFreehand *m_tool;
};

#endif /* __KIS_PAINTING_INFORMATION_BUILDER_H */
