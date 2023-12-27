/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _FISHEYEPOINT_ASSISTANT_H_
#define _FISHEYEPOINT_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include "Ellipse.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
//class FisheyePoint;

class FisheyePointAssistant : public KisPaintingAssistant
{
public:
    FisheyePointAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny, qreal moveThresholdPt) override;
    void adjustLine(QPointF &point, QPointF& strokeBegin) override;

    QPointF getDefaultEditorPosition() const override;
    int numHandles() const override { return 3; }

    bool isAssistantComplete() const override;

protected:
    QRect boundingRect() const override;
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin, qreal moveThresholdPt);
    explicit FisheyePointAssistant(const FisheyePointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
    mutable Ellipse e;
    mutable Ellipse extraE;
};

class FisheyePointAssistantFactory : public KisPaintingAssistantFactory
{
public:
    FisheyePointAssistantFactory();
    ~FisheyePointAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
