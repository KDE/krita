/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KoCanvasResourceProvider;

class KRITAUI_EXPORT KisPaintingInformationBuilder : public QObject
{
    Q_OBJECT

public:
    KisPaintingInformationBuilder();
    ~KisPaintingInformationBuilder() override;

    KisPaintInformation startStroke(KoPointerEvent *event, int timeElapsed, const KoCanvasResourceProvider *manager);

    KisPaintInformation continueStroke(KoPointerEvent *event,
                                       int timeElapsed);

    KisPaintInformation hover(const QPointF &imagePoint,
                              const KoPointerEvent *event,
                              bool isStrokeStarted);

    qreal pressureToCurve(qreal pressure);

protected Q_SLOTS:
    void updateSettings();

protected:
    virtual QPointF adjustDocumentPoint(const QPointF &point, const QPointF &startPoint);
    virtual QPointF documentToImage(const QPointF &point);
    virtual QPointF imageToView(const QPointF &point);
    virtual qreal calculatePerspective(const QPointF &documentPoint);

    virtual qreal canvasRotation() const;
    virtual bool canvasMirroredX() const;
    virtual bool canvasMirroredY() const;

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

    qreal canvasRotation() const override;
    bool canvasMirroredX() const override;
    bool canvasMirroredY() const override;

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

    qreal canvasRotation() const override;
    bool canvasMirroredX() const override;
    bool canvasMirroredY() const override;

private:
    KisToolFreehand *m_tool;
};

#endif /* __KIS_PAINTING_INFORMATION_BUILDER_H */
