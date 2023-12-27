/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _CURVILINEARPERSPECTIVE_ASSISTANT_H_
#define _CURVILINEARPERSPECTIVE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
//class CurvilinearPerspective;

class CurvilinearPerspectiveAssistant : public KisPaintingAssistant
{
public:
    CurvilinearPerspectiveAssistant();
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin, const bool snapToAny, qreal moveThresholdPt) override;
    void adjustLine(QPointF &point, QPointF& strokeBegin) override;

    QPointF getDefaultEditorPosition() const override;
    int numHandles() const override { return 2; }

    bool isAssistantComplete() const override;

protected:
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QLineF identifyCircle(const QPointF thirdPoint);
    explicit CurvilinearPerspectiveAssistant(const CurvilinearPerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);
};

class CurvilinearPerspectiveAssistantFactory : public KisPaintingAssistantFactory
{
public:
    CurvilinearPerspectiveAssistantFactory();
    ~CurvilinearPerspectiveAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
